/*! @mainpage Guia 1 Ejercicio 5
 *
 * @section genDesc General Description
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
 * | 16/08/2024 | Document creation		                         |
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
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define N_BITS 4
#define LCD_DIGITS 3
/*==================[internal data definition]===============================*/
typedef struct gpioConf_t
{
    gpio_t pin;    /*!< GPIO pin number */
    io_t dir;      /*!< GPIO direction '0' IN;  '1' OUT*/
}gpioConf_t;

/*==================[internal functions declaration]=========================*/
void BCDtoGPIO(uint8_t digit, gpioConf_t *gpio_config);

/*==================[external functions definition]==========================*/

// Implementación de BCDtoGPIO
void BCDtoGPIO(uint8_t digit, gpioConf_t *gpio_config) {
    for (uint8_t i = 0; i < N_BITS; i++) {
        GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
    }

    for (uint8_t i = 0; i < N_BITS; i++) {
        if ((digit & (1 << i)) == 0) 
        {
            GPIOOff(gpio_config[i].pin);
        } 
        else 
        {
            GPIOOn(gpio_config[i].pin);
        }
    }
}

// Función principal que se ejecuta al inicio del programa.	
void app_main(void)
{
    // Se define un número BCD para probar la función. En binario, 6 se representa como 0110, que corresponde a:
    //b0 = 0 (GPIO_20 apagado)
    //b1 = 1 (GPIO_21 encendido)
    //b2 = 1 (GPIO_22 encendido)
    //b3 = 0 (GPIO_23 apagado)
    uint8_t digit = 6;
    
    struct gpioConf_t config_pines[N_BITS]; // Declaro un arreglo de estructuras config_pines para almacenar la configuración de los 4 pines GPIO.
    // Asigna pines GPIO a cada bit 
    config_pines[0].pin = GPIO_20; // b0 -> GPIO_20
    config_pines[1].pin = GPIO_21; // b1 -> GPIO_21
	config_pines[2].pin = GPIO_22; // b2 -> GPIO_22
	config_pines[3].pin = GPIO_23; // b3 -> GPIO_23

	for(uint8_t i = 0; i < N_BITS; i++) // Un bucle para configurar la dirección de cada pin como salida.
	{
		config_pines[i].dir = 1; // Establece la dirección de cada pin en el arreglo como salida (1).
	}

	BCDtoGPIO(digit, config_pines); // Llama a la función BCDtoGPIO, pasando el dígito 6 y el arreglo config_pines para 
	//cambiar el estado de los pines GPIO según el dígito BCD.


}
/*==================[end of file]============================================*/