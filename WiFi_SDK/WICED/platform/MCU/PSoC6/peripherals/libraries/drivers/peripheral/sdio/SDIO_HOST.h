#if !defined(CY_SDIO_H)
#define CY_SDIO_H
/***************************************************************************//**
* \file cy_sdio_host.h
* \version 1.00
*
* \brief
*  This file provides types definition, constants and funciton definition for 
*  the SDIO driver.
*
*******************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************
* \addtogroup group_udb_sdio_changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*******************************************************************************/

/**
* \defgroup group_udb_sdio UDB_SDIO
* \{
*  SDIO - Secure Digital Input Output Is a standard for communicating with various
    external devices such as Wifi and bluetooth devices. 
* \{
* \defgroup group_udb_sdio_general_description General Description
* \defgroup group_udb_sdio_changelog Changelog
* \}
* \defgroup group_udb_sdio_API API Reference
* \{
* \defgroup group_udb_sdio_macro Macro
* \defgroup group_udb_sdio_functions Functions
* \defgroup group_udb_sdio_data_structures Data Structures
* \}
*/

/**
* \addtogroup group_udb_sdio_general_description
* \section group_udb_sdio_section_overview Overview
*  This driver is currently designed to only support communication with certain
*  Broadcom Wifi and Bluetooth chipsets, it is not desinged to work with a genearl
*  SDIO card, or even and SD card. Consult TDU#315 for information on limitations
*
* \section group_udb_sdio_section_configuration_considerations Configuration Considerations
* Features:
* * Always Four Wire Mode
* * Supports Card Interrupt
* * Uses DMA for command and data transfer 
**
* \section group_udb_sdio_section_more_information More Information
*
*/

#include "project.h"
    
