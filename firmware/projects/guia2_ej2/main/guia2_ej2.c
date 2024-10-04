/*! @mainpage Medidor de distancia por ultrasonido
 *
 * \section genDesc General Description
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
 * | 13/09/2024 | Document creation   |
 *
 * @author Federico Giussani (federico.giussani@ingenieria.uner.edu.ar)
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
/*==================[macros and definitions]=================================*/
#define REFRESCO_MEDICION 1000000

#define REFRESCO_DISPLAY 100000
/*==================[internal data definition]===============================*/
TaskHandle_t Medir_task_handle = NULL;

TaskHandle_t Mostrar_task_handle = NULL;

uint16_t distancia = 0;

bool hold;

bool on;
/*==================[internal functions declaration]=========================*/
void FuncTimerMedir(void *param)
{
    vTaskNotifyGiveFromISR(Medir_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a medir*/
}

void FuncTimerMostrar(void *param)
{
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}

void medirTask()
{ // en esta tarea se mide las distancias por el sensor y se modifica el estado de los leds dependiendo lo que se este midiendo
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        
    }
}

void mostrarTask()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
        if (on)
        {
            // aca se realizan las tareas de encender leds dependiendo la distancia y encender el display
            if (distancia < 10)
            {
                LedsOffAll();
            }
            else if ((distancia > 10) & (distancia < 20))
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if ((distancia > 20) & (distancia < 30))
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else if (distancia > 30)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }

            if (!hold)
            {
                LcdItsE0803Write(distancia);
            }
        }
        else
        {
            LcdItsE0803Off();
            LedsOffAll();
        }
        
    }
}

void TeclaOn()
{
    on = !on;
}

void TeclaHold()
{
    if (on)
    {
        hold = !hold;
    }
}
/*==================[external functions definition]==========================*/

void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    /* Inicialización de timer medicion */
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESCO_MEDICION,
        .func_p = FuncTimerMedir,
        .param_p = NULL};
    TimerInit(&timer_medicion);

    /* Inicialización de timer mostrar */
    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = REFRESCO_DISPLAY,
        .func_p = FuncTimerMostrar,
        .param_p = NULL};
    TimerInit(&timer_mostrar);

    /*En este caso, a partir de crear la interrupcion iniciada por el SWITCH_1 elijo que la funcion estadoTeclas
    se va a ejecutar*/
    SwitchActivInt(SWITCH_1, &TeclaOn, NULL);

    /*En este caso, a partir de crear la interrupcion iniciada por el SWITCH_2 elijo que la funcion estadoTeclas
    se va a ejecutar*/
    SwitchActivInt(SWITCH_2, &TeclaHold, NULL);

    /*creacion de tareas*/
    xTaskCreate(&medirTask, "medir", 512, NULL, 5, &Medir_task_handle);
    xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &Mostrar_task_handle);

    /*Inicio del conteo de timers*/
    TimerStart(timer_medicion.timer);
    TimerStart(timer_mostrar.timer);
}

/*==================[end of file]============================================*/