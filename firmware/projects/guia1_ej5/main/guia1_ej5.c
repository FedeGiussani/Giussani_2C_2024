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
#include "switch.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
    gpio_t pin;    /*!< GPIO pin number */
    io_t dir;      /*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);
void setGpioFromBcd(uint8_t bcd_digit, gpioConf_t *gpio_conf);
/*==================[external functions definition]==========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
    // Asegurarse de que el arreglo donde se almacenarán los dígitos no sea nulo.
    if (bcd_number == NULL)
    {
        return -1; // Error: puntero nulo
    }

    // Iterar para extraer cada dígito y almacenarlo en el arreglo
    for (int i = digits - 1; i >= 0; i--)
    {
        if (i < 0)
        {
            return -2; // Error: cantidad de dígitos fuera de rango
        }
        bcd_number[i] = data % 10;  // Extraer el dígito menos significativo
        data /= 10;  // Eliminar el dígito menos significativo
    }

    return 0; // Operación exitosa
}

void setGpioFromBcd(uint8_t bcd_digit, gpioConf_t *gpio_conf) {
    // Definir el mapeo de bits a pines GPIO
    gpioConf_t gpio_map[4] = {
        {GPIO_20, 1}, // b0 -> GPIO_20
        {GPIO_21, 1}, // b1 -> GPIO_21
        {GPIO_22, 1}, // b2 -> GPIO_22
        {GPIO_23, 1}  // b3 -> GPIO_23
    };

    // Configurar el estado de cada GPIO según el dígito BCD
    for (int i = 0; i < 4; i++) {
        if (bcd_digit & (1 << i)) {
            gpio_conf[i].pin = gpio_map[i].pin;
            gpio_conf[i].dir = 1; // Setear el pin a '1'
        } else {
            gpio_conf[i].pin = gpio_map[i].pin;
            gpio_conf[i].dir = 0; // Setear el pin a '0'
        }

        // Aquí podrías llamar a una función que realmente aplique la configuración a los pines
        // Por ejemplo: gpio_write(gpio_conf[i].pin, gpio_conf[i].dir);
    }
}

int app_main(){
	gpioConf_t gpio_conf[4]; // Arreglo para almacenar la configuración de los GPIOs
    uint8_t bcd_digit = 0b1010; // Ejemplo: dígito BCD (10 en decimal)

    // Llamar a la función para configurar los pines GPIO
    setGpioFromBcd(bcd_digit, gpio_conf);

    // Aquí podrías verificar la configuración o aplicarla realmente
    // por ejemplo, llamando a gpio_write(gpio_conf[i].pin, gpio_conf[i].dir) dentro de un bucle.
    
    return 0;
}
/*==================[end of file]============================================*/