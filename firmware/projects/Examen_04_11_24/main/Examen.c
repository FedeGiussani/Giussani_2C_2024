/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define REFRESCO_MEDICION 500000 //en us 
#define TIEMPO_CONVERSION_AD 5000 //el sensor arroja datos cada 10 ms, le pongo por lo menos el doble de frecuencia, mide cada 5000 us
/*==================[internal data definition]===============================*/
uint16_t distancia = 0;

TaskHandle_t Medir_task_handle = NULL;

TaskHandle_t ConversorAD_task_handle = NULL;

uint16_t valorAnalogico_1 = 0;

uint16_t valorAnalogico_2 = 0;

uint16_t valorAnalogico_3 = 0;
/*==================[internal functions declaration]=========================*/
//Parte de medicion y operar con leds
void FuncTimerMedir(void *param)
{
    vTaskNotifyGiveFromISR(Medir_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a medir*/
}

void OperarConDistancia()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
		distancia = HcSr04ReadDistanceInCentimeters();//mide la distancia del sensor

		if (distancia > 500)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		else if ((distancia < 500) & (distancia > 300))
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
			alarmaPrecaucion();
			escribirMensajeUART(0);
		}
		else if (distancia < 300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
			alarmaPeligro();
			escribirMensajeUART(1);
		}
    }
}

//Parte de hacer sonar el buffer
void alarmaPrecaucion(){
	while (true)
	{
		GPIOToggle(GPIO_9);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void alarmaPeligro(){
	while(true){
		GPIOToggle(GPIO_9);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

//parte del mensaje por UART
void escribirMensajeUART(uint16_t alerta)
{
	if (alerta == 0)//precaucion
	{
		UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca.");
	}
	else if(alerta == 1)//peligro
	{
		UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca.");
	}
	else if(alerta == 2)
	{
		UartSendString(UART_CONNECTOR, "Caida detectada.");
	}
}

//parte de la caida
void FuncTimerConversionAD()
{
    vTaskNotifyGiveFromISR(ConversorAD_task_handle, pdFALSE);
}

void AD_conversor_task()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valorAnalogico_1);
		AnalogInputReadSingle(CH2, &valorAnalogico_2);
		AnalogInputReadSingle(CH3, &valorAnalogico_3);
		uint16_t sumatoria = valorAnalogico_1 + valorAnalogico_2 + valorAnalogico_3;
		if (sumatoria > 4){
			escribirMensajeUART(2);
		}
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
	GPIOInit(GPIO_9, GPIO_OUTPUT);
	
	/* Inicialización de timer medicion */
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESCO_MEDICION,
        .func_p = FuncTimerMedir,
        .param_p = NULL};
    TimerInit(&timer_medicion);

	//configurar el puerto UART
	serial_config_t ControlarUart =
        {
            .port = UART_CONNECTOR,
            .baud_rate = 115200,
            .func_p = NULL,
            .param_p = NULL,
        };
    UartInit(&ControlarUart);

	//configurar el conversor AD
	/* Inicialización de timer de conversor AD */
    timer_config_t timer_conversionAD = {
        .timer = TIMER_B,
        .period = TIEMPO_CONVERSION_AD,
        .func_p = FuncTimerConversionAD,
        .param_p = NULL};
    TimerInit(&timer_conversionAD);

    analog_input_config_t Analog_input_1 = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input_1);

	analog_input_config_t Analog_input_2 = {
        .input = CH2,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input_2);

	analog_input_config_t Analog_input_3 = {
        .input = CH3,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input_3);

	//creo la tarea para medir y operar con las medidas
	xTaskCreate(&OperarConDistancia, "medir", 4096, NULL, 5, &Medir_task_handle);
	//creo la tarea para medir y operar con el acelerometro
	xTaskCreate(&AD_conversor_task, "convertir AD", 4096, NULL, 5, &ConversorAD_task_handle);

	TimerStart(timer_medicion.timer);
	TimerStart(timer_conversionAD.timer);

}
/*==================[end of file]============================================*/