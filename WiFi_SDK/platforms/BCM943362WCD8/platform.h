/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#pragma once

#include "platform_sleep.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/*
BCM943362WCD9 platform pin definitions ...
+--------------------------------------------------------------------------------------------------------+
| Enum ID       |Pin |   Pin Name on    |    Module     | STM32| Peripheral  |    Board     | Peripheral  |
|               | #  |      Module      |  GPIO Alias   | Port | Available   |  Connection  |   Alias     |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_1  | 20 | MICRO_WKUP       | WICED_GPIO_1  | A  0 | GPIO        |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_2  | 32 | MICRO_ADC_IN1    | WICED_GPIO_2  | A  1 | GPIO        | BUTTON SW2   |             |
|               |    |                  |               |      | TIM2_CH2    |              |             |
|               |    |                  |               |      | TIM5_CH2    |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_3  | 31 | MICRO_ADC_IN2    | WICED_GPIO_3  | A  2 | ADC123_IN2  |              |             |
|               |    |                  |               |      | GPIO        | BUTTON SW1   |             |
|               |    |                  |               |      | TIM2_CH3    |              |             |
|               |    |                  |               |      | TIM5_CH3    |              |             |
|               |    |                  |               |      | TIM9_CH1    |              |             |
|               |    |                  |               |      | USART2_TX   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_4  | 30 | MICRO_ADC_IN3    | WICED_GPIO_4  | A  3 | ADC123_IN3  |              | WICED_ADC_3 |
|               |    |                  |               |      | GPIO        | THERMISTOR   |             |
|               |    |                  |               |      | TIM2_CH4    |              |             |
|               |    |                  |               |      | TIM5_CH4    |              |             |
|               |    |                  |               |      | TIM9_CH2    |              |             |
|               |    |                  |               |      | UART2_RX    |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_5  | 37 | MICRO_SPI_SSN    | WICED_GPIO_5  | A  4 | ADC12_IN4   |              |             |
|               |    |                  |               |      | DAC1_OUT    |              |             |
|               |    |                  |               |      | GPIO        |              |             |
|               |    |                  |               |      | I2S3_WS     |              |             |
|               |    |                  |               |      | SPI1_NSS    |              |             |
|               |    |                  |               |      | SPI3_NSS    |              |             |
|               |    |                  |               |      | USART2_CK   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_6  | 36 | MICRO_SPI_SCK    | WICED_GPIO_6  | A  5 | ADC12_IN5   |              |             |
|               |    |                  |               |      | DAC2_OUT    |              |             |
|               |    |                  |               |      | GPIO        |              |             |
|               |    |                  |               |      | SPI1_SCK    |              |             |
|               |    |                  |               |      | TIM2_CH1_ETR|              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_7  | 35 | MICRO_SPI_MISO   | WICED_GPIO_7  | A  6 | ADC12_IN6   |              |             |
|               |    |                  |               |      | GPIO        |              |             |
|               |    |                  |               |      | SPI1_MISO   |              |             |
|               |    |                  |               |      | TIM1_BKIN   |              |             |
|               |    |                  |               |      | TIM3_CH1    |              |             |
|               |    |                  |               |      | TIM8_BKIN   |              |             |
|               |    |                  |               |      | TIM13_CH1   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_8  | 34 | MICRO_SPI_MOSI   | WICED_GPIO_8  | A  7 | ADC12_IN7   |              |             |
|               |    |                  |               |      | GPIO        |              |             |
|               |    |                  |               |      | SPI1_MOSI   |              |             |
|               |    |                  |               |      | TIM1_CH1N   |              |             |
|               |    |                  |               |      | TIM3_CH2    |              |             |
|               |    |                  |               |      | TIM8_CH1N   |              |             |
|               |    |                  |               |      | TIM14_CH1   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_9  | 9  | MICRO_UART_TX    | WICED_GPIO_9  | A  9 | GPIO        |              |             |
|               |    |                  |               |      | I2C3_SMBA   |              |             |
|               |    |                  |               |      | TIM1_CH2    |              |             |
|               |    |                  |               |      | USART1_TX   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_10 | 10 | MICRO_UART_RX    | WICED_GPIO_10 | A 10 | GPIO        |              |             |
|               |    |                  |               |      | TIM1_CH3    |              |             |
|               |    |                  |               |      | USART1_RX   |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_11 | 29 | MICRO_GPIO_0     | WICED_GPIO_11 | B  6 | GPIO        | LED D1       |             |
|               |    |                  |               |      | TIM4_CH1    |              | WICED_PWM_1 |
|               |    |                  |               |      | I2C1_SCL    |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_12 | 28 | MICRO_GPIO_1     | WICED_GPIO_12 | B  7 | GPIO        | LED D2       |             |
|               |    |                  |               |      | TIM4_CH2    |              | WICED_PWM_2 |
|               |    |                  |               |      | I2C1_SDA    |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_13 |  3 | MICRO_JTAG_TMS   | WICED_GPIO_13 | A 13 | GPIO        |              |             |
|               |    |                  |               |      | JTMS-SWDIO  |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_14 |  4 | MICRO_JTAG_TCK   | WICED_GPIO_14 | A 14 | GPIO        |              |             |
|               |    |                  |               |      | JTCK-SWCLK  |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_15 |  5 | MICRO_JTAG_TDI   | WICED_GPIO_15 | A 15 | GPIO        |              |             |
|               |    |                  |               |      | JTDI        |              |             |
|               |    |                  |               |      | I2S3_WS     |              |             |
|               |    |                  |               |      | SPI1_NSS    |              |             |
|               |    |                  |               |      | SPI3_NSS    |              |             |
|               |    |                  |               |      | TIM2_CH1_ETR|              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_16 |  6 | MICRO_JTAG_TDO   | WICED_GPIO_16 | B  3 | GPIO        |              |             |
|               |    |                  |               |      | JTDO        |              |             |
|               |    |                  |               |      | SPI1_SCK    |              |             |
|               |    |                  |               |      | SPI3_SCK    |              |             |
|               |    |                  |               |      | I2S3_SCK    |              |             |
|               |    |                  |               |      | TIM2_CH2    |              |             |
|               |    |                  |               |      | TRACESWO    |              |             |
|---------------+----+------------------+---------------+------+-------------+--------------+-------------|
| WICED_GPIO_17 | 7  | MICRO_JTAG_TRSTN | WICED_GPIO_17 | B  4 | GPIO        |              |             |
|               |    |                  |               |      | NJTRST      |              |             |
|               |    |                  |               |      | SPI1_MISO   |              |             |
|               |    |                  |               |      | SPI3_MISO   |              |             |
|               |    |                  |               |      | TIM3_CH1    |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_18 | 27 | MICRO_GPIO_2     | WICED_GPIO_18 | D 10 | GPIO        |              |             |
|               |    |                  |               |      | USART3_CK   |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_19 | 26 | MICRO_GPIO_3     | WICED_GPIO_19 | D 11 | GPIO        |              |             |
|               |    |                  |               |      | USART3_CTS  |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_20 | 25 | I2S2_CK          | WICED_GPIO_20 | D  3 | GPIO        |              |             |
|               |    |                  |               |      | USART2_CTS  |              |             |
|               |    |                  |               |      | SPI2_SCK    |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_21 | 24 | I2S2_MCK         | WICED_GPIO_21 | C  6 | GPIO        |              |             |
|               |    |                  |               |      | TIM8_CH1    |              |             |
|               |    |                  |               |      | USART6_TX   |              |             |
|               |    |                  |               |      | TIM3_CH1    |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_22 | 23 | I2S2_SD          | WICED_GPIO_22 | C  3 | GPIO        |              |             |
|               |    |                  |               |      | SPI2_MOSI   |              |             |
|               |    |                  |               |      | ADC123_IN13 |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_23 | 22 | I2S2_WS          | WICED_GPIO_23 | B  9 | GPIO        |              |             |
|               |    |                  |               |      | SPI2_NSS    |              |             |
|               |    |                  |               |      | TIM4_CH4    |              |             |
|               |    |                  |               |      | TIM11_CH1   |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_24 | 21 | I2S2ext_SD       | WICED_GPIO_24 | C  2 | GPIO        |              |             |
|               |    |                  |               |      | SPI2_MISO   |              |             |
|               |    |                  |               |      | ADC123_IN12 |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_25 | 15 | USART6_RTS       | WICED_GPIO_25 | G 12 | GPIO        |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_26 | 14 | USART6_CTS       | WICED_GPIO_26 | G 13 | GPIO        |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_27 | 13 | USART6_CK        | WICED_GPIO_27 | G  7 | GPIO        |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_28 | 12 | USART6_RX        | WICED_GPIO_28 | G  9 | GPIO        |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
| WICED_GPIO_29 | 11 | USART6_TX        | WICED_GPIO_29 | G 14 | GPIO        |              |             |
+---------------+----+------------------+------+---------------+-------------+--------------+-------------+
Notes
1. These mappings are defined in <WICED-SDK>/Platform/BCM943362WCD9/platform.c
*/


