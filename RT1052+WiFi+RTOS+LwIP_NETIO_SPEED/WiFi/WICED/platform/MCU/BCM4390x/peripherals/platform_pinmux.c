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

#include <stdint.h>
#include <string.h>
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_map.h"
#include "platform_appscr4.h"
#include "wwd_rtos.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"
#include "platform_pinmux.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define GCI_REG_MASK(mask, pos)              (((uint32_t)(mask)) << (pos))

/******************************************************
 *               Variables Definitions
 ******************************************************/

static const pin_function_index_t function_index_hw_default  = PIN_FUNCTION_INDEX_0;
static const pin_function_index_t function_index_same_as_pin = PIN_FUNCTION_INDEX_1;

/*
 * Specification of the 43909 pin function multiplexing table.
 * Documentation on pin function selection mapping can be found
 * inside the 43909 GCI Chip Control Register programming page.
 *
 * Note:
 * The pin function selections need to be explicitly stated.
 * Use PIN_FUNCTION_UNKNOWN if a pin's function selection is
 * not explicitly known or defined from software perspective.
 *
 * Do NOT use PIN_FUNCTION_HW_DEFAULT or PIN_FUNCTION_SAME_AS_PIN.
 * These are NOT explicit pin function selection values.
 */
static const platform_pin_internal_config_t pin_internal_config[] =
{
    {
        .pin_pad_name           = PIN_GPIO_0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 0,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RX},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MISO},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_12},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_8},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_USB, PIN_FUNCTION_USB20H_CTL1}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 4,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_TX},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CLK},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_13},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_9},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_2,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 8,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_2},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_0},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TCK},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_3,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 12,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_3},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_1},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TMS},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_4,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 16,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_4},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_2},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TDI},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_5,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 20,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_5},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_3},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TDO},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_6,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 24,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_6},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_4},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TRST_L},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_7,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_0,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 28,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_7},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RTS_OUT},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CS},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_15},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_11},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_PMU_TEST_O},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_8,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 0,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_8},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MISO},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RX},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_16},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_12},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_JTAG, PIN_FUNCTION_TAP_SEL_P},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_9,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 4,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_9},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CLK},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_TX},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_0},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_13},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_10,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 8,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_10},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MOSI},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_CTS_IN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_1},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_14},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_SEP_INT},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_SEP_INT_0D}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_11,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 12,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_11},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CS},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RTS_OUT},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_7},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_15},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_12,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 16,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_12},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RX},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MISO},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_8},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_16},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_SEP_INT_0D},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_SEP_INT}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_13,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 20,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_13},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_TX},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CLK},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_9},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_0},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_14,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 24,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_14},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_CTS_IN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MOSI},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_10},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_15,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_1,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 28,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_15},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_RTS_OUT},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_CS},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_CLK},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_11},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_7},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3}
        }
    },
    {
        .pin_pad_name           = PIN_GPIO_16,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 24,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_16},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_FAST_UART, PIN_FUNCTION_FAST_UART_CTS_IN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_SPI1, PIN_FUNCTION_SPI1_MOSI},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_I2C1, PIN_FUNCTION_I2C1_SDATA},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_14},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_10},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_MISC, PIN_FUNCTION_RF_DISABLE_L},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_USB, PIN_FUNCTION_USB20H_CTL2},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_CLK,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 0,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_CLK},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_CLK},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_CLK},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_CMD,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 4,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_CMD},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_CMD},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_CMD},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_DATA_0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 8,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_DATA_0},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_D0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_D0},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_DATA_1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 12,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_DATA_1},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_D1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_D1},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_DATA_2,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 16,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_DATA_2},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_D2},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_D2},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SDIO_DATA_3,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_2,
        .gci_chip_ctrl_mask     = 0xF,
        .gci_chip_ctrl_pos      = 20,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_TEST_SDIO_DATA_3},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_D3},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_SDIO, PIN_FUNCTION_SDIO_AOS_D3},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_RF_SW_CTRL_5,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_11,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 27,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_RF_SW_CTRL, PIN_FUNCTION_RF_SW_CTRL_5},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GCI_GPIO, PIN_FUNCTION_GCI_GPIO_5},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_RF_SW_CTRL_6,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 10,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_RF_SW_CTRL, PIN_FUNCTION_RF_SW_CTRL_6},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UART_DBG, PIN_FUNCTION_UART_DBG_RX},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_SECI, PIN_FUNCTION_SECI_IN},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_RF_SW_CTRL_7,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 12,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_RF_SW_CTRL, PIN_FUNCTION_RF_SW_CTRL_7},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_UART_DBG, PIN_FUNCTION_UART_DBG_TX},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_SECI, PIN_FUNCTION_SECI_OUT},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_RF_SW_CTRL_8,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 14,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_RF_SW_CTRL, PIN_FUNCTION_RF_SW_CTRL_8},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SECI, PIN_FUNCTION_SECI_IN},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UART_DBG, PIN_FUNCTION_UART_DBG_RX},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_RF_SW_CTRL_9,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 16,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_RF_SW_CTRL, PIN_FUNCTION_RF_SW_CTRL_9},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_SECI, PIN_FUNCTION_SECI_OUT},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_UART_DBG, PIN_FUNCTION_UART_DBG_TX},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 0,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_2},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_18},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 18,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_3},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_19},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_2,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 20,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM2},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_4},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_20},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_3,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 22,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM3},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_5},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_21},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_4,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_3,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 24,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM4},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_6},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_22},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_PWM_5,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_5,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 7,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_PWM, PIN_FUNCTION_PWM5},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_8},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_23},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SPI_0_MISO,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_5,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 9,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SPI0, PIN_FUNCTION_SPI0_MISO},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_17},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_24},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SPI_0_CLK,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_5,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 11,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SPI0, PIN_FUNCTION_SPI0_CLK},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_18},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_25},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SPI_0_MOSI,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_5,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 13,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SPI0, PIN_FUNCTION_SPI0_MOSI},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_19},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_26},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_SPI_0_CS,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 0,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_SPI0, PIN_FUNCTION_SPI0_CS},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_20},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_27},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2C0_SDATA,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 2,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2C0, PIN_FUNCTION_I2C0_SDATA},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_21},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_28},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2C0_CLK,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 4,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2C0, PIN_FUNCTION_I2C0_CLK},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_22},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_29},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_MCLK0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 6,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_MCLK0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_23},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_0},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SCLK0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 8,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SCLK0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_24},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_2},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_LRCLK0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 10,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_LRCLK0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_25},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_3},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SDATAI0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 12,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SDATAI0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_26},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_4},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SDATAO0,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 14,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SDATAO0},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_27},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_5},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SDATAO1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 16,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SDATAO1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_28},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_6},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SDATAI1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 18,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SDATAI1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_29},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_8},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_MCLK1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 20,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_MCLK1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_30},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_17},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_SCLK1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 22,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_SCLK1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_31},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_30},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_I2S_LRCLK1,
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_9,
        .gci_chip_ctrl_mask     = 0x3,
        .gci_chip_ctrl_pos      = 24,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_I2S, PIN_FUNCTION_I2S_LRCLK1},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_0},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_GPIO, PIN_FUNCTION_GPIO_31},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_UNKNOWN, PIN_FUNCTION_UNKNOWN}
        }
    },
    {
        .pin_pad_name           = PIN_MAX,  /* Invalid PIN_MAX used as table terminator */
        .gci_chip_ctrl_reg      = GCI_CHIPCONTROL_REG_INVALID,
        .gci_chip_ctrl_mask     = GCI_CHIPCONTROL_MASK_INVALID,
        .gci_chip_ctrl_pos      = GCI_CHIPCONTROL_POS_INVALID,
        .pin_function_selection =
        {
            [PIN_FUNCTION_INDEX_0]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_1]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_2]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_3]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_4]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_5]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_6]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_7]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_8]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_9]  = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_10] = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX},
            [PIN_FUNCTION_INDEX_11] = {PIN_FUNCTION_TYPE_MAX, PIN_FUNCTION_MAX}
        }
    }
};

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t
platform_pinmux_function_get( const platform_pin_internal_config_t *pin_conf, uint32_t *pin_function_index_ptr)
{
    uint8_t  reg;
    uint8_t  pos;
    uint32_t mask;
    uint32_t reg_val;
    uint32_t val;

    if ((pin_conf == NULL) || (pin_function_index_ptr == NULL))
    {
        return PLATFORM_ERROR;
    }

    /* Initialize invalid pin function selection value */
    *pin_function_index_ptr = PIN_FUNCTION_MAX_COUNT;

    reg  = pin_conf->gci_chip_ctrl_reg;
    pos  = pin_conf->gci_chip_ctrl_pos;
    mask = GCI_REG_MASK(pin_conf->gci_chip_ctrl_mask, pos);

    /* Read from the appropriate GCI chip control register */
    reg_val = platform_gci_chipcontrol(reg, 0, 0);

    /* Get the actual function selection value for this pin */
    val = (reg_val & mask) >> pos;
    *pin_function_index_ptr = val;

    return PLATFORM_SUCCESS;
}

