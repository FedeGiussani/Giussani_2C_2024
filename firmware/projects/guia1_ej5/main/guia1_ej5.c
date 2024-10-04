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
typedef struct
{
    gpio_t pin;    /*!< GPIO pin number */
    io_t dir;      /*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
void setGpioFromBcd(uint8_t bcd_digit, gpioConf_t *gpio_conf);

/*==================[external functions definition]==========================*/

// Implementación de BCDtoGPIO
void BCDtoGPIO(uint8_t digit, gpioConf_t *gpio_config) {
    for (uint8_t i = 0; i < N_BITS; i++) {
        GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
    }

    for (uint8_t i = 0; i < N_BITS; i++) {
        if ((digit & (1 << i)) == 0) {
            GPIOOff(gpio_config[i].pin);
        } else {
            GPIOOn(gpio_config[i].pin);
        }
    }
}

/*==================[end of file]============================================*/