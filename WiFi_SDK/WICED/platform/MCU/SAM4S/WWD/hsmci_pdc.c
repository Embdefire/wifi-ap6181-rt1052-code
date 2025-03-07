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
/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * \file
 *
 * Implementation of High Speed MultiMedia Card Interface (HSMCI) controller,
 * using PDC to transfer data.
 *
 */
/*---------------------------------------------------------------------------
 *         Headers
 *---------------------------------------------------------------------------*/

#include "sam4s.h"
#include "hsmci.h"
#include "pmc.h"
#include <assert.h>

/*---------------------------------------------------------------------------
 *         Local macros
 *---------------------------------------------------------------------------*/

/** Reset MCI */
#define MCI_RESET(pMciHw)      (pMciHw->HSMCI_CR = HSMCI_CR_SWRST)


/*---------------------------------------------------------------------------
 *         Exported functions
 *---------------------------------------------------------------------------*/

/**
 * Enable MCI
 * \param pMciHw  Pointer to a MCI peripheral.
 */
void  MCI_Enable(Hsmci *pMciHw)
{
    pMciHw->HSMCI_CR = HSMCI_CR_MCIEN;
}

/**
 * Disable MCI
 * \param pMciHw  Pointer to a MCI peripheral.
 */
void  MCI_Disable(Hsmci *pMciHw)
{
    pMciHw->HSMCI_CR = HSMCI_CR_MCIDIS;
}

/**
 * Initializes a MCI driver instance and the underlying peripheral.
 * \param pMci    Pointer to a MCI driver instance.
 * \param pMciHw  Pointer to a MCI peripheral.
 * \param mciId   MCI peripheral identifier.
 */
void MCI_Init( Mcid *pMci, Hsmci *pMciHw, uint8_t mciId, uint32_t dwMCk )
{
    unsigned long clkDiv;

    /* Initialize the MCI driver structure */
    pMci->pMciHw    = pMciHw;
    pMci->mciId     = mciId;
    pMci->semaphore = 1;
    pMci->pCommand  = NULL;

    /* Enable the MCI peripheral */
    pmc_enable_periph_clk( mciId );

    /* Reset the MCI */
    pMciHw->HSMCI_CR = HSMCI_CR_SWRST;

    /* Disable the MCI */
    pMciHw->HSMCI_CR = HSMCI_CR_MCIDIS | HSMCI_CR_PWSDIS;

    /* Disable all the interrupts */
    pMciHw->HSMCI_IDR = 0xFFFFFFFF;

    /* Set the Data Timeout Register */
    pMciHw->HSMCI_DTOR = HSMCI_DTOR_DTOCYC_Msk | HSMCI_DTOR_DTOMUL_Msk ;
    /* CSTOR ? */
    pMciHw->HSMCI_CSTOR = HSMCI_CSTOR_CSTOCYC_Msk | HSMCI_CSTOR_CSTOMUL_Msk ;

    /* Set the Mode Register: 400KHz for MCK = 48MHz (CLKDIV = 58) */
    clkDiv = (dwMCk / (MCI_INITIAL_SPEED * 2)) - 1;
    pMciHw->HSMCI_MR = ((uint8_t)clkDiv | (HSMCI_MR_PWSDIV( 0x07 )) ) ;

    /* Set the SDCard Register 1-bit, slot A */
    pMciHw->HSMCI_SDCR = HSMCI_SDCR_SDCSEL_SLOTA | HSMCI_SDCR_SDCBUS_1 ;

    /* Enable the MCI and the Power Saving */
    pMciHw->HSMCI_CR = HSMCI_CR_MCIEN;

    /* Configure MCI */
    pMciHw->HSMCI_CFG = HSMCI_CFG_FIFOMODE
                      | ((1 << 4) & HSMCI_CFG_FERRCTRL);

    /* Disable the MCI peripheral clock. */
    pmc_disable_periph_clk(mciId);
}

/**
 * Reset MCI HW interface and disable it.
 * \param keepSettings Keep old register settings, including
 *                     _MR, _SDCR, _DTOR, _CSTOR, _DMA and _CFG.
 */
void MCI_Reset(Mcid *pMci, uint8_t keepSettings)
{
    Hsmci *pMciHw = pMci->pMciHw;
    uint32_t mciDis;

    assert(pMci);
    assert(pMci->pMciHw);

    pmc_enable_periph_clk(pMci->mciId);
    mciDis = pmc_is_periph_clk_enabled( pMci->mciId );
    if (keepSettings)
    {
        uint32_t mr, sdcr, dtor, cstor;
        uint32_t cfg;
        mr    = pMciHw->HSMCI_MR;
        sdcr  = pMciHw->HSMCI_SDCR;
        dtor  = pMciHw->HSMCI_DTOR;
        cstor = pMciHw->HSMCI_CSTOR;
        cfg   = pMciHw->HSMCI_CFG;
        MCI_RESET(pMciHw);
        MCI_Disable(pMciHw);
        pMciHw->HSMCI_MR    = mr;
        pMciHw->HSMCI_SDCR  = sdcr;
        pMciHw->HSMCI_DTOR  = dtor;
        pMciHw->HSMCI_CSTOR = cstor;
        pMciHw->HSMCI_CFG   = cfg;
    }
    else
    {
        MCI_RESET(pMciHw);
        MCI_Disable(pMciHw);
    }
    if ( mciDis )
    {
//        pmc_disable_periph_clk( pMci->mciId ) ;
    }
}