static platform_result_t
platform_pin_function_set(const platform_pin_internal_config_t *pin_conf, uint32_t pin_function_index)
{
    uint8_t  reg;
    uint8_t  pos;
    uint32_t mask;
    uint32_t val;

    if ((pin_conf == NULL) || (pin_function_index >= PIN_FUNCTION_MAX_COUNT))
    {
        return PLATFORM_ERROR;
    }

    reg  = pin_conf->gci_chip_ctrl_reg;
    pos  = pin_conf->gci_chip_ctrl_pos;
    mask = GCI_REG_MASK(pin_conf->gci_chip_ctrl_mask, pos);
    val  = (pin_function_index << pos) & mask;

    /* Write the pin function selection value to the appropriate GCI chip control register */
    platform_gci_chipcontrol(reg, mask, val);

    return PLATFORM_SUCCESS;
}

const platform_pin_internal_config_t *
platform_pinmux_get_internal_config(platform_pin_t pin)
{
    const platform_pin_internal_config_t *pin_conf = NULL;

    if (pin >= PIN_MAX)
    {
        return NULL;
    }

    /* Lookup the desired pin internal function configuration */
    for (pin_conf = pin_internal_config ; ((pin_conf != NULL) && (pin_conf->pin_pad_name != PIN_MAX)) ; pin_conf++)
    {
        if (pin_conf->pin_pad_name == pin)
        {
            /* Found the desired pin configuration */
            return pin_conf;
        }
    }

    return NULL;
}