#if defined(__cplusplus)
extern "C" {
#endif


/***************************************
*            API Constants
***************************************/

#define SDIO_CMD_TIMEOUT                (100000u)    /**< Value used for firmware timeout*/
#define SDIO_DAT_TIMEOUT                (500000u) /**< Value used for firmware timeout*/
#define SDIO_SRC_CLK_FREQ_HZ            (10000000u)  /**< Frequency of CLK_PERI*/
#define SDIO_ENABLE_CNT                 (0x20u)      /**< Bit to set in Aux Ctrl reg to enable 7 bit counters.*/

/*!
\defgroup group_sdio_cmd_constants Constants for the command channel
*/
/* @{*/
#define SDIO_HOST_DIR                    (0x40u)    /**< Direction bit set in command */
#define SDIO_CMD_END_BIT                 (0x01u)    /**< End bit set in command*/
#define SDIO_NUM_CMD_BYTES               (6u)       /**< Number of command bytes to send*/
#define SDIO_NUM_RESP_BYTES              (6u)       /**< Number of response bytes to receive*/
/*@} group_sdio_cmd_constants */

/*!
\defgroup group_sdio_ctrl_reg SDIO control register bits
*/
/* @{*/
#define SDIO_CTRL_INT_CLK                (0x01u)    /**< Enable the internal clock running the SDIO block*/
#define SDIO_CTRL_SD_CLK                 (0x02u)    /**< Enable the the SD Clock*/
#define SDIO_CTRL_ENABLE_WRITE           (0x04u)    /**< Enable a write, should not be set if ENABLE_READ is set*/
#define SDIO_CTRL_ENABLE_READ            (0x08u)    /**< Enable a read, should not be set if ENABLE_WRITE is set*/
#define SDIO_CTRL_SKIP_RESPONSE          (0x10u)    /**< If set no response is required for the command*/
#define SDIO_CTRL_RESET                  (0x20u)    /**< If set the SDIO interface is reset*/
#define SDIO_CTRL_RESET_DP               (0x40u)    /**< If set the SDIO interface is reset*/
#define SDIO_CTRL_ENABLE_INT             (0x80u)    /**< Enables logic to detect card interrupt*/
/*@} group_sdio_ctrl_reg */

/*!
\defgroup group_sdio_status_reg SDIO status register bits
*/
/* @{*/
#define SDIO_STS_CMD_DONE                (0x01u)    /**< The command is done*/
#define SDIO_STS_WRITE_DONE              (0x02u)    /**< All data for a write has been sent*/
#define SDIO_STS_READ_DONE               (0x04u)    /**< All data for a read has been read*/
#define SDIO_STS_CRC_ERR                 (0x08u)    /**< A CRC error was detected during a read or write*/
#define SDIO_STS_CMD_IDLE                (0x10u)    /**< The command channel is idle*/
#define SDIO_STS_DAT_IDLE                (0x20u)    /**< The data channel is idle*/
#define SDIO_STS_CARD_INT                (0x40u)    /**< The SDIO card indicated an interrupt by driving DAT[1] low*/
/*@} group_sdio_status_reg */

/*!
\defgroup group_sdio_crc Constants for 7bit CRC for command
*/
/* @{*/
#define SDIO_CRC7_POLY                   (0x12u)    /** < Value of CRC polynomial*/
#define SDIO_CRC_UPPER_BIT               (0x80u)    /** < Upper bit to test if it is high*/
/*@} group_sdio_crc */


/***************************************
*       Type Definitions
***************************************/

/**
* \addtogroup group_sdio_data_structures
* \{
*/

/**
* Create a type for the card interrupt call back
*/
typedef void (* sdio_card_int_cb_t)(void);

/**
* \brief This enum is used when checking for specific events
*/
typedef enum en_sdio_event
{
    SdCmdEventCmdDone           = (1u),     /**< Check to see if a command is done*/         
    SdCmdEventTransferDone      = (2u)      /**< Check to see if a transfer is done*/
    
}en_sdio_event_t;

/**
* \brief Used to indicate the result of a function
*/
typedef enum en_sdio_result
{
    Ok                          = 0x00,     /**< No error*/ 
    Error                       = 0x01,     /**< Non-specific error code*/ 
    CommandCrcError             = 0x02,     /**< There was a CRC error on the Command/Response*/ 
    CommandIdxError             = 0x04,     /**< The index for the command didn't match*/ 
    CommandEndError             = 0x08,     /**< There was an end bit error on the command*/ 
    DataCrcError                = 0x10,     /**< There was a data CRC Error*/ 
    CMDTimeout                  = 0x20,     /**< The command didn't finish before the timeout period was over*/
    DataTimeout                 = 0x40,     /**< The data didn't finish before the timeout period was over*/
    ResponseFlagError           = 0x80      /**< There was an error in the resposne flag for command 53*/ 
    
} en_sdio_result_t;

/**
* \brief Flags used to indicate an event occured, set in the interrupt, cleared in the check events funciton
*/
typedef struct stc_sdcmd_event_flag
{
    uint8_t     u8CmdComplete;              /**< If non-zero a command has completed*/
    uint8_t     u8TransComplete;            /**< If non-zero a transfer has completed*/
    uint8_t     u8CRCError;                 /**< If non-zero a CRC error was detected in a data transfer*/
    
}stc_sdio_event_flag_t;

/**
* \brief Holds pointers to callback functions
*/
typedef struct stc_sdio_irq_cb
{
    sdio_card_int_cb_t pfnCardIntCb;            /**< Pointer to card interrupt callback function*/
}stc_sdio_irq_cb_t;

/**
* \brief Global structure used to hold data from interrupt and other functions
*/
typedef struct stc_sdio_gInternalData
{
    stc_sdio_irq_cb_t       pstcCallBacks;      /**< Holds pointers to all the call back functions*/
    stc_sdio_event_flag_t   stcEvents;          /**< Holds all of the event count flags, set in interrupt used in check events*/
}stc_sdio_gInternalData_t;

/**
* \brief structure used for configuring command
*/
typedef struct stc_sdio_cmd_config
{
    uint8_t                 u8CmdIndex;           /**< Command index*/
    uint32_t                u32Argument;          /**< The argument of command */
    uint8_t                 bResponseRequired;    /**< TRUE: A Response is required*/
    uint8_t                 *pu8ResponseBuf;      /**< Pointer to location to store response*/   
    
}stc_sdio_cmd_config_t;

/**
* \brief structure used for the data channel
*/
typedef struct stc_sdio_data_config
{
    uint8_t     bRead;                    /**< TRUE: Read, FALSE: write*/
    uint16_t    u16BlockSize;             /**< Block size*/
    uint16_t    u16BlockCount;            /**< Holds the number of blocks to send*/
    uint8_t     *pu8Data;                 /**< Pointer data buffer*/
    
}stc_sdio_data_config_t;

/**
* \brief strucutre used for configuring command and data
*/
typedef struct stc_sdio_cmd
{
    uint32_t                u32CmdIdx;          /**< Command index*/
    uint32_t                u32Arg;             /**< The argument of command*/
    uint32_t                *pu32Response;      /**< Pointer to location to store response*/
    uint8_t                 *pu8Data;           /**< Pointer data buffer*/
    uint8_t                 bRead;              /**< TRUE: Read, FALSE: write*/
    uint16_t                u16BlockCnt;        /**< Number of blocks to send*/  
    uint16_t                u16BlockSize;       /**< Block size*/

}stc_sdio_cmd_t;

/** \} group_sdio_data_structures */

/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_sdio_functions
* \{
*/

/* Main functions*/ 
void                SDIO_Init(stc_sdio_irq_cb_t* pfuCb);
en_sdio_result_t    SDIO_SendCommandAndWait(stc_sdio_cmd_t *pstcCmd);
void                SDIO_EnableIntClock(void);
void                SDIO_DisableIntClock(void);
void                SDIO_EnableSdClk(void);
void                SDIO_DisableSdClk(void);
void                SDIO_SetSdClkFrequency(uint32_t u32SdClkFreqHz);
void                SDIO_Reset(void);
void                SDIO_EnableChipInt(void);
void                SDIO_DisableChipInt(void);

/*Low Level Functions*/
void                SDIO_SendCommand(stc_sdio_cmd_config_t *pstcCmdConfig);
en_sdio_result_t    SDIO_GetResponse(uint8_t bCmdIndexCheck, uint8_t bCmdCrcCheck, uint8_t u8CmdIdx, uint32_t* pu32Response, uint8_t* pu8ResponseBuf);
void                SDIO_InitDataTransfer(stc_sdio_data_config_t *pstcDataConfig);
en_sdio_result_t    SDIO_CheckForEvent(en_sdio_event_t enEventType);
uint8_t             SDIO_CalculateCrc7(uint8_t* pu8Data, uint8_t pu8Size);
void                SDIO_SetBlockSize(uint8_t u8ByteCount);
void                SDIO_SetNumBlocks(uint8_t u8BlockCount);
en_sdio_result_t    SDIO_CheckReadCRC(void);

/*DMA setup funciton*/
void                SDIO_SetupDMA(void);

/*Interrupt Function*/
void                SDIO_IRQ(void);
void                SDIO_READ_DMA_IRQ(void);
void                SDIO_WRITE_DMA_IRQ(void);

void                SDIO_Crc7Init(void);


/***************************************
*       Hardware Registers
***************************************/

#define SDIO_CONTROL_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_CtrlReg__CONTROL_REG)

#define SDIO_CONTROL_PTR       (  (reg8 *) \
SDIO_HOST_bSDIO_CtrlReg__CONTROL_REG)

#define SDIO_STATUS_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_StatusReg__STATUS_REG)

#define SDIO_STATUS_PTR       (  (reg8 *) \
SDIO_HOST_bSDIO_StatusReg__STATUS_REG)