/**
 * Configure the  MCI CLKDIV in the MCI_MR register. The max. for MCI clock is
 * MCK/2 and corresponds to CLKDIV = 0
 * \param pMci  Pointer to the low level MCI driver.
 * \param mciSpeed  MCI clock speed in Hz, 0 will not change current speed.
 * \param mck       MCK to generate MCI Clock, in Hz
 * \return The actual speed used, 0 for fail.
 */
uint32_t MCI_SetSpeed( Mcid* pMci, uint32_t mciSpeed, uint32_t mck )
{
    Hsmci *pMciHw = pMci->pMciHw;
    uint32_t mciMr;
    uint32_t clkdiv;
    uint32_t mciDis;

    assert(pMci);
    assert(pMciHw);

    pmc_enable_periph_clk(pMci->mciId);
    mciDis = pmc_is_periph_clk_enabled(pMci->mciId);

    mciMr = pMciHw->HSMCI_MR & (~(uint32_t)HSMCI_MR_CLKDIV_Msk);
    /* Multimedia Card Interface clock (MCCK or MCI_CK) is Master Clock (MCK)
     * divided by (2*(CLKDIV+1))
     * mciSpeed = MCK / (2*(CLKDIV+1)) */
    if (mciSpeed > 0)
    {
        clkdiv = (mck / 2 / mciSpeed);
        /* Speed should not bigger than expired one */
        if (mciSpeed < mck/2/clkdiv)
        {
            clkdiv ++;
        }

        if ( clkdiv > 0 )
        {
            clkdiv -= 1;
        }
        assert( (clkdiv & 0xFFFFFF00) == 0 ) ; /* "mciSpeed too small" */
    }
    else
    {
        clkdiv = 0 ;
    }

    /* Actual MCI speed */
    mciSpeed = mck / 2 / (clkdiv + 1);
    /* Modify MR */
    pMciHw->HSMCI_MR = mciMr | clkdiv;
    if ( mciDis )
    {
//        pmc_disable_periph_clk( pMci->mciId ) ;
    }

    return (mciSpeed);
}

/**
 * Configure the MCI_CFG to enable the HS mode
 * \param pMci     Pointer to the low level MCI driver.
 * \param hsEnable 1 to enable, 0 to disable HS mode.
 */
uint8_t MCI_EnableHsMode(Mcid* pMci, uint8_t hsEnable)
{
    Hsmci *pMciHw = pMci->pMciHw;
    uint32_t cfgr;
    uint32_t mciDis;
    uint8_t rc = 0;

    assert(pMci);
    assert(pMci->pMciHw);

    pmc_enable_periph_clk(pMci->mciId);
    mciDis = pmc_is_periph_clk_enabled(pMci->mciId);

    cfgr = pMciHw->HSMCI_CFG;
    if (hsEnable == 1)
    {
        cfgr |=  HSMCI_CFG_HSMODE;
    }
    else
    {
        if (hsEnable == 0)
        {
            cfgr &= ~(uint32_t)HSMCI_CFG_HSMODE;
        }
        else
        {
            rc = ((cfgr & HSMCI_CFG_HSMODE) != 0);
        }
    }

    pMciHw->HSMCI_CFG = cfgr;
    if (mciDis)
    {
//        pmc_disable_periph_clk(pMci->mciId);
    }

    return rc;
}

/**
 * Configure the  MCI SDCBUS in the MCI_SDCR register. Only two modes available
 *
 * \param pMci  Pointer to the low level MCI driver.
 * \param busWidth  MCI bus width mode. 00: 1-bit, 10: 4-bit, 11: 8-bit.
 */
uint32_t MCI_SetBusWidth(Mcid*pMci, uint32_t busWidth)
{
    Hsmci *pMciHw = pMci->pMciHw;
    uint32_t mciSdcr;
    uint32_t mciDis;

    assert(pMci);
    assert(pMci->pMciHw);

    if( (busWidth != HSMCI_SDCR_SDCBUS_1) && (busWidth != HSMCI_SDCR_SDCBUS_4) && (busWidth != HSMCI_SDCR_SDCBUS_8) )
    {
        return (uint32_t)-1;
    }

    busWidth &= HSMCI_SDCR_SDCBUS_Msk ;

    pmc_enable_periph_clk(pMci->mciId);
    mciDis = pmc_is_periph_clk_enabled(pMci->mciId);

    mciSdcr = (pMciHw->HSMCI_SDCR & ~(uint32_t)(HSMCI_SDCR_SDCBUS_Msk));
    pMciHw->HSMCI_SDCR = mciSdcr | busWidth;

    if (mciDis)
    {
//        pmc_disable_periph_clk(pMci->mciId);
    }

    return 0;
}

