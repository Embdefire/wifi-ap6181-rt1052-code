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
 *
 */

#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "core_cmFunc.h"
#include "platform_init.h"
#include "platform_constants.h"
#include "platform_assert.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "platform_sleep.h"
#include "wwd_rtos_isr.h"
#include "wiced_defaults.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define RTT_CLOCK_PRESCALER          (4) /*                                                                                        */
#define RTT_CLOCK_HZ             (32768) /* 32K / prescaler                                                                        */
#define RTT_MAX_CYCLES           (64000) /* 7.8125 seconds                                                                         */
#define RC_OSC_DELAY_CYCLES         (15) /* Cycles required to stabilise clock after exiting WAIT mode                             */
#define JTAG_DEBUG_SLEEP_DELAY_MS (3000) /* Delay in ms to give OpenOCD enough time to halt the CPU before the CPU enter WAIT mode */

/******************************************************
 *                      Macros
 ******************************************************/

#define CYCLES_TO_MS( cycles ) ( ( ( cycles ) * 1000 * RTT_CLOCK_PRESCALER ) / RTT_CLOCK_HZ )
#define MS_TO_CYCLES( ms )     ( ( ( ms ) * RTT_CLOCK_HZ ) / ( 1000 * RTT_CLOCK_PRESCALER ) )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static unsigned long  wait_mode_power_down_hook        ( unsigned long delay_ms );
extern void           init_clocks                      ( void );
extern void           RTT_irq                          ( void );
#else /* WICED_DISABLE_MCU_POWERSAVE */
static unsigned long  idle_power_down_hook             ( unsigned long delay_ms );
#endif /* WICED_DISABLE_MCU_POWERSAVE */

/******************************************************
 *               Variable Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static volatile uint32_t     system_io_backup_value      = 0;
static volatile uint32_t     sam4s_clock_needed_counter  = 0;
static volatile wiced_bool_t wake_up_interrupt_triggered = WICED_FALSE;
#endif /* WICED_DISABLE_MCU_POWERSAVE */

/******************************************************
 *            WICED Function Definitions
 ******************************************************/


/******************************************************
 *            SAM4S Function Definitions
 ******************************************************/

platform_result_t platform_mcu_powersave_init( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE

    /* Initialise all pins to be input pull-up to save power */
    ioport_enable_port( IOPORT_PIOA,   0xffffffffU );
    ioport_set_port_mode( IOPORT_PIOA, 0xffffffffU, IOPORT_MODE_PULLUP );
    ioport_set_port_dir( IOPORT_PIOA,  0xffffffffU, IOPORT_DIR_INPUT );

    ioport_enable_port( IOPORT_PIOB,   0xffffffffU );
    ioport_set_port_mode( IOPORT_PIOB, 0xffffffffU, IOPORT_MODE_PULLUP );
    ioport_set_port_dir( IOPORT_PIOB,  0xffffffffU, IOPORT_DIR_INPUT );

#ifdef ID_PIOC
    /* On setup on SAM4S variants which PIOC is avaiable */
    ioport_enable_port( IOPORT_PIOC,   0xffffffffU );
    ioport_set_port_mode( IOPORT_PIOC, 0xffffffffU, IOPORT_MODE_PULLUP );
    ioport_set_port_dir( IOPORT_PIOC,  0xffffffffU, IOPORT_DIR_INPUT );
#endif

    NVIC_DisableIRQ( RTT_IRQn );
    NVIC_ClearPendingIRQ( RTT_IRQn );
    NVIC_SetPriority( RTT_IRQn, 0 );
    NVIC_EnableIRQ( RTT_IRQn );
    pmc_set_fast_startup_input( PMC_FSMR_RTTAL );

#endif /* WICED_DISABLE_MCU_POWERSAVE */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_mcu_powersave_disable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Atomic operation starts */
    WICED_DISABLE_INTERRUPTS();

    /* Increment counter to indicate CPU clock is needed preventing CPU from entering WAIT mode */
    sam4s_clock_needed_counter++;

    /* Atomic operation ends */
    WICED_ENABLE_INTERRUPTS();
#endif /* WICED_DISABLE_MCU_POWERSAVE */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_mcu_powersave_enable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Atomic operation starts */
    WICED_DISABLE_INTERRUPTS();

    /* Decrement counter only if it's not 0 */
    sam4s_clock_needed_counter -= ( sam4s_clock_needed_counter > 0 ) ? 1 : 0;

    /* Atomic operation ends */
    WICED_ENABLE_INTERRUPTS();
#endif /* WICED_DISABLE_MCU_POWERSAVE */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_powersave_enable_wakeup_pin( const platform_wakeup_pin_config_t* config )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    if ( config->is_wakeup_pin == WICED_TRUE )
    {
        /* Enable wake-up pin */
        pmc_set_fast_startup_input( 1 << config->wakeup_pin_number );

        if ( config->trigger == IOPORT_SENSE_RISING )
        {
            /* Set polarity of corresponding wake-up pin bit to active-high */
            PMC->PMC_FSPR |=  (uint32_t)( 1 << config->wakeup_pin_number );
        }
        else
        {
            /* Set polarity of corresponding wake-up pin bit to active-low */
            PMC->PMC_FSPR &= ~(uint32_t)( 1 << config->wakeup_pin_number );
        }

        return PLATFORM_SUCCESS;
    }
    else
    {
        return PLATFORM_ERROR;
    }
#endif /* WICED_DISABLE_MCU_POWERSAVE */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_powersave_disable_wakeup_pin( const platform_wakeup_pin_config_t* config )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    if ( config->is_wakeup_pin == WICED_TRUE )
    {
        pmc_clr_fast_startup_input( 1 << config->wakeup_pin_number );
        return PLATFORM_SUCCESS;
    }
    else
    {
        return PLATFORM_ERROR;
    }
#endif /* WICED_DISABLE_MCU_POWERSAVE */
    return PLATFORM_SUCCESS;
}