#define SDIO_STATUS_INT_MSK   (* (reg8*) \
SDIO_HOST_bSDIO_StatusReg__MASK_REG)

#define SDIO_STATUS_AUX_CTL   (* (reg8 *) \
SDIO_HOST_bSDIO_StatusReg__STATUS_AUX_CTL_REG)

#define SDIO_CMD_BIT_CNT_CONTROL_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_cmdBitCounter__CONTROL_AUX_CTL_REG)

#define SDIO_CMD_BIT_CNT_CONTROL_PTR   ( (reg8 *) \
SDIO_HOST_bSDIO_cmdBitCounter__CONTROL_AUX_CTL_REG)

#define SDIO_WRITE_CRC_CNT_CONTROL_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_writeCrcCounter__CONTROL_AUX_CTL_REG)

#define SDIO_WRITE_CRC_CNT_CONTROL_PTR   ( (reg8 *) \
SDIO_HOST_bSDIO_writeCrcCounter__CONTROL_AUX_CTL_REG)

#define SDIO_BYTE_CNT_CONTROL_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_byteCounter__CONTROL_AUX_CTL_REG)

#define SDIO_BYTE_CNT_CONTROL_PTR   ( (reg8 *) \
SDIO_HOST_bSDIO_byteCounter__CONTROL_AUX_CTL_REG)

