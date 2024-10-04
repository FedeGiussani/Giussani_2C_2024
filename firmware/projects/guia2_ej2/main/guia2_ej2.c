/*! @mainpage Medidor de distancia por ultrasonido
 *
 * \section genDesc General Description
 * 
 * Aplicación que mide distancias utilizando un sensor ultrasónico, muestra los resultados en un display LCD, 
 * y controla un conjunto de LEDs basándose en las mediciones obtenidas. El sistema también responde a entradas 
 * de usuario a través de teclas para encender/apagar el dispositivo y mantener la última medición en pantalla.
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
/**
 * @def REFRESCO_MEDICION
 * @brief Intervalo de refresco para la tarea de medición de distancia (en ms).
 */
#define REFRESCO_MEDICION 1000000

/**
 * @def REFRESCO_DISPLAY
 * @brief Intervalo de refresco para la tarea de actualización del display (en ms).
 */
#define REFRESCO_DISPLAY 100000
/*==================[internal data definition]===============================*/
/**
 * @brief Handle para la tarea de medición de distancia.
 */
TaskHandle_t Medir_task_handle = NULL;

/**
 * @brief Handle para la tarea de mostrar la distancia y controlar LEDs.
 */
TaskHandle_t Mostrar_task_handle = NULL;

/**
 * @brief Variable para almacenar la distancia medida por el sensor ultrasónico (en cm).
 */
uint16_t distancia = 0;

/**
 * @brief Estado que indica si el sistema está en modo "hold" (mantiene la distancia en pantalla).
 */
bool hold;

/**
 * @brief Estado del sistema, encendido o apagado.
 */
bool on;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void FuncTimerMedir(void *param)
 * @brief Función llamada por un temporizador para notificar a la tarea de medición de distancia.
 * 
 * Envía una notificación a la tarea encargada de realizar la medición de distancia para que se ejecute.
 * 
 * @param param Parámetro no utilizado.
 * @return 
 */
void FuncTimerMedir(void *param)
{
    vTaskNotifyGiveFromISR(Medir_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a medir*/
}

/**
 * @fn void FuncTimerMostrar(void *param)
 * @brief Función llamada por un temporizador para notificar a la tarea de mostrar la distancia y controlar LEDs.
 * 
 * Envía una notificación a la tarea encargada de mostrar la distancia y controlar los LEDs.
 * 
 * @param param Parámetro no utilizado.
 * @return 
 */
void FuncTimerMostrar(void *param)
{
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}

/**
 * @fn void medirTask()
 * @brief Tarea para medir la distancia utilizando el sensor ultrasónico.
 * 
 * La tarea se ejecuta tras recibir una notificación (mediante `ulTaskNotifyTake`), 
 * y cuando el sistema está encendido (`on`), mide la distancia con el sensor ultrasónico.
 * El resultado se almacena en la variable `distancia`.
 * 
 * @note La tarea espera indefinidamente hasta recibir la notificación para ejecutarse.
 * @return 
 */
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

/**
 * @fn void mostrarTask()
 * @brief Funcion para mostrar la distancia en el display y controlar los LEDs.
 * 
 * La tarea se ejecuta tras recibir una notificación (mediante `ulTaskNotifyTake`), 
 * y cuando el sistema está encendido (`on`), enciende o apaga los LEDs en función de la distancia medida 
 * y actualiza la pantalla LCD a menos que esté en modo "hold".
 * 
 * - Distancia < 10 cm: Apaga todos los LEDs.
 * - Distancia entre 10 y 20 cm: Enciende solo LED_1.
 * - Distancia entre 20 y 30 cm: Enciende LED_1 y LED_2.
 * - Distancia > 30 cm: Enciende todos los LEDs.
 * 
 * @note La tarea espera indefinidamente hasta recibir la notificación para ejecutarse.
 * @return 
 */
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

/**
 * @fn void TeclaOn()
 * @brief Función que alterna el estado de encendido/apagado del sistema.
 * 
 * Esta función es llamada cuando el usuario presiona el botón para encender o apagar el sistema.
 * @return
 */
void TeclaOn()
{
    on = !on;
}

/**
 * @fn void TeclaHold()
 * @brief Función que alterna el modo "hold" (mantener la distancia en pantalla) del sistema.
 * 
 * Esta función es llamada cuando el sistema está encendido (`on`) y el usuario presiona el botón para activar 
 * o desactivar el modo "hold".
 * @return 
 */
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