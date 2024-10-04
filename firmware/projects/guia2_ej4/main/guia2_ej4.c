/*! @mainpage 
 *
 * @section genDesc General Description
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description         |
 * |:----------:|:--------------------|
 * | 27/09/2024 | Document creation   |
 *
 * @author Federico Giussani (federico.giussani@ingenieria.uner.edu.ar)
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
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define TIEMPO_CONVERSION_AD 2000
#define TIEMPO_CONVERSION_DA 4000
#define BUFFER_SIZE 256
/*==================[internal data definition]===============================*/
TaskHandle_t ConversorAD_task_handle = NULL;

TaskHandle_t ConversorDA_task_handle = NULL;

uint16_t valorAnalogico = 0;

/*const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};*/

const char ecg[BUFFER_SIZE] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17
}; // ecg alternativo
/*==================[internal functions declaration]=========================*/
void escribirValorEnPc()
{
    UartSendString(UART_PC, (char *)UartItoa(valorAnalogico, 10));
    UartSendString(UART_PC, "\r");
}

void AD_conversor_task()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valorAnalogico);
        escribirValorEnPc();
    }
}

void DA_conversor_task()
{
    uint8_t i = 0;
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // enviar dato
        AnalogOutputWrite(ecg[i]);
        i++;
        if (i == BUFFER_SIZE-1)
        {
            i = 0;
        }
    }
}

void FuncTimerConversionDA()
{
    vTaskNotifyGiveFromISR(ConversorDA_task_handle, pdFALSE);
}

void FuncTimerConversionAD()
{
    vTaskNotifyGiveFromISR(ConversorAD_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    serial_config_t ControlarUart =
        {
            .port = UART_PC,
            .baud_rate = 115200,
            .func_p = NULL,
            .param_p = NULL,
        };
    UartInit(&ControlarUart);

	/* Inicialización de timer de conversor AD */
    timer_config_t timer_conversionDA = {
        .timer = TIMER_A,
        .period = TIEMPO_CONVERSION_DA,
        .func_p = FuncTimerConversionDA,
        .param_p = NULL};
    TimerInit(&timer_conversionDA);

    timer_config_t timer_conversionAD = {
        .timer = TIMER_B,
        .period = TIEMPO_CONVERSION_AD,
        .func_p = FuncTimerConversionAD,
        .param_p = NULL};
    TimerInit(&timer_conversionAD);

    analog_input_config_t Analog_input = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input);
    AnalogOutputInit();

    /*creacion de tareas*/
	xTaskCreate(&DA_conversor_task, "conversor DA", 512, NULL, 5, &ConversorDA_task_handle);
    xTaskCreate(&AD_conversor_task, "conversor AD", 4096, NULL, 5, &ConversorAD_task_handle);

    /*Inicio del conteo de timers*/
    TimerStart(timer_conversionAD.timer);
	TimerStart(timer_conversionDA.timer);
}
/*==================[end of file]============================================*/