#define SDIO_CRC_BIT_CNT_CONTROL_REG   (* (reg8 *) \
SDIO_HOST_bSDIO_crcBitCounter__CONTROL_AUX_CTL_REG)

#define SDIO_CRC_BIT_CNT_CONTROL_PTR   ( (reg8 *) \
SDIO_HOST_bSDIO_crcBitCounter__CONTROL_AUX_CTL_REG)

#define SDIO_DATA_BLOCK_COUNTER_A0_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__A0_REG)

#define SDIO_DATA_BLOCK_COUNTER_A0_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__A0_REG)

#define SDIO_DATA_BLOCK_COUNTER_D0_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__D0_REG)

#define SDIO_DATA_BLOCK_COUNTER_D0_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__D0_REG)

#define SDIO_DATA_BLOCK_COUNTER_A1_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__A1_REG)

#define SDIO_DATA_BLOCK_COUNTER_A1_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__A1_REG)

#define SDIO_DATA_BLOCK_COUNTER_D1_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__D1_REG)

#define SDIO_DATA_BLOCK_COUNTER_D1_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_blockCounter_u0__D1_REG)

#define SDIO_CMD_COMMAND_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_CMD__F0_REG)

#define SDIO_CMD_COMMAND_A0_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_CMD__A0_REG)

#define SDIO_CMD_COMMAND_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_CMD__F0_REG)

#define SDIO_CMD_RESPONSE_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_CMD__F1_REG)

#define SDIO_CMD_RESPONSE_PTR     ( (reg8 *) \
SDIO_HOST_bSDIO_CMD__F1_REG)

#define SDIO_DAT_WRITE_REG     (* (reg16 *) \
SDIO_HOST_bSDIO_Write_DP__F0_F1_REG)

#define SDIO_DAT_WRITE_PTR     ( (reg16 *) \
SDIO_HOST_bSDIO_Write_DP__F0_F1_REG)

#define SDIO_DAT_READ_REG     (* (reg16 *) \
SDIO_HOST_bSDIO_Read_DP__F0_F1_REG)

#define SDIO_DAT_READ_PTR     ( (reg16 *) \
SDIO_HOST_bSDIO_Read_DP__F0_F1_REG)

#define SDIO_BYTE_COUNT_REG     (* (reg8 *) \
SDIO_HOST_bSDIO_byteCounter__PERIOD_REG)

#if defined(__cplusplus)
}
#endif

#endif /* (CY_SDIO_H) */

/* [] END OF FILE */