platform_result_t
platform_pinmux_get_function_config(platform_pin_t pin, const platform_pin_internal_config_t **pin_conf_pp, uint32_t *pin_function_index_p)
{
    const platform_pin_internal_config_t *pin_conf_p = NULL;
    uint32_t pin_function_index = PIN_FUNCTION_MAX_COUNT;

    if ((pin_conf_pp == NULL) || (pin_function_index_p == NULL))
    {
        return PLATFORM_ERROR;
    }

    if ( pin >= PIN_MAX )
    {
        return PLATFORM_UNSUPPORTED;
    }

    *pin_conf_pp          = NULL;
    *pin_function_index_p = PIN_FUNCTION_MAX_COUNT;

    /* Lookup the desired pin internal function configuration */
    pin_conf_p = platform_pinmux_get_internal_config(pin);

    if ( (pin_conf_p == NULL) || (pin_conf_p->pin_pad_name != pin) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Read the current function index value for this pin */
    platform_pinmux_function_get(pin_conf_p, &pin_function_index);

    if (pin_function_index >= PIN_FUNCTION_MAX_COUNT)
    {
        return PLATFORM_UNSUPPORTED;
    }

    *pin_conf_pp          = pin_conf_p;
    *pin_function_index_p = pin_function_index;

    return PLATFORM_SUCCESS;
}

platform_result_t
platform_pinmux_function_init( const platform_pin_internal_config_t *pin_conf, uint32_t pin_function_index, platform_pin_config_t config)
{
    platform_result_t result;
    uint32_t pin_func_idx = PIN_FUNCTION_MAX_COUNT;
    const platform_pin_internal_config_t *pin_int_conf = NULL;

    if ((pin_conf == NULL) || (pin_function_index >= PIN_FUNCTION_MAX_COUNT))
    {
        return PLATFORM_UNSUPPORTED;
    }

    if ((pin_conf->pin_function_selection[pin_function_index].pin_function == PIN_FUNCTION_HW_DEFAULT) ||
        (pin_conf->pin_function_selection[pin_function_index].pin_function == PIN_FUNCTION_SAME_AS_PIN))
    {
        /* These pin function selection values should not be used by internal structures */
        return PLATFORM_UNSUPPORTED;
    }

    if (pin_conf->pin_function_selection[pin_function_index].pin_function != PIN_FUNCTION_UNKNOWN)
    {
        /* Check if another pin/pad has currently enabled this function */
        for (pin_int_conf = pin_internal_config ; ((pin_int_conf != NULL) && (pin_int_conf->pin_pad_name != PIN_MAX)) ; pin_int_conf++)
        {
            if (pin_int_conf->pin_pad_name != pin_conf->pin_pad_name)
            {
                /* Read the current function index value for this pin */
                pin_func_idx = PIN_FUNCTION_MAX_COUNT;
                platform_pinmux_function_get(pin_int_conf, &pin_func_idx);

                if (pin_func_idx >= PIN_FUNCTION_MAX_COUNT)
                {
                    continue;
                }

                if (pin_int_conf->pin_function_selection[pin_func_idx].pin_function == pin_conf->pin_function_selection[pin_function_index].pin_function)
                {
                    /* A function can be enabled on only one pin/pad at a given time */
                    return PLATFORM_UNSUPPORTED;
                }
            }
        }
    }

    if (pin_conf->pin_function_selection[pin_function_index].pin_function_type == PIN_FUNCTION_TYPE_GPIO)
    {
        /* Setup GPIO configuration for the pin */
        result = platform_chipcommon_gpio_init(pin_conf, pin_function_index, config);
    }
    else
    {
        /* Other function specific configuration to be added as required */
        result = PLATFORM_SUCCESS;
    }

    if (result == PLATFORM_SUCCESS)
    {
        /* Set the new pin function selection */
        result = platform_pin_function_set(pin_conf, pin_function_index);
    }

    return result;
}

platform_result_t
platform_pinmux_function_deinit( const platform_pin_internal_config_t *pin_conf, uint32_t pin_function_index)
{
    platform_result_t result;
    uint32_t pin_func_idx = PIN_FUNCTION_MAX_COUNT;

    if ((pin_conf == NULL) || (pin_function_index >= PIN_FUNCTION_MAX_COUNT))
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Read the current function index value for this pin */
    platform_pinmux_function_get(pin_conf, &pin_func_idx);

    if (pin_func_idx != pin_function_index)
    {
        return PLATFORM_UNSUPPORTED;
    }

    if (pin_conf->pin_function_selection[pin_function_index].pin_function_type == PIN_FUNCTION_TYPE_GPIO)
    {
        /* Reset GPIO configuration for the pin */
        result = platform_chipcommon_gpio_deinit(pin_conf, pin_function_index);
    }
    else
    {
        /* Reset other function specific configuration as required */
        result = PLATFORM_SUCCESS;
    }

    if (result == PLATFORM_SUCCESS)
    {
        /* Reset the pin to HW/Power-On default function selection */
        pin_func_idx = function_index_hw_default;
        result = platform_pin_function_set(pin_conf, pin_func_idx);
    }

    return result;
}

/*
 * This platform API is used to configure non-GPIO pins.
 * Use platform_gpio_init() for GPIO pin configuration instead.
 */
platform_result_t platform_pinmux_init(platform_pin_t pin, platform_pin_function_t function)
{
    platform_result_t result;
    uint32_t pin_function_index = PIN_FUNCTION_MAX_COUNT;
    const platform_pin_internal_config_t *pin_conf = NULL;

    if ( (pin >= PIN_MAX) || (function >= PIN_FUNCTION_MAX) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    if (function == PIN_FUNCTION_UNKNOWN)
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Lookup the desired pin internal function configuration */
    pin_conf = platform_pinmux_get_internal_config(pin);

    if ( (pin_conf == NULL) || (pin_conf->pin_pad_name != pin) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Determine the pin function selection value */
    if (function == PIN_FUNCTION_HW_DEFAULT)
    {
        /* HW decided - Power ON default */
        pin_function_index = function_index_hw_default;
    }
    else if (function == PIN_FUNCTION_SAME_AS_PIN)
    {
        /* Same as pin name */
        pin_function_index = function_index_same_as_pin;
    }
    else
    {
        /* Check if this pin supports the desired function selection */
        for (pin_function_index = 0 ; (pin_function_index < PIN_FUNCTION_MAX_COUNT) ; pin_function_index++)
        {
            if (pin_conf->pin_function_selection[pin_function_index].pin_function == function)
            {
                /* Found the desired function selection */
                break;
            }
        }
    }

    if (pin_function_index >= PIN_FUNCTION_MAX_COUNT)
    {
        /* The pin function index value is not valid */
        return PLATFORM_UNSUPPORTED;
    }

    /* For non GPIO pins, config parameter is not used. Hence use OUTPUT_OPEN_DRAIN_NO_PULL as dummy */
    result = platform_pinmux_function_init(pin_conf, pin_function_index, OUTPUT_OPEN_DRAIN_NO_PULL);

    return result;
}

platform_result_t platform_pinmux_deinit(platform_pin_t pin, platform_pin_function_t function)
{
    platform_result_t result;
    uint32_t pin_function_index = PIN_FUNCTION_MAX_COUNT;
    const platform_pin_internal_config_t *pin_conf = NULL;

    if ( (pin >= PIN_MAX) || (function >= PIN_FUNCTION_MAX) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    if (function == PIN_FUNCTION_UNKNOWN)
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Lookup the pin internal configuration and current function index value */
    result = platform_pinmux_get_function_config(pin, &pin_conf, &pin_function_index);

    if (result != PLATFORM_SUCCESS)
    {
        return result;
    }

    if ((pin_conf == NULL) || (pin_conf->pin_pad_name != pin) || (pin_function_index >= PIN_FUNCTION_MAX_COUNT))
    {
        return PLATFORM_UNSUPPORTED;
    }

    if (function == PIN_FUNCTION_HW_DEFAULT)
    {
        /* HW decided - Power ON default */
        if (pin_function_index != function_index_hw_default)
        {
            return PLATFORM_UNSUPPORTED;
        }
    }
    else if (function == PIN_FUNCTION_SAME_AS_PIN)
    {
        /* Same as pin name */
        if (pin_function_index != function_index_same_as_pin)
        {
            return PLATFORM_UNSUPPORTED;
        }
    }
    else
    {
        if (pin_conf->pin_function_selection[pin_function_index].pin_function != function)
        {
            return PLATFORM_UNSUPPORTED;
        }
    }

    result = platform_pinmux_function_deinit(pin_conf, pin_function_index);

    return result;
}