typedef enum
{
    WICED_GPIO_1,
    WICED_GPIO_2,
    WICED_GPIO_3,
    WICED_GPIO_4,
    WICED_GPIO_5,
    WICED_GPIO_6,
    WICED_GPIO_7,
    WICED_GPIO_8,
    WICED_GPIO_9,
    WICED_GPIO_10,
    WICED_GPIO_11,
    WICED_GPIO_12,
    WICED_GPIO_13,
    WICED_GPIO_14,
    WICED_GPIO_15,
    WICED_GPIO_16,
    WICED_GPIO_17,
    WICED_GPIO_18,
    WICED_GPIO_19,
    WICED_GPIO_20,
    WICED_GPIO_21,
    WICED_GPIO_22,
    WICED_GPIO_23,
    WICED_GPIO_24,
    WICED_GPIO_25,
    WICED_GPIO_26,
    WICED_GPIO_27,
    WICED_GPIO_28,
    WICED_GPIO_29,
    WICED_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
} wiced_gpio_t;

typedef enum
{
    WICED_SPI_1,
    WICED_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
} wiced_spi_t;

typedef enum
{
    WICED_I2C_1,
    WICED_I2C_MAX
} wiced_i2c_t;

typedef enum
{
    WICED_PWM_1,
    WICED_PWM_2,
    WICED_PWM_3,
    WICED_PWM_4,
    WICED_PWM_5,
    WICED_PWM_6,
    WICED_PWM_7,
    WICED_PWM_8,
    WICED_PWM_9,
    WICED_PWM_10,
    WICED_PWM_11,
    WICED_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
} wiced_pwm_t;

