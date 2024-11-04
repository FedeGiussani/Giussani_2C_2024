/*! @mainpage Guia 1 Ejercicio 6
 *
 * @section genDesc General Description
 * 
 * Este código configura pines GPIO para controlar un display LCD, mostrando números en formato BCD.
 *
 * Función que recibe un dato de 32 bits, la cantidad de dígitos de salida y dos vectores 
 * de estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al definido en el punto 
 * anterior y el otro vector mapea los puertos con el dígito del LCD a donde mostrar un dato.
 * La función muestra por display el valor que recibe.
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

/** @brief Número de bits utilizados para representar un dígito en BCD. */
#define N_BITS 4

/** @brief Número de dígitos que se mostrarán en el display LCD. */
#define LCD_DIGITS 3

/*==================[internal data definition]===============================*/

/** @brief Configuración de un pin GPIO. */
typedef struct
{
    gpio_t pin;    /*!< GPIO pin number */
    io_t dir;      /*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/** @fn int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 * @brief Convierte un número entero a un arreglo en formato BCD.
 * 
 * @param data Número a convertir.
 * @param digits Cantidad de dígitos que se mostrarán.
 * @param bcd_number Puntero al arreglo donde se almacenarán los dígitos en formato BCD.
 * @return int8_t Código de error: 0 si es exitoso, -1 si el puntero es nulo, -2 si la cantidad de dígitos es fuera de rango.
*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);

/**
 * @fn void BCDtoGPIO(uint8_t digit, gpioConf_t *gpio_config)
 * @brief Configura los pines GPIO para mostrar un dígito en formato BCD.
 *
 * @param digit Dígito en BCD a mostrar.
 * @param gpio_config Configuración de pines GPIO para mostrar el dígito.
 * @return
 */
void BCDtoGPIO(uint8_t digit, gpioConf_t *gpio_config);

/**
 * @fn void displayNumberOnLCD(uint32_t data, gpioConf_t *data_gpio_config, gpioConf_t *digit_gpio_config)
 * @brief Muestra un número en el display LCD.
 *
 * @param data Número a mostrar.
 * @param data_gpio_config Configuración de pines GPIO para los datos.
 * @param digit_gpio_config Configuración de pines GPIO para los dígitos del display.
 * @return
 */
void displayNumberOnLCD(uint32_t data, gpioConf_t *data_gpio_config, gpioConf_t *digit_gpio_config);

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

// Función para mostrar un número en el display LCD
void displayNumberOnLCD(uint32_t data, gpioConf_t *data_gpio_config, gpioConf_t *digit_gpio_config) {
    uint8_t bcd_array[LCD_DIGITS]; //arreglo donde se va a guardar el BCD
    
    // Convertir el número a formato BCD
    convertToBcdArray(data, LCD_DIGITS, bcd_array);

    // Inicializar los pines del LCD
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        GPIOInit(digit_gpio_config[i].pin, digit_gpio_config[i].dir);
    }

    // Mostrar cada dígito en el display
    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        // Apagar todos los dígitos
        for (uint8_t j = 0; j < LCD_DIGITS; j++) {
            GPIOOff(digit_gpio_config[j].pin);
        }
        
        // Configurar el dígito actual para que esté encendido
        GPIOOn(digit_gpio_config[i].pin);
        
        // Mostrar el valor BCD del dígito
        BCDtoGPIO(bcd_array[i], data_gpio_config);
        

    }
}

void app_main(void) {
    // Configuración de pines de datos y dígitos
    gpioConf_t data_gpio_config[N_BITS] = {
        {GPIO_20, 1},
        {GPIO_21, 1},
        {GPIO_22, 1},
        {GPIO_23, 1}
    };
    //configura el puerto donde se va a mostrar el digito 
    gpioConf_t digit_gpio_config[LCD_DIGITS] = {
        {GPIO_19, 1}, // Dígito 1
        {GPIO_18, 1}, // Dígito 2
        {GPIO_9, 1}   // Dígito 3
    };

    uint32_t number = 123; // Número a mostrar en el display

    displayNumberOnLCD(number, data_gpio_config, digit_gpio_config);
    LedsInit();
    LedOn(LED_1);
}
/*==================[end of file]============================================*/




