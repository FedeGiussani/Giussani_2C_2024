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
/**
 * @def REFRESCO_TECLAS
 * @brief Intervalo de refresco para la tarea de lectura de teclas (en ms).
 */
#define REFRESCO_TECLAS 50

/**
 * @def REFRESCO_MEDICION
 * @brief Intervalo de refresco para la tarea de medición de distancia (en ms).
 */
#define REFRESCO_MEDICION 1000

/**
 * @def REFRESCO_DISPLAY
 * @brief Intervalo de refresco para la tarea de actualización del display (en ms).
 */
#define REFRESCO_DISPLAY 100

/*==================[internal data definition]===============================*/
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
 * @fn void medirTask()
 * @brief Función para medir la distancia utilizando el sensor ultrasónico.
 * 
 * Esta función se ejecuta en un bucle continuo y lee la distancia cuando el sistema está encendido.
 * El valor leído se almacena en la variable `distancia`.
 * @param
 * @note La medición se realiza cada `REFRESCO_MEDICION` ms.
 * @return
 */
void medirTask()
{
    while (true)
    {
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        vTaskDelay(REFRESCO_MEDICION / portTICK_PERIOD_MS);
    }
}

/**
 * @fn void mostrarTask()
 * @brief Funcion para mostrar la distancia en un display LCD y controlar los LEDs según la distancia.
 * 
 * Dependiendo de la distancia medida, enciende los LEDs correspondientes y actualiza el display LCD.
 * 
 * - Si la distancia es menor a 10 cm, apaga todos los LEDs.
 * - Si la distancia está entre 10 y 20 cm, enciende el LED_1.
 * - Si la distancia está entre 20 y 30 cm, enciende LED_1 y LED_2.
 * - Si la distancia es mayor a 30 cm, enciende todos los LEDs.
 * 
 * @param
 * @note La pantalla se actualiza cada `REFRESCO_DISPLAY` ms, a menos que el sistema esté en modo "hold".
 * @return 
 */
void mostrarTask()
{
    while (true)
    {
        if (on)
        {
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

/**
 * @fn void teclasTask()
 * @brief Tarea para leer las teclas y actualizar el estado del sistema.
 * 
 * Lee las teclas y cambia el estado de `on` (encendido/apagado) o `hold` (mantener pantalla) según la tecla presionada:
 * 
 * - `SWITCH_1`: Alterna el estado de `on`.
 * - `SWITCH_2`: Alterna el estado de `hold`.
 * 
 * @param
 * @note Las teclas se leen cada `REFRESCO_TECLAS` ms.
 * @return
 */
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