typedef enum
{
    WICED_ADC_1,
    WICED_ADC_2,
    WICED_ADC_3,
    WICED_ADC_4,
    WICED_ADC_5,
    WICED_ADC_6,
    WICED_ADC_7,
    WICED_ADC_8,
    WICED_ADC_9,
    WICED_ADC_10,
    WICED_ADC_11,
    WICED_ADC_12,
    WICED_ADC_13,
    WICED_ADC_14,
    WICED_ADC_15,
    WICED_ADC_MAX, /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
} wiced_adc_t;

typedef enum
{
    WICED_UART_1,
    WICED_UART_2,
    WICED_UART_3,
    WICED_UART_4,
    WICED_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
} wiced_uart_t;

/* Logical Button-ids which map to phyiscal buttons on the board */
typedef enum
{
    PLATFORM_BUTTON_1,
    PLATFORM_BUTTON_2,
    PLATFORM_BUTTON_MAX, /* Denotes the total number of Buttons on the board. Not a valid Button Alias */
} platform_button_t;

#define WICED_PLATFORM_BUTTON_COUNT  ( 2 )

/* UART port used for standard I/O */
#define STDIO_UART ( WICED_UART_1 )


#define WICED_PLATFORM_INCLUDES_SPI_FLASH
#define WICED_SPI_FLASH_CS  (WICED_GPIO_5)

/* Components connected to external I/Os*/
#define PLATFORM_LED_COUNT               ( 2 )
#define WICED_LED1       (WICED_GPIO_11)
#define WICED_LED2       (WICED_GPIO_12)
#define WICED_LED1_ON_STATE  ( WICED_ACTIVE_HIGH )
#define WICED_LED2_ON_STATE  ( WICED_ACTIVE_HIGH )

#define WICED_BUTTON1    (WICED_GPIO_3)
#define WICED_BUTTON2    (WICED_GPIO_2)
#define WICED_THERMISTOR (WICED_GPIO_4)

/* I/O connection <-> Peripheral Connections */
#define WICED_LED1_JOINS_PWM       (WICED_PWM_1)
#define WICED_LED2_JOINS_PWM       (WICED_PWM_2)
#define WICED_THERMISTOR_JOINS_ADC (WICED_ADC_3)

/* Bootloader OTA and OTA2 factory reset during settings */
#define PLATFORM_FACTORY_RESET_BUTTON_INDEX     ( PLATFORM_BUTTON_1 )
#define PLATFORM_FACTORY_RESET_TIMEOUT          ( 10000 )

/* Generic button checking defines */
#define PLATFORM_BUTTON_PRESS_CHECK_PERIOD      ( 100 )
#define PLATFORM_BUTTON_PRESSED_STATE           (   0 )

#define PLATFORM_GREEN_LED_INDEX                ( WICED_LED_INDEX_1 )
#define PLATFORM_RED_LED_INDEX                  ( WICED_LED_INDEX_2 )

#ifdef __cplusplus
} /*extern "C" */
#endif
