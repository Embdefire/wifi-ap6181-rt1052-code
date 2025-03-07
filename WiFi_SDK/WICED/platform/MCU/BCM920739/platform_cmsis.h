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

/** @file
 * Defines BCM439x CMSIS data types and defines
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define __MPU_PRESENT             0
#define __NVIC_PRIO_BITS          4 /* 4 bits for the priority levels */
#define __Vendor_SysTickConfig    0
#define __CM3_REV                 0x0200

typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn          = -14,    /*!< 2 Non Maskable Interrupt                             */
  MemoryManagement_IRQn        = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn                = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn              = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                  = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn            = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                  = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                 = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                   */

  Reserved016_IRQn             = 0,
  Reserved017_IRQn             = 1,
  Reserved018_IRQn             = 2,
  Reserved019_IRQn             = 3,
  Reserved020_IRQn             = 4,
  Reserved021_IRQn             = 5,
  Reserved022_IRQn             = 6,
  Reserved023_IRQn             = 7,
  Reserved024_IRQn             = 8,
  Reserved025_IRQn             = 9,
  Reserved026_IRQn             = 10,
  Reserved027_IRQn             = 11,
  Reserved028_IRQn             = 12,
  Reserved029_IRQn             = 13,
  Reserved030_IRQn             = 14,
  PTU1_IRQn                    = 15,
  DmaDoneInt_IRQn              = 16,
  Reserved033_IRQn             = 17,
  Reserved034_IRQn             = 18,
  Wake_Up_IRQn                 = 19,
  GPIOA_BANK0_IRQn             = 20,
  Reserved037_IRQn             = 21,
  Reserved038_IRQn             = 22,
  Reserved039_IRQn             = 23,
  Reserved040_IRQn             = 24,
  GPIOA_BANK1_IRQn             = 25,
  Reserved042_IRQn             = 26,
  Reserved043_IRQn             = 27,
  Reserved044_IRQn             = 28,
  Reserved045_IRQn             = 29,
  Reserved046_IRQn             = 30,
  Reserved047_IRQn             = 31,
  Reserved048_IRQn             = 32,
  Reserved049_IRQn             = 33,
  Reserved050_IRQn             = 34,
  Reserved051_IRQn             = 35,
  Reserved052_IRQn             = 36,
  Reserved053_IRQn             = 37,
  Reserved054_IRQn             = 38,
  Reserved055_IRQn             = 39,
  Reserved056_IRQn             = 40,
  Reserved057_IRQn             = 41,
  Reserved058_IRQn             = 42,
  Reserved059_IRQn             = 43,
  Reserved060_IRQn             = 44,
  WL2APPS_IRQn                 = 45,
  WlanReady_IRQn               = 46,
  Reserved063_IRQn             = 47,
  PTU2_IRQn                    = 48,

} IRQn_Type;

#include "auto_flags.h"
#include "core_cm4.h"


#ifdef __cplusplus
} /*extern "C" */
#endif
