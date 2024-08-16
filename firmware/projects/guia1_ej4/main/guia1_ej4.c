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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);
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

int app_main(){
    uint32_t data = 123;        // Número a convertir
    uint8_t digits = 3;              // Cantidad de dígitos esperados en el BCD
    uint8_t bcd_number[3] = {0};     // Arreglo donde se almacenarán los dígitos BCD

    // Llamada a la función para convertir el número a BCD
    int8_t result = convertToBcdArray(data, digits, bcd_number);

    // Verificar si la conversión fue exitosa
    if (result == 0) {
        printf("Conversion exitosa!\n");
        printf("BCD: ");
        for (int i = 0; i < digits; i++) {
            printf("%d ", bcd_number[i]);
        }
        printf("\n");
    } else {
        printf("Error en la conversion, código de error: %d\n", result);
    }

    return 0;
}
/*==================[end of file]============================================*/