void platform_mcu_powersave_exit_notify( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    wake_up_interrupt_triggered = WICED_TRUE;
#endif /* WICED_DISABLE_MCU_POWERSAVE */
}

/******************************************************
 *      RTOS Powersave Hook Function Definitions
 ******************************************************/

unsigned long platform_power_down_hook( unsigned long delay_ms )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    return wait_mode_power_down_hook( delay_ms );
#else /* WICED_DISABLE_MCU_POWERSAVE */
    return idle_power_down_hook( delay_ms );
#endif /* WICED_DISABLE_MCU_POWERSAVE */
}



void platform_idle_hook( void )
{
    __asm("wfi");
}

/******************************************************
 *            Static Function Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static unsigned long wait_mode_power_down_hook( unsigned long delay_ms )
{
    wiced_bool_t jtag_enabled       = ( ( CoreDebug ->DHCSR & CoreDebug_DEMCR_TRCENA_Msk ) != 0 ) ? WICED_TRUE : WICED_FALSE;
    wiced_bool_t jtag_delay_elapsed = ( host_rtos_get_time() > JTAG_DEBUG_SLEEP_DELAY_MS ) ? WICED_TRUE : WICED_FALSE;
    uint32_t     elapsed_cycles     = 0;
    uint32_t     counter_read_v1 = 0, counter_read_v2 = 0;

    /* Criteria to enter WAIT mode
     * 1. Clock needed counter is 0 and no JTAG debugging
     * 2. Clock needed counter is 0, in JTAG debugging session, and WICED system tick has progressed over 3 seconds.
     *    This is to give OpenOCD enough time to poke the JTAG tap before the CPU enters WAIT mode.
     */
    if ( ( sam4s_clock_needed_counter == 0 ) && ( ( jtag_enabled == WICED_FALSE ) || ( ( jtag_enabled == WICED_TRUE ) && ( jtag_delay_elapsed == WICED_TRUE ) ) ) )
    {
        uint32_t total_sleep_cycles;
        uint32_t total_delay_cycles;

        /* Enable RTT */
        rtt_enable( RTT );
        /* select source */
        rtt_sel_source( RTT, false );
        /* Clear all interrupt sources */
        rtt_disable_interrupt( RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN );
        /* Start real-time timer */
        rtt_init( RTT, RTT_CLOCK_PRESCALER );

        /* Start atomic operation */
        WICED_DISABLE_INTERRUPTS();

        /* Ensure deep sleep bit is enabled, otherwise system doesn't go into deep sleep */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Disable SysTick */
        SysTick->CTRL &= ( ~( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk ) );

        /* End atomic operation */
        WICED_ENABLE_INTERRUPTS();

        /* Expected total time CPU executing in this function (including WAIT mode time) */
        total_sleep_cycles = MS_TO_CYCLES( delay_ms );

        /* Total cycles in WAIT mode loop */
        total_delay_cycles = ( total_sleep_cycles / RTT_MAX_CYCLES + 1 ) * RC_OSC_DELAY_CYCLES + WAIT_MODE_ENTER_DELAY_CYCLES + WAIT_MODE_EXIT_DELAY_CYCLES;

        if ( total_sleep_cycles > total_delay_cycles )
        {
            /* Adjust total sleep cycle to exclude exit delay */
            total_sleep_cycles -= WAIT_MODE_EXIT_DELAY_CYCLES;

            /* Prepare platform specific settings before entering powersave */
//            platform_enter_powersave();

            /* Prepare WLAN bus before entering powersave */
            platform_bus_enter_powersave();

            /* Disable brownout detector */
            supc_disable_brownout_detector( SUPC );

            /* Backup system I/0 functions and set all to GPIO to save power */
            system_io_backup_value = matrix_get_system_io();
            matrix_set_system_io( 0x0CF0 );

            /* Switch Master Clock to Main Clock (internal fast RC oscillator) */
            /* Switch on internal fast RC oscillator, switch Main Clock source to internal fast RC oscillator and disables external fast crystal */
            pmc_switch_mck_to_sclk(PMC_MCKR_PRES_CLK_1);
            pmc_switch_mainck_to_fastrc(CKGR_MOR_MOSCRCF_4_MHz);
            pmc_switch_mck_to_mainck(PMC_PCK_PRES_CLK_1);

            /* Disable external fast crystal */
            pmc_osc_disable_xtal( 0 );

            /* Disable PLLA */
            pmc_disable_pllack( );

            /* This above process introduces certain delay. Add delay to the elapsed cycles */
            do {
                counter_read_v1 = rtt_read_timer_value( RTT );
                counter_read_v2 = rtt_read_timer_value( RTT );
            } while (counter_read_v1 != counter_read_v2);
            elapsed_cycles += (counter_read_v2 * RTT_CLOCK_PRESCALER);

            while ( wake_up_interrupt_triggered == WICED_FALSE  && elapsed_cycles < total_sleep_cycles )
            {
                uint32_t current_sleep_cycles = total_sleep_cycles - elapsed_cycles;

                /* Start real-time timer and alarm */
                rtt_init( RTT, RTT_CLOCK_PRESCALER );
                /* From Atmel reference code */
                counter_read_v1 = rtt_read_timer_value(RTT);
                while (counter_read_v1 == rtt_read_timer_value(RTT));
                counter_read_v2 = counter_read_v1 = 0;
                rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
                rtt_write_alarm_time( RTT, ( current_sleep_cycles > RTT_MAX_CYCLES ) ? RTT_MAX_CYCLES - RC_OSC_DELAY_CYCLES : current_sleep_cycles - RC_OSC_DELAY_CYCLES );

                /* Enter WAIT mode */
                pmc_enable_waitmode();

                /* Clear wake-up status */
                rtt_get_status( RTT );

                /* Add sleep time to the elapsed cycles */
                do {
                    counter_read_v1 = rtt_read_timer_value( RTT );
                    counter_read_v2 = rtt_read_timer_value( RTT );
                } while (counter_read_v1 != counter_read_v2);
                elapsed_cycles += (counter_read_v2 * RTT_CLOCK_PRESCALER);
                counter_read_v2 = counter_read_v1 = 0;
            }

            /* Re-enable real-time timer to time clock reinitialisation delay */
            rtt_disable_interrupt( RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN );
            rtt_init( RTT, RTT_CLOCK_PRESCALER );

            /* Reinit fast clock. This takes ~19ms, but the timing has been compensated */
            pmc_switch_mck_to_sclk(PMC_MCKR_PRES_CLK_1);

            /* Switch mainck to external xtal using default values */
            pmc_switch_mainck_to_xtal(0, 15625UL);

            sysclk_init();

            /* Disable unused clock to save power */
            pmc_osc_disable_fastrc();

            /* Restore system I/O pins */
            matrix_set_system_io( system_io_backup_value );

            /* Restore WLAN bus */
            platform_bus_exit_powersave();

//            /* Restore platform-specific settings */
//            platform_exit_powersave();

            /* Add clock reinitialisation delay to elapsed cycles */
            do {
                counter_read_v1 = rtt_read_timer_value( RTT );
                counter_read_v2 = rtt_read_timer_value( RTT );
            } while (counter_read_v1 != counter_read_v2);
            elapsed_cycles += (counter_read_v2 * RTT_CLOCK_PRESCALER);
            counter_read_v2 = counter_read_v1 = 0;

            /* Disable RTT to save power */
            rtt_disable ( RTT );
        }
    }

    /* Start atomic operation */
    WICED_DISABLE_INTERRUPTS();

    /* Switch SysTick back on */
    SysTick->CTRL |= ( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );

    /* Clear flag indicating interrupt triggered by wake up pin */
    wake_up_interrupt_triggered = WICED_FALSE;

    /* End atomic operation */
    WICED_ENABLE_INTERRUPTS();

    /* Return total time in milliseconds */
    return CYCLES_TO_MS( elapsed_cycles );
}
#else /* WICED_DISABLE_MCU_POWERSAVE */
static unsigned long idle_power_down_hook( unsigned long delay_ms  )
{
    UNUSED_PARAMETER( delay_ms );
    WICED_ENABLE_INTERRUPTS();
    __asm("wfi");
    return 0;
}
#endif /* WICED_DISABLE_MCU_POWERSAVE */


/******************************************************
 *        Interrupt Service Routine Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
WWD_RTOS_DEFINE_ISR(rtc_wakeup_irq)
{
     uint32_t ul_status;

     /* Get RTT status */
     ul_status = rtt_get_status(RTT);

     if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
     }

     /* Alarm */
     if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
     }
}
WWD_RTOS_MAP_ISR( rtc_wakeup_irq, RTT_irq )
#endif /* WICED_DISABLE_MCU_POWERSAVE */


