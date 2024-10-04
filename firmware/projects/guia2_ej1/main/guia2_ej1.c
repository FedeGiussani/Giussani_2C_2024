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
 * |   Date	    | Description            |
 * |:----------:|:-----------------------|
 * | 30/08/2024 | Document creation      |
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
/*==================[macros and definitions]=================================*/
#define REFRESCO_TECLAS 50

#define REFRESCO_MEDICION 1000

#define REFRESCO_DISPLAY 100
/*==================[internal data definition]===============================*/
uint16_t distancia = 0;

bool hold;

bool on;
/*==================[internal functions declaration]=========================*/
void medirTask()
{ // en esta tarea se mide las distancias por el sensor y se modifica el estado de los leds dependiendo lo que se este midiendo
    while (true)
    {
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        vTaskDelay(REFRESCO_MEDICION / portTICK_PERIOD_MS);
    }
}

void mostrarTask()
{
    while (true)
    {
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
        vTaskDelay(REFRESCO_DISPLAY / portTICK_PERIOD_MS);
    }
}

void teclasTask()
{
    uint8_t teclas;
    while (true)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            on = !on;
            break;
        case SWITCH_2:
            hold = !hold;
            break;
        }
        vTaskDelay(REFRESCO_TECLAS / portTICK_PERIOD_MS);
    }
} 
/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    xTaskCreate(&medirTask, "medir", 512, NULL, 5, NULL);
    xTaskCreate(&teclasTask, "mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&mostrarTask, "teclas", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/