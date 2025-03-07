/**
  ******************************************************************************
  * @file    stm32h7xx_hal_fdcan.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-August-2017
  * @brief   FDCAN HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Flexible DataRate Controller Area Network
  *          (FDCAN) peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Configuration and Control functions
  *           + Peripheral State and Error functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
      (#) Initialize the FDCAN peripheral using HAL_FDCAN_Init function.

      (#) If needed , configure the reception filters and optional features using
          the following configuration functions:
            (++) HAL_FDCAN_ConfigClockCalibration
            (++) HAL_FDCAN_ConfigFilter
            (++) HAL_FDCAN_ConfigGlobalFilter
            (++) HAL_FDCAN_ConfigExtendedIdMask
            (++) HAL_FDCAN_ConfigRxFifoOverwrite
            (++) HAL_FDCAN_ConfigFifoWatermark
            (++) HAL_FDCAN_ConfigRamWatchdog
            (++) HAL_FDCAN_ConfigTimestampCounter
            (++) HAL_FDCAN_EnableTimestampCounter
            (++) HAL_FDCAN_DisableTimestampCounter
            (++) HAL_FDCAN_ConfigTimeoutCounter
            (++) HAL_FDCAN_EnableTimeoutCounter
            (++) HAL_FDCAN_DisableTimeoutCounter
            (++) HAL_FDCAN_ConfigTxDelayCompensation
            (++) HAL_FDCAN_EnableTxDelayCompensation
            (++) HAL_FDCAN_DisableTxDelayCompensation
            (++) HAL_FDCAN_TT_ConfigOperation
            (++) HAL_FDCAN_TT_ConfigReferenceMessage
            (++) HAL_FDCAN_TT_ConfigTrigger

      (#) Start the FDCAN module using HAL_FDCAN_Start function. At this level
          the node is active on the bus: it can send and receive messages.

      (#) The following Tx control functions can only be called when the FDCAN
          module is started:
            (++) HAL_FDCAN_AddMessageToTxFifoQ
            (++) HAL_FDCAN_EnableTxBufferRequest
            (++) HAL_FDCAN_AbortTxRequest

      (#) When a message is received into the FDCAN message RAM, it can be
          retrieved using the HAL_FDCAN_GetRxMessage function.

      (#) Calling the HAL_FDCAN_Stop function stops the FDCAN module by entering
          it to initialization mode and re-enabling access to configuration
          registers through the configuration functions listed here above.

      (#) All other control functions can be called any time after initialization
          phase, no matter if the FDCAN module is started or stoped.

      *** Polling mode operation ***
      ==============================

    [..]
        (#) Reception and transmission states can be monitored via the following
            functions:
              (++) HAL_FDCAN_IsRxBufferMessageAvailable
              (++) HAL_FDCAN_IsTxBufferMessagePending
              (++) HAL_FDCAN_GetRxFifoFillLevel
              (++) HAL_FDCAN_GetTxFifoFreeLevel

      *** Interrupt mode operation ***
      ================================
      [..]
        (#) There are two interrupt lines: line 0 and 1.
            By default, all interrupts are assigned to line 0. Interrupt lines
            can be configured using HAL_FDCAN_ConfigInterruptLines function.

        (#) Notifications are activated using HAL_FDCAN_ActivateNotification
            function. Then, the process can be controlled through one of the
            available user callbacks: HAL_FDCAN_xxxCallback.

  @endverbatim

  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "../../../../STM32H7xx/peripherals/libraries/inc/stm32h7xx_hal.h"

/** @addtogroup STM32H7xx_HAL_Driver
  * @{
  */

/** @defgroup FDCAN FDCAN
  * @brief FDCAN HAL module driver
  * @{
  */

#ifdef HAL_FDCAN_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @addtogroup FDCAN_Private_Constants
  * @{
  */
#define FDCAN_TIMEOUT_VALUE 10

#define FDCAN_TX_EVENT_FIFO_MASK (FDCAN_IR_TEFL | FDCAN_IR_TEFF | FDCAN_IR_TEFW | FDCAN_IR_TEFN)
#define FDCAN_RX_FIFO0_MASK (FDCAN_IR_RF0L | FDCAN_IR_RF0F | FDCAN_IR_RF0W | FDCAN_IR_RF0N)
#define FDCAN_RX_FIFO1_MASK (FDCAN_IR_RF1L | FDCAN_IR_RF1F | FDCAN_IR_RF1W | FDCAN_IR_RF1N)
#define FDCAN_ERROR_MASK (FDCAN_IR_ELO | FDCAN_IR_EP | FDCAN_IR_EW | FDCAN_IR_BO | \
                          FDCAN_IR_WDI | FDCAN_IR_PEA | FDCAN_IR_PED | FDCAN_IR_ARA)
#define FDCAN_TT_SCHEDULE_SYNC_MASK (FDCAN_TTIR_SBC | FDCAN_TTIR_SMC | FDCAN_TTIR_CSM | FDCAN_TTIR_SOG)
#define FDCAN_TT_TIME_MARK_MASK (FDCAN_TTIR_RTMI | FDCAN_TTIR_TTMI)
#define FDCAN_TT_GLOBAL_TIME_MASK (FDCAN_TTIR_GTW | FDCAN_TTIR_GTD)
#define FDCAN_TT_DISTURBING_ERROR_MASK (FDCAN_TTIR_GTE | FDCAN_TTIR_TXU | FDCAN_TTIR_TXO | \
                                        FDCAN_TTIR_SE1 | FDCAN_TTIR_SE2 | FDCAN_TTIR_ELC)
#define FDCAN_TT_FATAL_ERROR_MASK (FDCAN_TTIR_IWT | FDCAN_TTIR_WT | FDCAN_TTIR_AW | FDCAN_TTIR_CER)

#define FDCAN_ELEMENT_MASK_STDID ((uint32_t)0x1FFC0000U) /* Standard Identifier         */
#define FDCAN_ELEMENT_MASK_EXTID ((uint32_t)0x1FFFFFFFU) /* Extended Identifier         */
#define FDCAN_ELEMENT_MASK_RTR   ((uint32_t)0x20000000U) /* Remote Transmission Request */
#define FDCAN_ELEMENT_MASK_XTD   ((uint32_t)0x40000000U) /* Extended Identifier         */
#define FDCAN_ELEMENT_MASK_ESI   ((uint32_t)0x80000000U) /* Error State Indicator       */
#define FDCAN_ELEMENT_MASK_TS    ((uint32_t)0x0000FFFFU) /* Timestamp                   */
#define FDCAN_ELEMENT_MASK_DLC   ((uint32_t)0x000F0000U) /* Data Length Code            */
#define FDCAN_ELEMENT_MASK_BRS   ((uint32_t)0x00100000U) /* Bit Rate Switch             */
#define FDCAN_ELEMENT_MASK_FDF   ((uint32_t)0x00200000U) /* FD Format                   */
#define FDCAN_ELEMENT_MASK_EFC   ((uint32_t)0x00800000U) /* Event FIFO Control          */
#define FDCAN_ELEMENT_MASK_MM    ((uint32_t)0xFF000000U) /* Message Marker              */
#define FDCAN_ELEMENT_MASK_FIDX  ((uint32_t)0x7F000000U) /* Filter Index                */
#define FDCAN_ELEMENT_MASK_ANMF  ((uint32_t)0x80000000U) /* Accepted Non-matching Frame */
#define FDCAN_ELEMENT_MASK_ET    ((uint32_t)0x00C00000U) /* Event type                  */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
static const uint8_t CvtEltSize[] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7};

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup FDCAN_Private_Functions_Prototypes
  * @{
  */
static HAL_StatusTypeDef FDCAN_CalcultateRamBlockAddresses(FDCAN_HandleTypeDef *hfdcan);
static HAL_StatusTypeDef FDCAN_CopyMessageToRAM(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData, uint32_t BufferIndex);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup FDCAN_Exported_Functions FDCAN Exported Functions
  * @{
  */

/** @defgroup FDCAN_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the FDCAN.
      (+) De-initialize the FDCAN.
      (+) Enter FDCAN peripheral in power down mode.
      (+) Exit power down mode.

@endverbatim
  * @{
  */

/**
  * @brief  Initializes the FDCAN peripheral according to the specified
  *         parameters in the FDCAN_InitTypeDef structure.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_Init(FDCAN_HandleTypeDef* hfdcan)
{
  uint32_t tickstart = 0U;

  /* Check FDCAN handle */
  if(hfdcan == NULL)
  {
     return HAL_ERROR;
  }

  /* Check FDCAN instance */
  if(hfdcan->Instance == FDCAN1)
  {
    hfdcan->ttcan = (TTCAN_TypeDef *)((uint32_t)hfdcan->Instance + 0x100);
  }

  /* Check function parameters */
  assert_param(IS_FDCAN_ALL_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_FRAME_FORMAT(hfdcan->Init.FrameFormat));
  assert_param(IS_FDCAN_MODE(hfdcan->Init.Mode));
  assert_param(IS_FUNCTIONAL_STATE(hfdcan->Init.AutoRetransmission));
  assert_param(IS_FUNCTIONAL_STATE(hfdcan->Init.TransmitPause));
  assert_param(IS_FUNCTIONAL_STATE(hfdcan->Init.ProtocolException));
  assert_param(IS_FDCAN_NOMINAL_PRESCALER(hfdcan->Init.NominalPrescaler));
  assert_param(IS_FDCAN_NOMINAL_SJW(hfdcan->Init.NominalSyncJumpWidth));
  assert_param(IS_FDCAN_NOMINAL_TSEG1(hfdcan->Init.NominalTimeSeg1));
  assert_param(IS_FDCAN_NOMINAL_TSEG2(hfdcan->Init.NominalTimeSeg2));
  if(hfdcan->Init.FrameFormat == FDCAN_FRAME_FD_BRS)
  {
    assert_param(IS_FDCAN_DATA_PRESCALER(hfdcan->Init.DataPrescaler));
    assert_param(IS_FDCAN_DATA_SJW(hfdcan->Init.DataSyncJumpWidth));
    assert_param(IS_FDCAN_DATA_TSEG1(hfdcan->Init.DataTimeSeg1));
    assert_param(IS_FDCAN_DATA_TSEG2(hfdcan->Init.DataTimeSeg2));
  }
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.StdFiltersNbr, 128));
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.ExtFiltersNbr, 64));
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.RxFifo0ElmtsNbr, 64));
  if(hfdcan->Init.RxFifo0ElmtsNbr > 0)
  {
    assert_param(IS_FDCAN_DATA_SIZE(hfdcan->Init.RxFifo0ElmtSize));
  }
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.RxFifo1ElmtsNbr, 64));
  if(hfdcan->Init.RxFifo1ElmtsNbr > 0)
  {
    assert_param(IS_FDCAN_DATA_SIZE(hfdcan->Init.RxFifo1ElmtSize));
  }
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.RxBuffersNbr, 64));
  if(hfdcan->Init.RxBuffersNbr > 0)
  {
    assert_param(IS_FDCAN_DATA_SIZE(hfdcan->Init.RxBufferSize));
  }
  assert_param(IS_FDCAN_MAX_VALUE(hfdcan->Init.TxEventsNbr, 32));
  assert_param(IS_FDCAN_MAX_VALUE((hfdcan->Init.TxBuffersNbr + hfdcan->Init.TxFifoQueueElmtsNbr), 32));
  if(hfdcan->Init.TxFifoQueueElmtsNbr > 0)
  {
    assert_param(IS_FDCAN_TX_FIFO_QUEUE_MODE(hfdcan->Init.TxFifoQueueMode));
  }
  if((hfdcan->Init.TxBuffersNbr + hfdcan->Init.TxFifoQueueElmtsNbr) > 0)
  {
    assert_param(IS_FDCAN_DATA_SIZE(hfdcan->Init.TxElmtSize));
  }

  if(hfdcan->State == HAL_FDCAN_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hfdcan->Lock = HAL_UNLOCKED;

    /* Init the low level hardware */
    HAL_FDCAN_MspInit(hfdcan);
  }

  /* Exit from Sleep mode */
  CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_CSR);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Check Sleep mode acknowledge */
  while((hfdcan->Instance->CCCR & FDCAN_CCCR_CSA) == FDCAN_CCCR_CSA)
  {
    if((HAL_GetTick() - tickstart) > FDCAN_TIMEOUT_VALUE)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

      /* Change FDCAN state */
      hfdcan->State = HAL_FDCAN_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Request initialisation */
  SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until the INIT bit into CCCR register is set */
  while((hfdcan->Instance->CCCR & FDCAN_CCCR_INIT) == RESET)
  {
    /* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > FDCAN_TIMEOUT_VALUE)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

      /* Change FDCAN state */
      hfdcan->State = HAL_FDCAN_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Enable configuration change */
  SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_CCE);

  /* Set the no automatic retransmission */
  if(hfdcan->Init.AutoRetransmission == ENABLE)
  {
    CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_DAR);
  }
  else
  {
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_DAR);
  }

  /* Set the transmit pause feature */
  if(hfdcan->Init.TransmitPause == ENABLE)
  {
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_TXP);
  }
  else
  {
    CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_TXP);
  }

  /* Set the Protocol Exception Handling */
  if(hfdcan->Init.ProtocolException == ENABLE)
  {
    CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_PXHD);
  }
  else
  {
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_PXHD);
  }

  /* Set FDCAN Frame Format */
  MODIFY_REG(hfdcan->Instance->CCCR, FDCAN_FRAME_FD_BRS, hfdcan->Init.FrameFormat);

  /* Set FDCAN Operating Mode:
               | Normal | Restricted |    Bus     | Internal | External
               |        | Operation  | Monitoring | LoopBack | LoopBack
     CCCR.TEST |   0    |     0      |     0      |    1     |    1
     CCCR.MON  |   0    |     0      |     1      |    0     |    1
     TEST.LBCK |   0    |     0      |     0      |    1     |    1
     CCCR.ASM  |   0    |     1      |     0      |    0     |    0
  */
  if(hfdcan->Init.Mode == FDCAN_MODE_RESTRICTED_OPERATION)
  {
    /* Enable Restricted Operation mode */
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_ASM);
  }
  else if(hfdcan->Init.Mode != FDCAN_MODE_NORMAL)
  {
    if(hfdcan->Init.Mode != FDCAN_MODE_BUS_MONITORING)
    {
      /* Enable write access to TEST register */
      SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_TEST);

      /* Enable LoopBack mode */
      SET_BIT(hfdcan->Instance->TEST, FDCAN_TEST_LBCK);

      if(hfdcan->Init.Mode == FDCAN_MODE_EXTERNAL_LOOPBACK)
      {
        SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_MON);
      }
    }
    else
    {
      /* Enable bus monitoring mode */
      SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_MON);
    }
  }

  /* Set the nominal bit timing register */
  hfdcan->Instance->NBTP = ((((uint32_t)hfdcan->Init.NominalSyncJumpWidth - 1) << 25) | \
                            (((uint32_t)hfdcan->Init.NominalTimeSeg1 - 1) << 8)       | \
                            ((uint32_t)hfdcan->Init.NominalTimeSeg2 - 1)              | \
                            (((uint32_t)hfdcan->Init.NominalPrescaler - 1) << 16));

  /* If FD operation with BRS is selected, set the data bit timing register */
  if(hfdcan->Init.FrameFormat == FDCAN_FRAME_FD_BRS)
  {
    hfdcan->Instance->DBTP = (((uint32_t)hfdcan->Init.DataSyncJumpWidth - 1)   | \
                              (((uint32_t)hfdcan->Init.DataTimeSeg1 - 1) << 8) | \
                              (((uint32_t)hfdcan->Init.DataTimeSeg2 - 1) << 4) | \
                              (((uint32_t)hfdcan->Init.DataPrescaler - 1) << 16));
  }

  if(hfdcan->Init.TxFifoQueueElmtsNbr > 0)
  {
    /* Select between Tx FIFO and Tx Queue operation modes */
    SET_BIT(hfdcan->Instance->TXBC, hfdcan->Init.TxFifoQueueMode);
  }

  /* Configure Tx element size */
  if((hfdcan->Init.TxBuffersNbr + hfdcan->Init.TxFifoQueueElmtsNbr) > 0)
  {
    MODIFY_REG(hfdcan->Instance->TXESC, FDCAN_TXESC_TBDS, CvtEltSize[hfdcan->Init.TxElmtSize]);
  }

  /* Configure Rx FIFO 0 element size */
  if(hfdcan->Init.RxFifo0ElmtsNbr > 0)
  {
    MODIFY_REG(hfdcan->Instance->RXESC, FDCAN_RXESC_F0DS, CvtEltSize[hfdcan->Init.RxFifo0ElmtSize]);
  }

  /* Configure Rx FIFO 1 element size */
  if(hfdcan->Init.RxFifo1ElmtsNbr > 0)
  {
    MODIFY_REG(hfdcan->Instance->RXESC, FDCAN_RXESC_F1DS, (CvtEltSize[hfdcan->Init.RxFifo1ElmtSize] << 4));
  }

  /* Configure Rx buffer element size */
  if(hfdcan->Init.RxBuffersNbr > 0)
  {
    MODIFY_REG(hfdcan->Instance->RXESC, FDCAN_RXESC_RBDS, (CvtEltSize[hfdcan->Init.RxBufferSize] << 8));
  }

  /* By default operation mode is set to Event-driven communication.
     If Time-triggered communication is needed, user should call the
     HAL_FDCAN_TT_ConfigOperation function just after the HAL_FDCAN_Init */
  if(hfdcan->Instance == FDCAN1)
  {
    CLEAR_BIT(hfdcan->ttcan->TTOCF, FDCAN_TTOCF_OM);
  }

  /* Calculate each RAM block address */
  FDCAN_CalcultateRamBlockAddresses(hfdcan);

  /* Initialize the error code */
  hfdcan->ErrorCode = HAL_FDCAN_ERROR_NONE;

  /* Initialize the FDCAN state */
  hfdcan->State = HAL_FDCAN_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Deinitializes the FDCAN peripheral registers to their default reset values.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_DeInit(FDCAN_HandleTypeDef* hfdcan)
{
  /* Check FDCAN handle */
  if(hfdcan == NULL)
  {
     return HAL_ERROR;
  }

  /* Check function parameters */
  assert_param(IS_FDCAN_ALL_INSTANCE(hfdcan->Instance));

  /* Stop the FDCAN module */
  HAL_FDCAN_Stop(hfdcan);

  /* DeInit the low level hardware */
  HAL_FDCAN_MspDeInit(hfdcan);

  /* Reset the FDCAN ErrorCode */
  hfdcan->ErrorCode = HAL_FDCAN_ERROR_NONE;

  /* Change FDCAN state */
  hfdcan->State = HAL_FDCAN_STATE_RESET;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initializes the FDCAN MSP.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitializes the FDCAN MSP.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  Enter FDCAN peripheral in sleep mode.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_EnterPowerDownMode(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t tickstart = 0U;

  /* Request clock stop */
  SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_CSR);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until FDCAN is ready for power down */
  while((hfdcan->Instance->CCCR & FDCAN_CCCR_CSA) == RESET)
  {
    if((HAL_GetTick() - tickstart) > FDCAN_TIMEOUT_VALUE)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

      /* Change FDCAN state */
      hfdcan->State = HAL_FDCAN_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Exit power down mode.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ExitPowerDownMode(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t tickstart = 0U;

  /* Reset clock stop request */
  CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_CSR);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until FDCAN exits sleep mode */
  while((hfdcan->Instance->CCCR & FDCAN_CCCR_CSA) == FDCAN_CCCR_CSA)
  {
    if((HAL_GetTick() - tickstart) > FDCAN_TIMEOUT_VALUE)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

      /* Change FDCAN state */
      hfdcan->State = HAL_FDCAN_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Enter normal operation */
  CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group2 Configuration functions
 *  @brief    FDCAN Configuration functions.
 *
@verbatim
  ==============================================================================
              ##### Configuration functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) HAL_FDCAN_ConfigClockCalibration        : Configure the FDCAN clock calibration unit
        (+) HAL_FDCAN_GetClockCalibrationState      : Get the clock calibration state
        (+) HAL_FDCAN_ResetClockCalibrationState    : Reset the clock calibration state
        (+) HAL_FDCAN_GetClockCalibrationCounter    : Get the clock calibration counters values
      (+) HAL_FDCAN_ConfigFilter                  : Configure the FDCAN reception filters
      (+) HAL_FDCAN_ConfigGlobalFilter            : Configure the FDCAN global filter
      (+) HAL_FDCAN_ConfigExtendedIdMask          : Configure the extended ID mask
      (+) HAL_FDCAN_ConfigRxFifoOverwrite         : Configure the Rx FIFO operation mode
      (+) HAL_FDCAN_ConfigFifoWatermark           : Configure the FIFO watermark
      (+) HAL_FDCAN_ConfigRamWatchdog             : Configure the RAM watchdog
      (+) HAL_FDCAN_ConfigTimestampCounter        : Configure the timestamp counter
        (+) HAL_FDCAN_EnableTimestampCounter        : Enable the timestamp counter
        (+) HAL_FDCAN_DisableTimestampCounter       : Disable the timestamp counter
        (+) HAL_FDCAN_GetTimestampCounter           : Get the timestamp counter value
        (+) HAL_FDCAN_ResetTimestampCounter         : Reset the timestamp counter to zero
      (+) HAL_FDCAN_ConfigTimeoutCounter          : Configure the timeout counter
        (+) HAL_FDCAN_EnableTimeoutCounter          : Enable the timeout counter
        (+) HAL_FDCAN_DisableTimeoutCounter         : Disable the timeout counter
        (+) HAL_FDCAN_GetTimeoutCounter             : Get the timeout counter value
        (+) HAL_FDCAN_ResetTimeoutCounter           : Reset the timeout counter to its start value
      (+) HAL_FDCAN_ConfigTxDelayCompensation     : Configure the transmitter delay compensation
        (+) HAL_FDCAN_EnableTxDelayCompensation     : Enable the transmitter delay compensation
        (+) HAL_FDCAN_DisableTxDelayCompensation    : Disable the transmitter delay compensation

@endverbatim
  * @{
  */

/**
  * @brief  Configure the FDCAN clock calibration unit according to the specified
  *         parameters in the FDCAN_ClkCalUnitTypeDef structure.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  sCcuConfig: pointer to an FDCAN_ClkCalUnitTypeDef structure that
  *         contains the clock calibration information
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigClockCalibration(FDCAN_HandleTypeDef* hfdcan, FDCAN_ClkCalUnitTypeDef* sCcuConfig)
{
  /* Check function parameters */
  assert_param(IS_FUNCTIONAL_STATE(sCcuConfig->ClockCalibration));
  if(sCcuConfig->ClockCalibration == DISABLE)
  {
    assert_param(IS_FDCAN_CKDIV(sCcuConfig->ClockDivider));
  }
  else
  {
    assert_param(IS_FDCAN_MAX_VALUE(sCcuConfig->MinOscClkPeriods, 0xFF));
    assert_param(IS_FDCAN_CALIBRATION_FIELD_LENGTH(sCcuConfig->CalFieldLength));
    assert_param(IS_FDCAN_MIN_VALUE(sCcuConfig->TimeQuantaPerBitTime, 4));
    assert_param(IS_FDCAN_MAX_VALUE(sCcuConfig->TimeQuantaPerBitTime, 0x25));
    assert_param(IS_FDCAN_MAX_VALUE(sCcuConfig->WatchdogStartValue, 0xFFFF));
  }

  /* FDCAN1 should be initialized in order to use clock calibration */
  if(hfdcan->Instance != FDCAN1)
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    if(sCcuConfig->ClockCalibration == DISABLE)
    {
      /* Bypass clock calibration */
      SET_BIT(FDCAN_CCU->CCFG, FDCANCCU_CCFG_BCC);

      /* Configure clock divider */
      MODIFY_REG(FDCAN_CCU->CCFG, FDCANCCU_CCFG_CDIV, sCcuConfig->ClockDivider);
    }
    else /* sCcuConfig->ClockCalibration == ENABLE */
    {
      /* Clock calibration unit generates time quanta clock */
      CLEAR_BIT(FDCAN_CCU->CCFG, FDCANCCU_CCFG_BCC);

      /* Configure clock calibration unit */
      MODIFY_REG(FDCAN_CCU->CCFG,
                 (FDCANCCU_CCFG_TQBT | FDCANCCU_CCFG_CFL | FDCANCCU_CCFG_OCPM),
                 (sCcuConfig->TimeQuantaPerBitTime | sCcuConfig->CalFieldLength | (sCcuConfig->MinOscClkPeriods << 8)));

      /* Configure the start value of the calibration watchdog counter */
      MODIFY_REG(FDCAN_CCU->CWD, FDCANCCU_CWD_WDC, sCcuConfig->WatchdogStartValue);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get the clock calibration state.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval State: clock calibration state (can be a value of @arg FDCAN_calibration_state)
  */
uint32_t HAL_FDCAN_GetClockCalibrationState(FDCAN_HandleTypeDef* hfdcan)
{
  return (FDCAN_CCU->CSTAT & FDCANCCU_CSTAT_CALS);
}

/**
  * @brief  Reset the clock calibration state.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ResetClockCalibrationState(FDCAN_HandleTypeDef* hfdcan)
{
  /* Calibration software reset */
  SET_BIT(FDCAN_CCU->CCFG, FDCANCCU_CCFG_SWR);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Get the clock calibration counter value.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  Counter: clock calibration counter.
  *                  This parameter can be a value of @arg FDCAN_calibration_counter.
  * @retval Value: clock calibration counter value
  */
uint32_t HAL_FDCAN_GetClockCalibrationCounter(FDCAN_HandleTypeDef* hfdcan, uint32_t Counter)
{
  if(Counter == FDCAN_CALIB_TIME_QUANTA_COUNTER)
  {
    return ((FDCAN_CCU->CSTAT & FDCANCCU_CSTAT_TQC) >> 18);
  }
  else if(Counter == FDCAN_CALIB_CLOCK_PERIOD_COUNTER)
  {
    return (FDCAN_CCU->CSTAT & FDCANCCU_CSTAT_OCPC);
  }
  else /* Counter == FDCAN_CALIB_WATCHDOG_COUNTER */
  {
    return ((FDCAN_CCU->CWD & FDCANCCU_CWD_WDV) >> 16);
  }
}

/**
  * @brief  Configure the FDCAN reception filter according to the specified
  *         parameters in the FDCAN_FilterTypeDef structure.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  sFilterConfig: pointer to an FDCAN_FilterTypeDef structure that
  *         contains the filter configuration information
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigFilter(FDCAN_HandleTypeDef* hfdcan, FDCAN_FilterTypeDef* sFilterConfig)
{
  uint32_t FilterElementW1;
  uint32_t FilterElementW2;
  uint32_t *FilterAddress;

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check function parameters */
    assert_param(IS_FDCAN_ID_TYPE(sFilterConfig->IdType));
    assert_param(IS_FDCAN_FILTER_CFG(sFilterConfig->FilterConfig));
    if(sFilterConfig->FilterConfig == FDCAN_FILTER_TO_RXBUFFER)
    {
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->RxBufferIndex, 63));
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->IsCalibrationMsg, 1));
    }

    if(sFilterConfig->IdType == FDCAN_STANDARD_ID)
    {
      /* Check function parameters */
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterIndex, (hfdcan->Init.StdFiltersNbr - 1)));
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterID1, 0x7FF));
      if(sFilterConfig->FilterConfig != FDCAN_FILTER_TO_RXBUFFER)
      {
        assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterID2, 0x7FF));
        assert_param(IS_FDCAN_STD_FILTER_TYPE(sFilterConfig->FilterType));
      }

      /* Build filter element */
      if(sFilterConfig->FilterConfig == FDCAN_FILTER_TO_RXBUFFER)
      {
        FilterElementW1 = ((FDCAN_FILTER_TO_RXBUFFER << 27)       |
                           (sFilterConfig->FilterID1 << 16)       |
                           (sFilterConfig->IsCalibrationMsg << 8) |
                           sFilterConfig->RxBufferIndex            );
      }
      else
      {
        FilterElementW1 = ((sFilterConfig->FilterType << 30)   |
                           (sFilterConfig->FilterConfig << 27) |
                           (sFilterConfig->FilterID1 << 16)    |
                           sFilterConfig->FilterID2             );
      }

      /* Calculate filter address */
      FilterAddress = (uint32_t *)(hfdcan->msgRam.StandardFilterSA + (sFilterConfig->FilterIndex * 4));

      /* Write filter element to the message RAM */
      *FilterAddress = FilterElementW1;
    }
    else /* sFilterConfig->IdType == FDCAN_EXTENDED_ID */
    {
      /* Check function parameters */
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterIndex, (hfdcan->Init.ExtFiltersNbr - 1)));
      assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterID1, 0x1FFFFFFF));
      if(sFilterConfig->FilterConfig != FDCAN_FILTER_TO_RXBUFFER)
      {
        assert_param(IS_FDCAN_MAX_VALUE(sFilterConfig->FilterID2, 0x1FFFFFFF));
        assert_param(IS_FDCAN_EXT_FILTER_TYPE(sFilterConfig->FilterType));
      }

      /* Build first word of filter element */
      FilterElementW1 = ((sFilterConfig->FilterConfig << 29) | sFilterConfig->FilterID1);

      /* Build second word of filter element */
      if(sFilterConfig->FilterConfig == FDCAN_FILTER_TO_RXBUFFER)
      {
        FilterElementW2 = sFilterConfig->RxBufferIndex;
      }
      else
      {
        FilterElementW2 = ((sFilterConfig->FilterType << 30) | sFilterConfig->FilterID2);
      }

      /* Calculate filter address */
      FilterAddress = (uint32_t *)(hfdcan->msgRam.ExtendedFilterSA + (sFilterConfig->FilterIndex * 4 * 2));

      /* Write filter element to the message RAM */
      *FilterAddress++ = FilterElementW1;
      *FilterAddress = FilterElementW2;
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the FDCAN global filter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  NonMatchingStd: Defines how received messages with 11-bit IDs that
                            do not match any element of the filter list are treated.
                            This parameter can be a value of @arg FDCAN_Non_Matching_Frames.
  * @param  NonMatchingExt: Defines how received messages with 29-bit IDs that
                            do not match any element of the filter list are treated.
                            This parameter can be a value of @arg FDCAN_Non_Matching_Frames.
  * @param  RejectRemoteStd: Enable or disable the remote standard frames rejection.
                             This parameter can be set to ENABLE or DISABLE.
  * @param  RejectRemoteExt: Enable or disable the remote extended frames rejection.
                             This parameter can be set to ENABLE or DISABLE.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigGlobalFilter(FDCAN_HandleTypeDef *hfdcan,
                                               uint32_t NonMatchingStd,
                                               uint32_t NonMatchingExt,
                                               uint32_t RejectRemoteStd,
                                               uint32_t RejectRemoteExt)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_NON_MATCHING(NonMatchingStd));
  assert_param(IS_FDCAN_NON_MATCHING(NonMatchingExt));
  assert_param(IS_FUNCTIONAL_STATE(RejectRemoteStd));
  assert_param(IS_FUNCTIONAL_STATE(RejectRemoteExt));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure global filter */
    hfdcan->Instance->GFC = ((NonMatchingStd << 4U) | (NonMatchingExt << 2U) | (RejectRemoteStd << 1U) | RejectRemoteExt);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the extended ID mask.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  Mask: Extended ID Mask.
                  This parameter must be a number between 0 and 0x1FFFFFFF
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigExtendedIdMask(FDCAN_HandleTypeDef *hfdcan, uint32_t Mask)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_MAX_VALUE(Mask, 0x1FFFFFFF));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure the extended ID mask */
    hfdcan->Instance->XIDAM = Mask;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the Rx FIFO operation mode.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxFifo: Rx FIFO.
  *                 This parameter can be one of the following values:
  *                   @arg FDCAN_RX_FIFO0: Rx FIFO 0
  *                   @arg FDCAN_RX_FIFO1: Rx FIFO 1
  * @param  OperationMode: operation mode.
  *                        This parameter can be a value of @arg FDCAN_Rx_FIFO_operation_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigRxFifoOverwrite(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo, uint32_t OperationMode)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_RX_FIFO(RxFifo));
  assert_param(IS_FDCAN_RX_FIFO_MODE(OperationMode));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    if(RxFifo == FDCAN_RX_FIFO0)
    {
      /* Select FIFO 0 Operation Mode */
      MODIFY_REG(hfdcan->Instance->RXF0C, FDCAN_RXF0C_F0OM, OperationMode);
    }
    else /* RxFifo == FDCAN_RX_FIFO1 */
    {
      /* Select FIFO 1 Operation Mode */
      MODIFY_REG(hfdcan->Instance->RXF1C, FDCAN_RXF1C_F1OM, OperationMode);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the FIFO watermark.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  FIFO: select the FIFO to be configured.
  *               This parameter can be a value of @arg FDCAN_FIFO_watermark.
  * @param  Watermark: level for FIFO watermark interrupt.
  *                    This parameter must be a number between:
  *                     - 0 and 32, if FIFO is FDCAN_CFG_TX_EVENT_FIFO
  *                     - 0 and 64, if FIFO is FDCAN_CFG_RX_FIFO0 or FDCAN_CFG_RX_FIFO1
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigFifoWatermark(FDCAN_HandleTypeDef *hfdcan, uint32_t FIFO, uint32_t Watermark)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_FIFO_WATERMARK(FIFO));
  if(FIFO == FDCAN_CFG_TX_EVENT_FIFO)
  {
    assert_param(IS_FDCAN_MAX_VALUE(Watermark, 32));
  }
  else /* (FIFO == FDCAN_CFG_RX_FIFO0) || (FIFO == FDCAN_CFG_RX_FIFO1) */
  {
    assert_param(IS_FDCAN_MAX_VALUE(Watermark, 64));
  }

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Set the level for FIFO watermark interrupt */
    if(FIFO == FDCAN_CFG_TX_EVENT_FIFO)
    {
      MODIFY_REG(hfdcan->Instance->TXEFC, FDCAN_TXEFC_EFWM, (Watermark << 24));
    }
    else if(FIFO == FDCAN_CFG_RX_FIFO0)
    {
      MODIFY_REG(hfdcan->Instance->RXF0C, FDCAN_RXF0C_F0WM, (Watermark << 24));
    }
    else /* FIFO == FDCAN_CFG_RX_FIFO1 */
    {
      MODIFY_REG(hfdcan->Instance->RXF1C, FDCAN_RXF1C_F1WM, (Watermark << 24));
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the RAM watchdog.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  CounterStartValue: Start value of the Message RAM Watchdog Counter,
  *                            This parameter must be a number between 0x00 and 0xFF,
  *                            with the reset value of 0x00 the counter is disabled.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigRamWatchdog(FDCAN_HandleTypeDef *hfdcan, uint32_t CounterStartValue)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_MAX_VALUE(CounterStartValue, 0xFF));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure the RAM watchdog counter start value */
    MODIFY_REG(hfdcan->Instance->RWD, FDCAN_RWD_WDC, CounterStartValue);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the timestamp counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TimestampPrescaler: Timestamp Counter Prescaler.
  *                             This parameter can be a value of @arg FDCAN_Timestamp_Prescaler.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigTimestampCounter(FDCAN_HandleTypeDef *hfdcan, uint32_t TimestampPrescaler)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TIMESTAMP_PRESCALER(TimestampPrescaler));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure prescaler */
    MODIFY_REG(hfdcan->Instance->TSCC, FDCAN_TSCC_TCP, TimestampPrescaler);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable the timestamp counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TimestampOperation: Timestamp counter operation.
  *                             This parameter can be a value of @arg FDCAN_Timestamp.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_EnableTimestampCounter(FDCAN_HandleTypeDef *hfdcan, uint32_t TimestampOperation)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TIMESTAMP(TimestampOperation));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Enable timestamp counter */
    MODIFY_REG(hfdcan->Instance->TSCC, FDCAN_TSCC_TSS, TimestampOperation);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable the timestamp counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_DisableTimestampCounter(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Disable timestamp counter */
    CLEAR_BIT(hfdcan->Instance->TSCC, FDCAN_TSCC_TSS);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get the timestamp counter value.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval Value: Timestamp counter value
  */
uint16_t HAL_FDCAN_GetTimestampCounter(FDCAN_HandleTypeDef *hfdcan)
{
  return (uint16_t)(hfdcan->Instance->TSCV);
}

/**
  * @brief  Reset the timestamp counter to zero.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ResetTimestampCounter(FDCAN_HandleTypeDef *hfdcan)
{
  if((hfdcan->Instance->TSCC & FDCAN_TSCC_TSS) != FDCAN_TIMESTAMP_EXTERNAL)
  {
    /* Reset timestamp counter.
       Actually any write operation to TSCV clears the counter */
    CLEAR_REG(hfdcan->Instance->TSCV);
  }
  else
  {
    /* Update error code.
       Unable to reset external counter */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

    return HAL_ERROR;
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Configure the timeout counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TimeoutOperation: Timeout counter operation.
  *                           This parameter can be a value of @arg FDCAN_Timeout_Operation.
  * @param  TimeoutPeriod: Start value of the timeout down-counter.
  *                        This parameter must be a number between 0x0000 and 0xFFFF
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigTimeoutCounter(FDCAN_HandleTypeDef *hfdcan, uint32_t TimeoutOperation, uint32_t TimeoutPeriod)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TIMEOUT(TimeoutOperation));
  assert_param(IS_FDCAN_MAX_VALUE(TimeoutPeriod, 0xFFFF));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Select timeout operation and configure period */
    MODIFY_REG(hfdcan->Instance->TOCC, (FDCAN_TOCC_TOS | FDCAN_TOCC_TOP), (TimeoutOperation | (TimeoutPeriod << 16)));

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable the timeout counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_EnableTimeoutCounter(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Enable timeout counter */
    SET_BIT(hfdcan->Instance->TOCC, FDCAN_TOCC_ETOC);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable the timeout counter.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_DisableTimeoutCounter(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Disable timeout counter */
    CLEAR_BIT(hfdcan->Instance->TOCC, FDCAN_TOCC_ETOC);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get the timeout counter value.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval Value: Timeout counter value
  */
uint16_t HAL_FDCAN_GetTimeoutCounter(FDCAN_HandleTypeDef *hfdcan)
{
  return (uint16_t)(hfdcan->Instance->TOCV);
}

/**
  * @brief  Reset the timeout counter to its start value.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ResetTimeoutCounter(FDCAN_HandleTypeDef *hfdcan)
{
  if((hfdcan->Instance->TOCC & FDCAN_TOCC_TOS) != FDCAN_TIMEOUT_CONTINUOUS)
  {
    /* Reset timestamp counter to start value */
    CLEAR_REG(hfdcan->Instance->TOCV);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code.
       Unable to reset counter: controlled only by FIFO empty state */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the transmitter delay compensation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TdcOffset: Transmitter Delay Compensation Offset.
  *                    This parameter must be a number between 0x00 and 0xFF.
  * @param  TdcFilter: Transmitter Delay Compensation Filter Window Length.
  *                    This parameter must be a number between 0x00 and 0xFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigTxDelayCompensation(FDCAN_HandleTypeDef *hfdcan, uint32_t TdcOffset, uint32_t TdcFilter)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_MAX_VALUE(TdcOffset, 0xFF));
  assert_param(IS_FDCAN_MAX_VALUE(TdcFilter, 0xFF));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure TDC offset and filter window */
    hfdcan->Instance->TDCR = (TdcFilter | (TdcOffset << 8));

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable the transmitter delay compensation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_EnableTxDelayCompensation(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Enable transmitter delay compensation */
    SET_BIT(hfdcan->Instance->DBTP, FDCAN_DBTP_TDC);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable the transmitter delay compensation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_DisableTxDelayCompensation(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Disable transmitter delay compensation */
    CLEAR_BIT(hfdcan->Instance->DBTP, FDCAN_DBTP_TDC);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group3 Control functions
 *  @brief    Control functions
 *
@verbatim
  ==============================================================================
                          ##### Control functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) HAL_FDCAN_Start                        : Start the FDCAN module
      (+) HAL_FDCAN_Stop                         : Stop the FDCAN module and enable access to configuration registers
      (+) HAL_FDCAN_AddMessageToTxFifoQ          : Add a message to the Tx FIFO/Queue and activate the corresponding transmission request
      (+) HAL_FDCAN_AddMessageToTxBuffer         : Add a message to a dedicated Tx buffer
      (+) HAL_FDCAN_EnableTxBufferRequest        : Enable transmission request
      (+) HAL_FDCAN_AbortTxRequest               : Abort transmission request
      (+) HAL_FDCAN_GetRxMessage                 : Get an FDCAN frame from the Rx Buffer/FIFO zone into the message RAM
      (+) HAL_FDCAN_GetTxEvent                   : Get an FDCAN Tx event from the Tx Event FIFO zone into the message RAM
      (+) HAL_FDCAN_GetHighPriorityMessageStatus : Get high priority message status
      (+) HAL_FDCAN_GetProtocolStatus            : Get protocol status
      (+) HAL_FDCAN_GetErrorCounters             : Get error counter values
      (+) HAL_FDCAN_IsRxBufferMessageAvailable   : Check if a new message is received in the selected Rx buffer
      (+) HAL_FDCAN_IsTxBufferMessagePending     : Check if a transmission request is pending on the selected Tx buffer
      (+) HAL_FDCAN_GetRxFifoFillLevel           : Return Rx FIFO fill level
      (+) HAL_FDCAN_GetTxFifoFreeLevel           : Return Tx FIFO free level
      (+) HAL_FDCAN_IsRestrictedOperationMode    : Check if the FDCAN peripheral entered Restricted Operation Mode
      (+) HAL_FDCAN_ExitRestrictedOperationMode  : Exit Restricted Operation Mode

@endverbatim
  * @{
  */

/**
  * @brief  Start the FDCAN module.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_Start(FDCAN_HandleTypeDef *hfdcan)
{
  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Change FDCAN peripheral state */
    hfdcan->State = HAL_FDCAN_STATE_BUSY;

    /* Request leave initialisation */
    CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);

    /* Reset the FDCAN ErrorCode */
    hfdcan->ErrorCode = HAL_FDCAN_ERROR_NONE;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Stop the FDCAN module and enable access to configuration registers.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_Stop(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  if(hfdcan->State == HAL_FDCAN_STATE_BUSY)
  {
    /* Request initialisation */
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);

    /* Wait until the INIT bit into CCCR register is set */
    while((hfdcan->Instance->CCCR & FDCAN_CCCR_INIT) == RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Enable configuration change */
    SET_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_CCE);

    /* Change FDCAN peripheral state */
    hfdcan->State = HAL_FDCAN_STATE_READY;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Add a message to the Tx FIFO/Queue and activate the corresponding transmission request
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  pTxHeader: pointer to a FDCAN_TxHeaderTypeDef structure.
  * @param  pTxData: pointer to a buffer containing the payload of the Tx frame.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData)
{
  uint32_t PutIndex;

  /* Check function parameters */
  assert_param(IS_FDCAN_ID_TYPE(pTxHeader->IdType));
  if(pTxHeader->IdType == FDCAN_STANDARD_ID)
  {
    assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->Identifier, 0x7FF));
  }
  else /* pTxHeader->IdType == FDCAN_EXTENDED_ID */
  {
    assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->Identifier, 0x1FFFFFFF));
  }
  assert_param(IS_FDCAN_FRAME_TYPE(pTxHeader->TxFrameType));
  assert_param(IS_FDCAN_DLC(pTxHeader->DataLength));
  assert_param(IS_FDCAN_ESI(pTxHeader->ErrorStateIndicator));
  assert_param(IS_FDCAN_BRS(pTxHeader->BitRateSwitch));
  assert_param(IS_FDCAN_FDF(pTxHeader->FDFormat));
  assert_param(IS_FDCAN_EFC(pTxHeader->TxEventFifoControl));
  assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->MessageMarker, 0xFF));

  if(hfdcan->State == HAL_FDCAN_STATE_BUSY)
  {
    /* Check that the Tx FIFO/Queue has an allocated area into the RAM */
    if((hfdcan->Instance->TXBC & FDCAN_TXBC_TFQS) == 0)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }

    /* Check that the Tx FIFO/Queue is not full */
    if((hfdcan->Instance->TXFQS & FDCAN_TXFQS_TFQF) != 0)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }
    else
    {
      /* Retrieve the Tx FIFO PutIndex */
      PutIndex = ((hfdcan->Instance->TXFQS & FDCAN_TXFQS_TFQPI) >> 16);

      /* Add the message to the Tx FIFO/Queue */
      FDCAN_CopyMessageToRAM(hfdcan, pTxHeader, pTxData, PutIndex);

      /* Activate the corresponding transmission request */
      hfdcan->Instance->TXBAR = (1 << PutIndex);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Add a message to a dedicated Tx buffer
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  pTxHeader: pointer to a FDCAN_TxHeaderTypeDef structure.
  * @param  pTxData: pointer to a buffer containing the payload of the Tx frame.
  * @param  BufferIndex: index of the buffer to be configured.
  *                      This parameter can be a value of @arg FDCAN_Tx_location.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxBuffer(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData, uint32_t BufferIndex)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_ID_TYPE(pTxHeader->IdType));
  if(pTxHeader->IdType == FDCAN_STANDARD_ID)
  {
    assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->Identifier, 0x7FF));
  }
  else /* pTxHeader->IdType == FDCAN_EXTENDED_ID */
  {
    assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->Identifier, 0x1FFFFFFF));
  }
  assert_param(IS_FDCAN_FRAME_TYPE(pTxHeader->TxFrameType));
  assert_param(IS_FDCAN_DLC(pTxHeader->DataLength));
  assert_param(IS_FDCAN_ESI(pTxHeader->ErrorStateIndicator));
  assert_param(IS_FDCAN_BRS(pTxHeader->BitRateSwitch));
  assert_param(IS_FDCAN_FDF(pTxHeader->FDFormat));
  assert_param(IS_FDCAN_EFC(pTxHeader->TxEventFifoControl));
  assert_param(IS_FDCAN_MAX_VALUE(pTxHeader->MessageMarker, 0xFF));
  assert_param(IS_FDCAN_TX_LOCATION(BufferIndex));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the selected buffer has an allocated area into the RAM */
    if(POSITION_VAL(BufferIndex) >= ((hfdcan->Instance->TXBC & FDCAN_TXBC_NDTB) >> 16))
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }

    /* Check that there is no transmittion request pending for the selected buffer */
    if((hfdcan->Instance->TXBRP & BufferIndex) != 0)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PENDING;

      return HAL_ERROR;
    }
    else
    {
      /* Add the message to the Tx buffer */
      FDCAN_CopyMessageToRAM(hfdcan, pTxHeader, pTxData, POSITION_VAL(BufferIndex));
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable transmission request.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  BufferIndex: buffer index.
  *                      This parameter can be any combination of @arg FDCAN_Tx_location.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_EnableTxBufferRequest(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndex)
{
  if(hfdcan->State == HAL_FDCAN_STATE_BUSY)
  {
    /* Add transmission request */
    hfdcan->Instance->TXBAR = BufferIndex;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Abort transmission request
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  BufferIndex: buffer index.
  *                      This parameter can be any combination of @arg FDCAN_Tx_location.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_AbortTxRequest(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndex)
{
  if(hfdcan->State == HAL_FDCAN_STATE_BUSY)
  {
    /* Add cancellation request */
    hfdcan->Instance->TXBCR = BufferIndex;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get an FDCAN frame from the Rx Buffer/FIFO zone into the message RAM.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxLocation: Location of the received message to be read.
                        This parameter can be a value of @arg FDCAN_Rx_location.
  * @param  pRxHeader: pointer to a FDCAN_RxHeaderTypeDef structure.
  * @param  pRxData: pointer to a buffer where the payload of the Rx frame will be stored.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_GetRxMessage(FDCAN_HandleTypeDef *hfdcan, uint32_t RxLocation, FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData)
{
  uint32_t *RxAddress;
  uint8_t  *pData;
  uint32_t ByteCounter;
  uint32_t GetIndex = 0;

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if(RxLocation == FDCAN_RX_FIFO0) /* Rx element is assigned to the Rx FIFO 0 */
    {
      /* Check that the Rx FIFO 0 has an allocated area into the RAM */
      if((hfdcan->Instance->RXF0C & FDCAN_RXF0C_F0S) == 0)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

        return HAL_ERROR;
      }

      /* Check that the Rx FIFO 0 is not empty */
      if((hfdcan->Instance->RXF0S & FDCAN_RXF0S_F0FL) == 0)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

        return HAL_ERROR;
      }
      else
      {
        /* Calculate Rx FIFO 0 element address */
        GetIndex = ((hfdcan->Instance->RXF0S & FDCAN_RXF0S_F0GI) >> 8);
        RxAddress = (uint32_t *)(hfdcan->msgRam.RxFIFO0SA + (GetIndex * hfdcan->Init.RxFifo0ElmtSize * 4));
      }
    }
    else if(RxLocation == FDCAN_RX_FIFO1) /* Rx element is assigned to the Rx FIFO 1 */
    {
      /* Check that the Rx FIFO 1 has an allocated area into the RAM */
      if((hfdcan->Instance->RXF1C & FDCAN_RXF1C_F1S) == 0)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

        return HAL_ERROR;
      }

      /* Check that the Rx FIFO 0 is not empty */
      if((hfdcan->Instance->RXF1S & FDCAN_RXF1S_F1FL) == 0)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

        return HAL_ERROR;
      }
      else
      {
        /* Calculate Rx FIFO 1 element address */
        GetIndex = ((hfdcan->Instance->RXF1S & FDCAN_RXF1S_F1GI) >> 8);
        RxAddress = (uint32_t *)(hfdcan->msgRam.RxFIFO1SA + (GetIndex * hfdcan->Init.RxFifo1ElmtSize * 4));
      }
    }
    else /* Rx element is assigned to a dedicated Rx buffer */
    {
      /* Check that the selected buffer has an allocated area into the RAM */
      if(RxLocation >= hfdcan->Init.RxBuffersNbr)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

        return HAL_ERROR;
      }
      else
      {
        /* Calculate Rx buffer address */
        RxAddress = (uint32_t *)(hfdcan->msgRam.RxBufferSA + (RxLocation * hfdcan->Init.RxBufferSize * 4));
      }
    }

    /* Retrieve IdType */
    pRxHeader->IdType = *RxAddress & FDCAN_ELEMENT_MASK_XTD;

    /* Retrieve Identifier */
    if(pRxHeader->IdType == FDCAN_STANDARD_ID) /* Standard ID element */
    {
      pRxHeader->Identifier = ((*RxAddress & FDCAN_ELEMENT_MASK_STDID) >> 18);
    }
    else /* Extended ID element */
    {
      pRxHeader->Identifier = (*RxAddress & FDCAN_ELEMENT_MASK_EXTID);
    }

    /* Retrieve RxFrameType */
    pRxHeader->RxFrameType = (*RxAddress & FDCAN_ELEMENT_MASK_RTR);

    /* Retrieve ErrorStateIndicator */
    pRxHeader->ErrorStateIndicator = (*RxAddress++ & FDCAN_ELEMENT_MASK_ESI);

    /* Retrieve RxTimestamp */
    pRxHeader->RxTimestamp = (*RxAddress & FDCAN_ELEMENT_MASK_TS);

    /* Retrieve DataLength */
    pRxHeader->DataLength = (*RxAddress & FDCAN_ELEMENT_MASK_DLC);

    /* Retrieve BitRateSwitch */
    pRxHeader->BitRateSwitch = (*RxAddress & FDCAN_ELEMENT_MASK_BRS);

    /* Retrieve FDFormat */
    pRxHeader->FDFormat = (*RxAddress & FDCAN_ELEMENT_MASK_FDF);

    /* Retrieve FilterIndex */
    pRxHeader->FilterIndex = ((*RxAddress & FDCAN_ELEMENT_MASK_FIDX) >> 24);

    /* Retrieve NonMatchingFrame */
    pRxHeader->IsFilterMatchingFrame = ((*RxAddress++ & FDCAN_ELEMENT_MASK_ANMF) >> 31);

    /* Retrieve Rx payload */
    pData = (uint8_t *)RxAddress;
    for(ByteCounter = 0; ByteCounter < DLCtoBytes[pRxHeader->DataLength >> 16]; ByteCounter++)
    {
      *pRxData++ = *pData++;
    }

    if(RxLocation == FDCAN_RX_FIFO0) /* Rx element is assigned to the Rx FIFO 0 */
    {
      /* Acknowledge the Rx FIFO 0 that the oldest element is read so that it increments the GetIndex */
      hfdcan->Instance->RXF0A = GetIndex;
    }
    else if(RxLocation == FDCAN_RX_FIFO1) /* Rx element is assigned to the Rx FIFO 1 */
    {
      /* Acknowledge the Rx FIFO 1 that the oldest element is read so that it increments the GetIndex */
      hfdcan->Instance->RXF1A = GetIndex;
    }
    else /* Rx element is assigned to a dedicated Rx buffer */
    {
      /* Clear the New Data flag of the current Rx buffer */
      if(RxLocation < FDCAN_RX_BUFFER32)
      {
        hfdcan->Instance->NDAT1 = (1 << RxLocation);
      }
      else /* FDCAN_RX_BUFFER32 <= RxLocation <= FDCAN_RX_BUFFER63 */
      {
        hfdcan->Instance->NDAT2 = (1 << (RxLocation - 0x20));
      }
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get an FDCAN Tx event from the Tx Event FIFO zone into the message RAM.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  pTxEvent: pointer to a FDCAN_TxEventFifoTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_GetTxEvent(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxEventFifoTypeDef *pTxEvent)
{
  uint32_t *TxEventAddress;
  uint32_t GetIndex;

  /* Check function parameters */
  assert_param(IS_FDCAN_MIN_VALUE(hfdcan->Init.TxEventsNbr, 1));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the Tx Event FIFO has an allocated area into the RAM */
    if((hfdcan->Instance->TXEFC & FDCAN_TXEFC_EFS) == 0)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }

    /* Check that the Tx event FIFO is not empty */
    if((hfdcan->Instance->TXEFS & FDCAN_TXEFS_EFFL) == 0)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }

    /* Calculate Tx event FIFO element address */
    GetIndex = ((hfdcan->Instance->TXEFS & FDCAN_TXEFS_EFGI) >> 8);
    TxEventAddress = (uint32_t *)(hfdcan->msgRam.TxEventFIFOSA + (GetIndex * 2 * 4));

    /* Retrieve IdType */
    pTxEvent->IdType = *TxEventAddress & FDCAN_ELEMENT_MASK_XTD;

    /* Retrieve Identifier */
    if(pTxEvent->IdType == FDCAN_STANDARD_ID) /* Standard ID element */
    {
      pTxEvent->Identifier = ((*TxEventAddress & FDCAN_ELEMENT_MASK_STDID) >> 18);
    }
    else /* Extended ID element */
    {
      pTxEvent->Identifier = (*TxEventAddress & FDCAN_ELEMENT_MASK_EXTID);
    }

    /* Retrieve RxFrameType */
    pTxEvent->TxFrameType = (*TxEventAddress & FDCAN_ELEMENT_MASK_RTR);

    /* Retrieve ErrorStateIndicator */
    pTxEvent->ErrorStateIndicator = (*TxEventAddress++ & FDCAN_ELEMENT_MASK_ESI);

    /* Retrieve RxTimestamp */
    pTxEvent->TxTimestamp = (*TxEventAddress & FDCAN_ELEMENT_MASK_TS);

    /* Retrieve DataLength */
    pTxEvent->DataLength = (*TxEventAddress & FDCAN_ELEMENT_MASK_DLC);

    /* Retrieve BitRateSwitch */
    pTxEvent->BitRateSwitch = (*TxEventAddress & FDCAN_ELEMENT_MASK_BRS);

    /* Retrieve FDFormat */
    pTxEvent->FDFormat = (*TxEventAddress & FDCAN_ELEMENT_MASK_FDF);

    /* Retrieve EventType */
    pTxEvent->EventType = (*TxEventAddress & FDCAN_ELEMENT_MASK_ET);

    /* Retrieve MessageMarker */
    pTxEvent->MessageMarker = ((*TxEventAddress & FDCAN_ELEMENT_MASK_MM) >> 24);

    /* Acknowledge the Tx Event FIFO that the oldest element is read so that it increments the GetIndex */
    hfdcan->Instance->TXEFA = GetIndex;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get high priority message status.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  HpMsgStatus: pointer to an FDCAN_HpMsgStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_GetHighPriorityMessageStatus(FDCAN_HandleTypeDef *hfdcan, FDCAN_HpMsgStatusTypeDef *HpMsgStatus)
{
  HpMsgStatus->FilterList = ((hfdcan->Instance->HPMS & FDCAN_HPMS_FLST) >> 15);
  HpMsgStatus->FilterIndex = ((hfdcan->Instance->HPMS & FDCAN_HPMS_FIDX) >> 8);
  HpMsgStatus->MessageStorage = (hfdcan->Instance->HPMS & FDCAN_HPMS_MSI);
  HpMsgStatus->MessageIndex = (hfdcan->Instance->HPMS & FDCAN_HPMS_BIDX);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Get protocol status.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ProtocolStatus: pointer to an FDCAN_ProtocolStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_GetProtocolStatus(FDCAN_HandleTypeDef *hfdcan, FDCAN_ProtocolStatusTypeDef *ProtocolStatus)
{
  uint32_t StatusReg;

  /* Read the protocol status register */
  StatusReg = READ_REG(hfdcan->Instance->PSR);

  /* Fill the protocol status structure */
  ProtocolStatus->LastErrorCode = (StatusReg & FDCAN_PSR_LEC);
  ProtocolStatus->DataLastErrorCode =  ((StatusReg & FDCAN_PSR_DLEC) >> 8);
  ProtocolStatus->Activity =  (StatusReg & FDCAN_PSR_ACT);
  ProtocolStatus->ErrorPassive =  ((StatusReg & FDCAN_PSR_EP) >> 5);
  ProtocolStatus->Warning =  ((StatusReg & FDCAN_PSR_EW) >> 6);
  ProtocolStatus->BusOff =  ((StatusReg & FDCAN_PSR_BO) >> 7);
  ProtocolStatus->RxESIflag =  ((StatusReg & FDCAN_PSR_RESI) >> 11);
  ProtocolStatus->RxBRSflag =  ((StatusReg & FDCAN_PSR_RBRS) >> 12);
  ProtocolStatus->RxFDFflag =  ((StatusReg & FDCAN_PSR_REDL) >> 13);
  ProtocolStatus->ProtocolException =  ((StatusReg & FDCAN_PSR_PXE) >> 14);
  ProtocolStatus->TDCvalue =  ((StatusReg & FDCAN_PSR_TDCV) >> 16);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Get error counter values.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ErrorCounters: pointer to an FDCAN_ErrorCountersTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_GetErrorCounters(FDCAN_HandleTypeDef *hfdcan, FDCAN_ErrorCountersTypeDef *ErrorCounters)
{
  uint32_t CountersReg;

  /* Read the error counters register */
  CountersReg = READ_REG(hfdcan->Instance->ECR);

  /* Fill the error counters structure */
  ErrorCounters->TxErrorCnt = (CountersReg & FDCAN_ECR_TEC);
  ErrorCounters->RxErrorCnt = ((CountersReg & FDCAN_ECR_TREC) >> 8);
  ErrorCounters->RxErrorPassive = ((CountersReg & FDCAN_ECR_RP) >> 15);
  ErrorCounters->ErrorLogging = ((CountersReg & FDCAN_ECR_CEL) >> 16);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Check if a new message is received in the selected Rx buffer.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxBufferIndex: Rx buffer index.
  *                        This parameter must be a number between 0 and 63.
  * @retval Status:
  *          - 0 : No new message on RxBufferIndex.
  *          - 1 : New message received on RxBufferIndex.
  */
uint32_t HAL_FDCAN_IsRxBufferMessageAvailable(FDCAN_HandleTypeDef *hfdcan, uint32_t RxBufferIndex)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_MAX_VALUE(RxBufferIndex, 63));

  /* Check new message reception on the selected buffer */
  if(((RxBufferIndex < 32) && ((hfdcan->Instance->NDAT1 & (1 << RxBufferIndex)) == 0)) ||
     ((RxBufferIndex >= 32) && ((hfdcan->Instance->NDAT2 & (1 << (RxBufferIndex - 0x20))) == 0)))
  {
    return 0;
  }

  /* Clear the New Data flag of the current Rx buffer */
  if(RxBufferIndex < 32)
  {
    hfdcan->Instance->NDAT1 = (1 << RxBufferIndex);
  }
  else /* 32 <= RxBufferIndex <= 63 */
  {
    hfdcan->Instance->NDAT2 = (1 << (RxBufferIndex - 0x20));
  }

  return 1;
}

/**
  * @brief  Check if a transmission request is pending on the selected Tx buffer.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TxBufferIndex: Tx buffer index.
  *                        This parameter can be a value of @arg FDCAN_Tx_location.
  * @retval Status:
  *          - 0 : No pending transmission request on RxBufferIndex.
  *          - 1 : Pending transmission request on RxBufferIndex.
  */
uint32_t HAL_FDCAN_IsTxBufferMessagePending(FDCAN_HandleTypeDef *hfdcan, uint32_t TxBufferIndex)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TX_LOCATION(TxBufferIndex));

  /* Check pending transmittion request on the selected buffer */
  if((hfdcan->Instance->TXBRP & TxBufferIndex) == 0)
  {
    return 0;
  }
  return 1;
}

/**
  * @brief  Return Rx FIFO fill level.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxFifo: Rx FIFO.
  *                 This parameter can be one of the following values:
  *                   @arg FDCAN_RX_FIFO0: Rx FIFO 0
  *                   @arg FDCAN_RX_FIFO1: Rx FIFO 1
  * @retval Level: Rx FIFO fill level.
  */
uint32_t HAL_FDCAN_GetRxFifoFillLevel(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo)
{
  uint32_t FillLevel;

  /* Check function parameters */
  assert_param(IS_FDCAN_RX_FIFO(RxFifo));

  if(RxFifo == FDCAN_RX_FIFO0)
  {
    FillLevel = hfdcan->Instance->RXF0S & FDCAN_RXF0S_F0FL;
  }
  else /* RxFifo == FDCAN_RX_FIFO1 */
  {
    FillLevel = hfdcan->Instance->RXF1S & FDCAN_RXF1S_F1FL;
  }

  /* Return Rx FIFO fill level */
  return FillLevel;
}

/**
  * @brief  Return Tx FIFO free level: number of consecutive free Tx FIFO
  *         elements starting from Tx FIFO GetIndex.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval Level: Tx FIFO free level.
  */
uint32_t HAL_FDCAN_GetTxFifoFreeLevel(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t FreeLevel;

  FreeLevel = hfdcan->Instance->TXFQS & FDCAN_TXFQS_TFFL;

  /* Return Tx FIFO free level */
  return FreeLevel;
}

/**
  * @brief  Check if the FDCAN peripheral entered Restricted Operation Mode.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval Status:
  *          - 0 : Normal FDCAN operation.
  *          - 1 : Restricted Operation Mode active.
  */
uint32_t HAL_FDCAN_IsRestrictedOperationMode(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t OperationMode;

  /* Get Operation Mode */
  OperationMode = ((hfdcan->Instance->CCCR & FDCAN_CCCR_ASM) >> 2);

  return OperationMode;
}

/**
  * @brief  Exit Restricted Operation Mode.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ExitRestrictedOperationMode(FDCAN_HandleTypeDef *hfdcan)
{
  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Exit Restricted Operation mode */
    CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_ASM);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group4 TT Configuration and control functions
 *  @brief    TT Configuration and control functions
 *
@verbatim
  ==============================================================================
              ##### TT Configuration and control functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) HAL_FDCAN_TT_ConfigOperation                  : Initialize TT operation parameters
      (+) HAL_FDCAN_TT_ConfigReferenceMessage           : Configure the reference message
      (+) HAL_FDCAN_TT_ConfigTrigger                    : Configure the FDCAN trigger
      (+) HAL_FDCAN_TT_SetGlobalTime                    : Schedule global time adjustment
      (+) HAL_FDCAN_TT_SetClockSynchronization          : Schedule TUR numerator update
      (+) HAL_FDCAN_TT_ConfigStopWatch                  : Configure stop watch source and polarity
      (+) HAL_FDCAN_TT_ConfigRegisterTimeMark           : Configure register time mark pulse generation
        (+) HAL_FDCAN_TT_EnableRegisterTimeMarkPulse      : Enable register time mark pulse generation
        (+) HAL_FDCAN_TT_DisableRegisterTimeMarkPulse     : Disable register time mark pulse generation
      (+) HAL_FDCAN_TT_EnableTriggerTimeMarkPulse       : Enable trigger time mark pulse generation
      (+) HAL_FDCAN_TT_DisableTriggerTimeMarkPulse      : Disable trigger time mark pulse generation
      (+) HAL_FDCAN_TT_EnableHardwareGapControl         : Enable gap control by input pin fdcan1_evt
      (+) HAL_FDCAN_TT_DisableHardwareGapControl        : Disable gap control by input pin fdcan1_evt
      (+) HAL_FDCAN_TT_EnableTimeMarkGapControl         : Enable gap control (finish only) by register time mark interrupt
      (+) HAL_FDCAN_TT_DisableTimeMarkGapControl        : Disable gap control by register time mark interrupt
      (+) HAL_FDCAN_TT_SetNextIsGap                     : Transmit next reference message with Next_is_Gap = "1"
      (+) HAL_FDCAN_TT_SetEndOfGap                      : Finish a Gap by requesting start of reference message
      (+) HAL_FDCAN_TT_ConfigExternalSyncPhase          : Configure target phase used for external synchronization
        (+) HAL_FDCAN_TT_EnableExternalSynchronization    : Synchronize the phase of the FDCAN schedule to an external schedule
        (+) HAL_FDCAN_TT_DisableExternalSynchronization   : Disable external schedule synchronization
      (+) HAL_FDCAN_TT_GetOperationStatus               : Get TT operation status

@endverbatim
  * @{
  */

/**
  * @brief  Initialize TT operation parameters.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  pTTParams: pointer to a FDCAN_TT_ConfigTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigOperation(FDCAN_HandleTypeDef *hfdcan, FDCAN_TT_ConfigTypeDef *pTTParams)
{
  uint32_t tickstart = 0U;
  uint32_t RAMcounter;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_TUR_NUMERATOR(pTTParams->TURNumerator));
  assert_param(IS_FDCAN_TT_TUR_DENOMINATOR(pTTParams->TURDenominator));
  assert_param(IS_FDCAN_TT_TIME_MASTER(pTTParams->TimeMaster));
  assert_param(IS_FDCAN_MAX_VALUE(pTTParams->SyncDevLimit, 7));
  assert_param(IS_FDCAN_MAX_VALUE(pTTParams->InitRefTrigOffset, 127));
  assert_param(IS_FDCAN_MAX_VALUE(pTTParams->TriggerMemoryNbr, 64));
  assert_param(IS_FDCAN_TT_CYCLE_START_SYNC(pTTParams->CycleStartSync));
  assert_param(IS_FDCAN_TT_STOP_WATCH_TRIGGER(pTTParams->StopWatchTrigSel));
  assert_param(IS_FDCAN_TT_EVENT_TRIGGER(pTTParams->EventTrigSel));
  if(pTTParams->TimeMaster == FDCAN_TT_POTENTIAL_MASTER)
  {
    assert_param(IS_FDCAN_TT_BASIC_CYCLES_NUMBER(pTTParams->BasicCyclesNbr));
  }
  if(pTTParams->OperationMode != FDCAN_TT_COMMUNICATION_LEVEL0)
  {
    assert_param(IS_FDCAN_TT_OPERATION(pTTParams->GapEnable));
    assert_param(IS_FDCAN_MAX_VALUE(pTTParams->AppWdgLimit, 255));
    assert_param(IS_FDCAN_TT_EVENT_TRIGGER_POLARITY(pTTParams->EvtTrigPolarity));
    assert_param(IS_FDCAN_TT_TX_ENABLE_WINDOW(pTTParams->TxEnableWindow));
    assert_param(IS_FDCAN_MAX_VALUE(pTTParams->ExpTxTrigNbr, 4095));
  }
  if(pTTParams->OperationMode != FDCAN_TT_COMMUNICATION_LEVEL1)
  {
    assert_param(IS_FDCAN_TT_TUR_LEVEL_0_2(pTTParams->TURNumerator, pTTParams->TURDenominator));
    assert_param(IS_FDCAN_TT_EXTERNAL_CLK_SYNC(pTTParams->ExternalClkSync));
    assert_param(IS_FDCAN_TT_GLOBAL_TIME_FILTERING(pTTParams->GlobalTimeFilter));
    assert_param(IS_FDCAN_TT_AUTO_CLK_CALIBRATION(pTTParams->ClockCalibration));
  }
  else
  {
    assert_param(IS_FDCAN_TT_TUR_LEVEL_1(pTTParams->TURNumerator, pTTParams->TURDenominator));
  }

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Stop local time in order to enable write access to the other bits of TURCF register */
    CLEAR_BIT(hfdcan->ttcan->TURCF, FDCAN_TURCF_ELT);

    /* Get tick */
    tickstart = HAL_GetTick();

    /* Wait until the ELT bit into TURCF register is reset */
    while((hfdcan->ttcan->TURCF & FDCAN_TURCF_ELT) != RESET)
    {
      /* Check for the Timeout */
      if((HAL_GetTick() - tickstart) > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Configure TUR (Time Unit Ratio) */
    MODIFY_REG(hfdcan->ttcan->TURCF,
               (FDCAN_TURCF_NCL | FDCAN_TURCF_DC),
               ((pTTParams->TURNumerator - 0x10000) | (pTTParams->TURDenominator << 16)));

    /* Enable local time */
    SET_BIT(hfdcan->ttcan->TURCF, FDCAN_TURCF_ELT);

    /* Configure TT operation */
    MODIFY_REG(hfdcan->ttcan->TTOCF,
               (FDCAN_TTOCF_OM | FDCAN_TTOCF_TM | FDCAN_TTOCF_LDSDL | FDCAN_TTOCF_IRTO),
               (pTTParams->OperationMode            | \
                pTTParams->TimeMaster               | \
                (pTTParams->SyncDevLimit << 5)      | \
                (pTTParams->InitRefTrigOffset << 8)));
    if(pTTParams->OperationMode != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      MODIFY_REG(hfdcan->ttcan->TTOCF,
                (FDCAN_TTOCF_GEN | FDCAN_TTOCF_AWL | FDCAN_TTOCF_EVTP),
                (pTTParams->GapEnable           | \
                 (pTTParams->AppWdgLimit << 16) | \
                 pTTParams->EvtTrigPolarity));
    }
    if(pTTParams->OperationMode != FDCAN_TT_COMMUNICATION_LEVEL1)
    {
      MODIFY_REG(hfdcan->ttcan->TTOCF,
                 (FDCAN_TTOCF_EECS | FDCAN_TTOCF_EGTF | FDCAN_TTOCF_ECC),
                 (pTTParams->ExternalClkSync  | \
                  pTTParams->GlobalTimeFilter | \
                  pTTParams->ClockCalibration));
    }

    /* Configure system matrix limits */
    MODIFY_REG(hfdcan->ttcan->TTMLM, FDCAN_TTMLM_CSS, pTTParams->CycleStartSync);
    if(pTTParams->OperationMode != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      MODIFY_REG(hfdcan->ttcan->TTMLM,
                 (FDCAN_TTMLM_TXEW | FDCAN_TTMLM_ENTT),
                 (((pTTParams->TxEnableWindow - 1) << 8) | (pTTParams->ExpTxTrigNbr << 16)));
    }
    if(pTTParams->TimeMaster == FDCAN_TT_POTENTIAL_MASTER)
    {
      MODIFY_REG(hfdcan->ttcan->TTMLM, FDCAN_TTMLM_CCM, pTTParams->BasicCyclesNbr);
    }

    /* Configure input triggers: Stop watch and Event */
    MODIFY_REG(hfdcan->ttcan->TTTS,
               (FDCAN_TTTS_SWTSEL | FDCAN_TTTS_EVTSEL),
               (pTTParams->StopWatchTrigSel | pTTParams->EventTrigSel));

    /* Configure trigger memory start address */
     hfdcan->msgRam.TTMemorySA = (hfdcan->msgRam.EndAddress - SRAMCAN_BASE) / 4;
     MODIFY_REG(hfdcan->ttcan->TTTMC, FDCAN_TTTMC_TMSA, (hfdcan->msgRam.TTMemorySA << 2));

     /* Trigger memory elements number */
     MODIFY_REG(hfdcan->ttcan->TTTMC, FDCAN_TTTMC_TME, (pTTParams->TriggerMemoryNbr << 16));

    /* Recalculate End Address */
    hfdcan->msgRam.TTMemorySA = SRAMCAN_BASE + (hfdcan->msgRam.TTMemorySA * 4);
    hfdcan->msgRam.EndAddress = hfdcan->msgRam.TTMemorySA + (pTTParams->TriggerMemoryNbr * 2 * 4);

    if(hfdcan->msgRam.EndAddress > 0x4000B5FC) /* Last address of the Message RAM */
    {
      /* Update error code.
         Message RAM overflow */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

      return HAL_ERROR;
    }
    else
    {
      /* Flush the allocated Message RAM area */
      for(RAMcounter = hfdcan->msgRam.TTMemorySA; RAMcounter < hfdcan->msgRam.EndAddress; RAMcounter += 4)
      {
        *(__IO uint32_t *)(RAMcounter) = 0x00000000;
      }
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the reference message.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  IdType: Identifier Type.
  *                 This parameter can be a value of @arg FDCAN_id_type.
  * @param  Identifier: Reference Identifier.
  *                     This parameter must be a number between:
  *                      - 0 and 0x7FF, if IdType is FDCAN_STANDARD_ID
  *                      - 0 and 0x1FFFFFFF, if IdType is FDCAN_EXTENDED_ID
  * @param  Payload: Enable or disable the additional payload.
  *                  This parameter can be a value of @arg FDCAN_TT_Reference_Message_Payload.
  *                  This parameter is ignored in case of time slaves.
  *                  If this parameter is set to FDCAN_TT_REF_MESSAGE_ADD_PAYLOAD, the
  *                  following elements are taken from Tx Buffer 0:
  *                   - MessageMarker
  *                   - TxEventFifoControl
  *                   - DataLength
  *                   - Data Bytes (payload):
  *                      - bytes 2-8, for Level 1
  *                      - bytes 5-8, for Level 0 and Level 2
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigReferenceMessage(FDCAN_HandleTypeDef *hfdcan, uint32_t IdType, uint32_t Identifier, uint32_t Payload)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_ID_TYPE(IdType));
  if(IdType == FDCAN_STANDARD_ID)
  {
    assert_param(IS_FDCAN_MAX_VALUE(Identifier, 0x7FF));
  }
  else /* IdType == FDCAN_EXTENDED_ID */
  {
    assert_param(IS_FDCAN_MAX_VALUE(Identifier, 0x1FFFFFFF));
  }
  assert_param(IS_FDCAN_TT_REFERENCE_MESSAGE_PAYLOAD(Payload));

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Configure reference message identifier type, identifier and payload */
    if(IdType == FDCAN_EXTENDED_ID)
    {
      MODIFY_REG(hfdcan->ttcan->TTRMC, (FDCAN_TTRMC_RID | FDCAN_TTRMC_XTD | FDCAN_TTRMC_RMPS), (Payload | IdType | Identifier));
    }
    else /* IdType == FDCAN_STANDARD_ID */
    {
      MODIFY_REG(hfdcan->ttcan->TTRMC, (FDCAN_TTRMC_RID | FDCAN_TTRMC_XTD | FDCAN_TTRMC_RMPS), (Payload | IdType | (Identifier << 18)));
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the FDCAN trigger according to the specified
  *         parameters in the FDCAN_TriggerTypeDef structure.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  sTriggerConfig: pointer to an FDCAN_TriggerTypeDef structure that
  *         contains the trigger configuration information
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigTrigger(FDCAN_HandleTypeDef* hfdcan, FDCAN_TriggerTypeDef* sTriggerConfig)
{
  uint32_t CycleCode;
  uint32_t MessageNumber;
  uint32_t TriggerElementW1;
  uint32_t TriggerElementW2;
  uint32_t *TriggerAddress;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_MAX_VALUE(sTriggerConfig->TriggerIndex, 63));
  assert_param(IS_FDCAN_MAX_VALUE(sTriggerConfig->TimeMark, 0xFFFF));
  assert_param(IS_FDCAN_TT_REPEAT_FACTOR(sTriggerConfig->RepeatFactor));
  if(sTriggerConfig->RepeatFactor != FDCAN_TT_REPEAT_EVERY_CYCLE)
  {
    assert_param(IS_FDCAN_MAX_VALUE(sTriggerConfig->StartCycle, (sTriggerConfig->RepeatFactor - 1)));
  }
  assert_param(IS_FDCAN_TT_TM_EVENT_INTERNAL(sTriggerConfig->TmEventInt));
  assert_param(IS_FDCAN_TT_TM_EVENT_EXTERNAL(sTriggerConfig->TmEventExt));
  assert_param(IS_FDCAN_TT_TRIGGER_TYPE(sTriggerConfig->TriggerType));
  assert_param(IS_FDCAN_ID_TYPE(sTriggerConfig->FilterType));
  if((sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_SINGLE     ) ||
     (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_CONTINUOUS ) ||
     (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_ARBITRATION) ||
     (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_MERGED     ))
  {
    assert_param(IS_FDCAN_TX_LOCATION(sTriggerConfig->TxBufferIndex));
  }
  if(sTriggerConfig->TriggerType == FDCAN_TT_RX_TRIGGER)
  {
    if(sTriggerConfig->FilterType == FDCAN_STANDARD_ID)
    {
      assert_param(IS_FDCAN_MAX_VALUE(sTriggerConfig->FilterIndex, 63));
    }
    else /* sTriggerConfig->FilterType == FDCAN_EXTENDED_ID */
    {
      assert_param(IS_FDCAN_MAX_VALUE(sTriggerConfig->FilterIndex, 127));
    }
  }

  if(hfdcan->State == HAL_FDCAN_STATE_READY)
  {
    /* Calculate cycle code */
    if(sTriggerConfig->RepeatFactor == FDCAN_TT_REPEAT_EVERY_CYCLE)
    {
      CycleCode = FDCAN_TT_REPEAT_EVERY_CYCLE;
    }
    else /* sTriggerConfig->RepeatFactor != FDCAN_TT_REPEAT_EVERY_CYCLE */
    {
      CycleCode = sTriggerConfig->RepeatFactor + sTriggerConfig->StartCycle;
    }

    /* Build first word of trigger element */
    TriggerElementW1 = ((sTriggerConfig->TimeMark << 16) | \
                        (CycleCode << 8)                 | \
                        sTriggerConfig->TmEventInt       | \
                        sTriggerConfig->TmEventExt       | \
                        sTriggerConfig->TriggerType);

    /* Select message number depending on trigger type (transmission or reception) */
    if(sTriggerConfig->TriggerType == FDCAN_TT_RX_TRIGGER)
    {
      MessageNumber = sTriggerConfig->FilterIndex;
    }
    else if((sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_SINGLE     ) ||
            (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_CONTINUOUS ) ||
            (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_ARBITRATION) ||
            (sTriggerConfig->TriggerType == FDCAN_TT_TX_TRIGGER_MERGED     ))
    {
      MessageNumber = POSITION_VAL(sTriggerConfig->TxBufferIndex);
    }
    else
    {
      MessageNumber = 0U;
    }

    /* Build second word of trigger element */
    TriggerElementW2 = ((sTriggerConfig->FilterType >> 7) | (MessageNumber << 16));

    /* Calculate trigger address */
    TriggerAddress = (uint32_t *)(hfdcan->msgRam.TTMemorySA + (sTriggerConfig->TriggerIndex * 4 * 2));

    /* Write trigger element to the message RAM */
    *TriggerAddress++ = TriggerElementW1;
    *TriggerAddress = TriggerElementW2;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Schedule global time adjustment for the next reference message.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TimePreset: time preset value.
  *                     This parameter must be a number between:
  *                      - 0x0000 and 0x7FFF, Next_Master_Ref_Mark = Current_Master_Ref_Mark + TimePreset
  *                      or:
  *                      - 0x8001 and 0xFFFF, Next_Master_Ref_Mark = Current_Master_Ref_Mark - (0x10000 - TimePreset)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_SetGlobalTime(FDCAN_HandleTypeDef* hfdcan, uint32_t TimePreset)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_TIME_PRESET(TimePreset));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the external clock synchronization is enabled */
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_EECS) != FDCAN_TTOCF_EECS)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }

    /* Check that no global time preset is pending */
    if((hfdcan->ttcan->TTOST & FDCAN_TTOST_WGTD) == FDCAN_TTOST_WGTD)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PENDING;

      return HAL_ERROR;
    }

    /* Configure time preset */
    MODIFY_REG(hfdcan->ttcan->TTGTP, FDCAN_TTGTP_TP, TimePreset);

    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Schedule time preset to take effect by the next reference message */
    SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_SGT);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Schedule TUR numerator update for the next reference message.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  NewTURNumerator: new value of the TUR numerator.
  *                          This parameter must be a number between 0x10000 and 0x1FFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_SetClockSynchronization(FDCAN_HandleTypeDef* hfdcan, uint32_t NewTURNumerator)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_TUR_NUMERATOR(NewTURNumerator));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the external clock synchronization is enabled */
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_EECS) != FDCAN_TTOCF_EECS)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }

    /* Check that no external clock synchronization is pending */
    if((hfdcan->ttcan->TTOST & FDCAN_TTOST_WECS) == FDCAN_TTOST_WECS)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PENDING;

      return HAL_ERROR;
    }

    /* Configure new TUR numerator */
    MODIFY_REG(hfdcan->ttcan->TURCF, FDCAN_TURCF_NCL, (NewTURNumerator - 0x10000));

    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Schedule TUR numerator update by the next reference message */
    SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_ECS);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure stop watch source and polarity.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  Source: stop watch source.
  *                 This parameter can be a value of @arg FDCAN_TT_stop_watch_source.
  * @param  Polarity: stop watch polarity.
  *                   This parameter can be a value of @arg FDCAN_TT_stop_watch_polarity.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigStopWatch(FDCAN_HandleTypeDef* hfdcan, uint32_t Source, uint32_t Polarity)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_STOP_WATCH_SOURCE(Source));
  assert_param(IS_FDCAN_TT_STOP_WATCH_POLARITY(Polarity));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Select stop watch source and polarity */
    MODIFY_REG(hfdcan->ttcan->TTOCN, (FDCAN_TTOCN_SWS | FDCAN_TTOCN_SWP), (Source | Polarity));

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure register time mark pulse generation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TimeMarkSource: time mark source.
  *                         This parameter can be a value of @arg FDCAN_TT_time_mark_source.
  * @param  TimeMarkValue: time mark value (reference).
  *                        This parameter must be a number between 0 and 0xFFFF.
  * @param  RepeatFactor: repeat factor of the cycle for which the time mark is valid.
  *                       This parameter can be a value of @arg FDCAN_TT_Repeat_Factor.
  * @param  StartCycle: index of the first cycle in which the time mark becomes valid.
  *                     This parameter is ignored if RepeatFactor is set to FDCAN_TT_REPEAT_EVERY_CYCLE.
  *                     This parameter must be a number between 0 and RepeatFactor.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigRegisterTimeMark(FDCAN_HandleTypeDef* hfdcan,
                                                     uint32_t TimeMarkSource, uint32_t TimeMarkValue,
                                                     uint32_t RepeatFactor, uint32_t StartCycle)
{
  uint32_t Counter = 0U;
  uint32_t CycleCode;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_REGISTER_TIME_MARK_SOURCE(TimeMarkSource));
  assert_param(IS_FDCAN_MAX_VALUE(TimeMarkValue, 0xFFFF));
  assert_param(IS_FDCAN_TT_REPEAT_FACTOR(RepeatFactor));
  if(RepeatFactor != FDCAN_TT_REPEAT_EVERY_CYCLE)
  {
    assert_param(IS_FDCAN_MAX_VALUE(StartCycle, (RepeatFactor - 1)));
  }

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Disable the time mark compare function */
    CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TMC);

    if(TimeMarkSource != FDCAN_TT_REG_TIMEMARK_DIABLED)
    {
      /* Calculate cycle code */
      if(RepeatFactor == FDCAN_TT_REPEAT_EVERY_CYCLE)
      {
        CycleCode = FDCAN_TT_REPEAT_EVERY_CYCLE;
      }
      else /* RepeatFactor != FDCAN_TT_REPEAT_EVERY_CYCLE */
      {
        CycleCode = RepeatFactor + StartCycle;
      }

      Counter = 0U;

      /* Wait until the LCKM bit into TTTMK register is reset */
      while((hfdcan->ttcan->TTTMK & FDCAN_TTTMK_LCKM) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Configure time mark value and cycle code */
      hfdcan->ttcan->TTTMK = (TimeMarkValue | (CycleCode << 16));

      Counter = 0U;

      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Update the register time mark compare source */
      MODIFY_REG(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TMC, TimeMarkSource);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable register time mark pulse generation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_EnableRegisterTimeMarkPulse(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Enable Register Time Mark Interrupt output on fdcan1_rtp */
    SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_RTIE);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable register time mark pulse generation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DisableRegisterTimeMarkPulse(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Disable Register Time Mark Interrupt output on fdcan1_rtp */
    CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_RTIE);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable trigger time mark pulse generation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_EnableTriggerTimeMarkPulse(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Enable Trigger Time Mark Interrupt output on fdcan1_tmp */
      SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TTIE);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable trigger time mark pulse generation.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DisableTriggerTimeMarkPulse(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Disable Trigger Time Mark Interrupt output on fdcan1_rtp */
      CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TTIE);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable gap control by input pin fdcan1_evt.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_EnableHardwareGapControl(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Enable gap control by pin fdcan1_evt */
      SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_GCS);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable gap control by input pin fdcan1_evt.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DisableHardwareGapControl(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Disable gap control by pin fdcan1_evt */
      CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_GCS);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable gap control (finish only) by register time mark interrupt.
  *         The next register time mark interrupt (TTIR.RTMI = "1") will finish
  *         the Gap and start the reference message.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_EnableTimeMarkGapControl(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Enable gap control by register time mark interrupt */
      SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TMG);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable gap control by register time mark interrupt.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DisableTimeMarkGapControl(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Disable gap control by register time mark interrupt */
      CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_TMG);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Transmit next reference message with Next_is_Gap = "1".
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_SetNextIsGap(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the node is configured for external event-synchronized TT operation */
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_GEN) != FDCAN_TTOCF_GEN)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }

    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Set Next is Gap */
      SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_NIG);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Finish a Gap by requesting start of reference message.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_SetEndOfGap(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that the node is configured for external event-synchronized TT operation */
    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_GEN) != FDCAN_TTOCF_GEN)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }

    if((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != FDCAN_TT_COMMUNICATION_LEVEL0)
    {
      /* Wait until the LCKC bit into TTOCN register is reset */
      while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
      {
        /* Check for the Timeout */
        if(Counter++ > FDCAN_TIMEOUT_VALUE)
        {
          /* Update error code */
          hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

          /* Change FDCAN state */
          hfdcan->State = HAL_FDCAN_STATE_ERROR;

          return HAL_ERROR;
        }
      }

      /* Set Finish Gap */
      SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_FGP);

      /* Return function status */
      return HAL_OK;
    }
    else
    {
      /* Update error code.
         Feature not supported for TT Level 0 */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_SUPPORTED;

      return HAL_ERROR;
    }
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure target phase used for external synchronization by event
  *         trigger input pin fdcan1_evt.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TargetPhase: defines target value of cycle time when a rising edge
  *                      of fdcan1_evt is expected.
  *                      This parameter must be a number between 0 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigExternalSyncPhase(FDCAN_HandleTypeDef *hfdcan, uint32_t TargetPhase)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_MAX_VALUE(TargetPhase, 0xFFFF));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Check that no external schedule synchronization is pending */
    if((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_ESCN) == FDCAN_TTOCN_ESCN)
    {
      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PENDING;

      return HAL_ERROR;
    }

    /* Configure cycle time target phase */
    MODIFY_REG(hfdcan->ttcan->TTGTP, FDCAN_TTGTP_CTP, (TargetPhase << 16));

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Synchronize the phase of the FDCAN schedule to an external schedule
  *         using event trigger input pin fdcan1_evt.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_EnableExternalSynchronization(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Enable external synchronization */
    SET_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_ESCN);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable external schedule synchronization.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DisableExternalSynchronization(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t Counter = 0U;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Wait until the LCKC bit into TTOCN register is reset */
    while((hfdcan->ttcan->TTOCN & FDCAN_TTOCN_LCKC) != RESET)
    {
      /* Check for the Timeout */
      if(Counter++ > FDCAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hfdcan->ErrorCode |= HAL_FDCAN_ERROR_TIMEOUT;

        /* Change FDCAN state */
        hfdcan->State = HAL_FDCAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }

    /* Disable external synchronization */
    CLEAR_BIT(hfdcan->ttcan->TTOCN, FDCAN_TTOCN_ESCN);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get TT operation status.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TTOpStatus: pointer to an FDCAN_TTOperationStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_GetOperationStatus(FDCAN_HandleTypeDef *hfdcan, FDCAN_TTOperationStatusTypeDef *TTOpStatus)
{
  uint32_t TTStatusReg;

  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));

  /* Read the TT operation status register */
  TTStatusReg = READ_REG(hfdcan->ttcan->TTOST);

  /* Fill the TT operation status structure */
  TTOpStatus->ErrorLevel = (TTStatusReg & FDCAN_TTOST_EL);
  TTOpStatus->MasterState =  (TTStatusReg & FDCAN_TTOST_MS);
  TTOpStatus->SyncState =  (TTStatusReg & FDCAN_TTOST_SYS);
  TTOpStatus->GTimeQuality =  ((TTStatusReg & FDCAN_TTOST_QGTP) >> 6);
  TTOpStatus->ClockQuality =  ((TTStatusReg & FDCAN_TTOST_QCS) >> 7);
  TTOpStatus->RefTrigOffset =  ((TTStatusReg & FDCAN_TTOST_RTO) >> 8);
  TTOpStatus->GTimeDiscPending =  ((TTStatusReg & FDCAN_TTOST_WGTD) >> 22);
  TTOpStatus->GapFinished =  ((TTStatusReg & FDCAN_TTOST_GFI) >> 23);
  TTOpStatus->MasterPriority =  ((TTStatusReg & FDCAN_TTOST_TMP) >> 24);
  TTOpStatus->GapStarted =  ((TTStatusReg & FDCAN_TTOST_GSI) >> 27);
  TTOpStatus->WaitForEvt =  ((TTStatusReg & FDCAN_TTOST_WFE) >> 28);
  TTOpStatus->AppWdgEvt =  ((TTStatusReg & FDCAN_TTOST_AWE) >> 29);
  TTOpStatus->ECSPending =  ((TTStatusReg & FDCAN_TTOST_WECS) >> 30);
  TTOpStatus->PhaseLock =  ((TTStatusReg & FDCAN_TTOST_SPL) >> 31);

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group5 Interrupts management
 *  @brief    Interrupts management
 *
@verbatim
  ==============================================================================
                       ##### Interrupts management #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) HAL_FDCAN_ConfigInterruptLines      : Assign interrupts to either Interrupt line 0 or 1
      (+) HAL_FDCAN_TT_ConfigInterruptLines   : Assign TT interrupts to either Interrupt line 0 or 1
      (+) HAL_FDCAN_ActivateNotification      : Enable interrupts
      (+) HAL_FDCAN_DeactivateNotification    : Disable interrupts
      (+) HAL_FDCAN_TT_ActivateNotification   : Enable TT interrupts
      (+) HAL_FDCAN_TT_DeactivateNotification : Disable TT interrupts
      (+) HAL_FDCAN_IRQHandler                : Handles FDCAN interrupt request

@endverbatim
  * @{
  */

/**
  * @brief  Assign interrupts to either Interrupt line 0 or 1.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ITList: indicates which interrupts will be assigned to the selected interrupt line.
  *                 This parameter can be any combination of @arg FDCAN_Interrupts.
  * @param  InterruptLine: Interrupt line.
  *                        This parameter can be a value of @arg FDCAN_Interrupt_Line.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ConfigInterruptLines(FDCAN_HandleTypeDef *hfdcan, uint32_t ITList, uint32_t InterruptLine)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_IT(ITList));
  assert_param(IS_FDCAN_IT_LINE(InterruptLine));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Assign list of interrupts to the selected line */
    if(InterruptLine == FDCAN_INTERRUPT_LINE0)
    {
      CLEAR_BIT(hfdcan->Instance->ILS, ITList);
    }
    else /* InterruptLine == FDCAN_INTERRUPT_LINE1 */
    {
      SET_BIT(hfdcan->Instance->ILS, ITList);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Assign TT interrupts to either Interrupt line 0 or 1.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TTITList: indicates which interrupts will be assigned to the selected interrupt line.
  *                   This parameter can be any combination of @arg FDCAN_TTInterrupts.
  * @param  InterruptLine: Interrupt line.
  *                        This parameter can be a value of @arg FDCAN_Interrupt_Line.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ConfigInterruptLines(FDCAN_HandleTypeDef *hfdcan, uint32_t TTITList, uint32_t InterruptLine)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TT_INSTANCE(hfdcan->Instance));
  assert_param(IS_FDCAN_TT_IT(TTITList));
  assert_param(IS_FDCAN_IT_LINE(InterruptLine));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Assign list of interrupts to the selected line */
    if(InterruptLine == FDCAN_INTERRUPT_LINE0)
    {
      CLEAR_BIT(hfdcan->ttcan->TTILS, TTITList);
    }
    else /* InterruptLine == FDCAN_INTERRUPT_LINE1 */
    {
      SET_BIT(hfdcan->ttcan->TTILS, TTITList);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable interrupts.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ActiveITs: indicates which interrupts will be enabled.
  *                    This parameter can be any combination of @arg FDCAN_Interrupts.
  * @param  BufferIndexes: Tx Buffer Indexes.
  *                        This parameter can be any combination of @arg FDCAN_Tx_location.
  *                        This parameter is ignored if ActiveITs does not include one of the following:
  *                         - FDCAN_IT_TX_COMPLETE
  *                         - FDCAN_IT_TX_ABORT_COMPLETE
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_ActivateNotification(FDCAN_HandleTypeDef *hfdcan, uint32_t ActiveITs, uint32_t BufferIndexes)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_IT(ActiveITs));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Enable Interrupt lines */
    if((ActiveITs & hfdcan->Instance->ILS) == RESET)
    {
      /* Enable Interrupt line 0 */
      SET_BIT(hfdcan->Instance->ILE, FDCAN_INTERRUPT_LINE0);
    }
    else if((ActiveITs & hfdcan->Instance->ILS) == ActiveITs)
    {
      /* Enable Interrupt line 1 */
      SET_BIT(hfdcan->Instance->ILE, FDCAN_INTERRUPT_LINE1);
    }
    else
    {
      /* Enable Interrupt lines 0 and 1 */
      hfdcan->Instance->ILE = (FDCAN_INTERRUPT_LINE0 | FDCAN_INTERRUPT_LINE1);
    }

    if((ActiveITs & FDCAN_IT_TX_COMPLETE) != RESET)
    {
      /* Enable Tx Buffer Transmission Interrupt to set TC flag in IR register,
         but interrupt will only occure if TC is enabled in IE register */
      SET_BIT(hfdcan->Instance->TXBTIE, BufferIndexes);
    }

    if((ActiveITs & FDCAN_IT_TX_ABORT_COMPLETE) != RESET)
    {
      /* Enable Tx Buffer Cancellation Finished Interrupt to set TCF flag in IR register,
         but interrupt will only occure if TCF is enabled in IE register */
      SET_BIT(hfdcan->Instance->TXBCIE, BufferIndexes);
    }

    /* Enable the selected interrupts */
    __HAL_FDCAN_ENABLE_IT(hfdcan, ActiveITs);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable interrupts.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  InactiveITs: indicates which interrupts will be disabled.
  *                      This parameter can be any combination of @arg FDCAN_Interrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_DeactivateNotification(FDCAN_HandleTypeDef *hfdcan, uint32_t InactiveITs)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_IT(InactiveITs));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Disable the selected interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, InactiveITs);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable TT interrupts.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ActiveTTITs: indicates which TT interrupts will be enabled.
  *                      This parameter can be any combination of @arg FDCAN_TTInterrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_ActivateNotification(FDCAN_HandleTypeDef *hfdcan, uint32_t ActiveTTITs)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TT_IT(ActiveTTITs));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Enable Interrupt lines */
    if((ActiveTTITs & hfdcan->ttcan->TTILS) == RESET)
    {
      /* Enable Interrupt line 0 */
      SET_BIT(hfdcan->Instance->ILE, FDCAN_INTERRUPT_LINE0);
    }
    else if((ActiveTTITs & hfdcan->ttcan->TTILS) == ActiveTTITs)
    {
      /* Enable Interrupt line 1 */
      SET_BIT(hfdcan->Instance->ILE, FDCAN_INTERRUPT_LINE1);
    }
    else
    {
      /* Enable Interrupt lines 0 and 1 */
      hfdcan->Instance->ILE = (FDCAN_INTERRUPT_LINE0 | FDCAN_INTERRUPT_LINE1);
    }

    /* Enable the selected TT interrupts */
    __HAL_FDCAN_TT_ENABLE_IT(hfdcan, ActiveTTITs);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable TT interrupts.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  InactiveTTITs: indicates which TT interrupts will be disabled.
  *                        This parameter can be any combination of @arg FDCAN_TTInterrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FDCAN_TT_DeactivateNotification(FDCAN_HandleTypeDef *hfdcan, uint32_t InactiveTTITs)
{
  /* Check function parameters */
  assert_param(IS_FDCAN_TT_IT(InactiveTTITs));

  if((hfdcan->State == HAL_FDCAN_STATE_READY) || (hfdcan->State == HAL_FDCAN_STATE_BUSY))
  {
    /* Disable the selected TT interrupts */
    __HAL_FDCAN_TT_DISABLE_IT(hfdcan, InactiveTTITs);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Handles FDCAN interrupt request.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL status
  */
void HAL_FDCAN_IRQHandler(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t ClkCalibrationITs;
  uint32_t TxEventFifoITs;
  uint32_t RxFifo0ITs;
  uint32_t RxFifo1ITs;
  uint32_t ErrStatus;
  uint32_t TransmittedBuffers;
  uint32_t AbortedBuffers;
  uint32_t TTSchedSyncITs;
  uint32_t TTTimeMarkITs;
  uint32_t TTGlobTimeITs;
  uint32_t TTDistErrors;
  uint32_t TTFatalErrors;
  uint32_t SWTime;
  uint32_t SWCycleCount;

  ClkCalibrationITs = (FDCAN_CCU->IR << 30);
  ClkCalibrationITs &= (FDCAN_CCU->IE << 30);
  TxEventFifoITs = hfdcan->Instance->IR & FDCAN_TX_EVENT_FIFO_MASK;
  TxEventFifoITs &= hfdcan->Instance->IE;
  RxFifo0ITs = hfdcan->Instance->IR & FDCAN_RX_FIFO0_MASK;
  RxFifo0ITs &= hfdcan->Instance->IE;
  RxFifo1ITs = hfdcan->Instance->IR & FDCAN_RX_FIFO1_MASK;
  RxFifo1ITs &= hfdcan->Instance->IE;
  ErrStatus = hfdcan->Instance->IR & FDCAN_ERROR_MASK;
  ErrStatus &= hfdcan->Instance->IE;

  /* High Priority Message interrupt management *******************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_RX_HIGH_PRIORITY_MSG) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_RX_HIGH_PRIORITY_MSG) != RESET)
    {
      /* Disable the High Priority Message interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_RX_HIGH_PRIORITY_MSG);

      /* Clear the High Priority Message flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RX_HIGH_PRIORITY_MSG);

      /* High Priority Message Callback */
      HAL_FDCAN_HighPriorityMessageCallback(hfdcan);
    }
  }

  /* Transmission Abort interrupt management **********************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_TX_ABORT_COMPLETE) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_TX_ABORT_COMPLETE) != RESET)
    {
      /* Disable the Transmission Cancellation interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_TX_ABORT_COMPLETE);

      /* List of aborted monitored buffers */
      AbortedBuffers = hfdcan->Instance->TXBCF;
      AbortedBuffers &= hfdcan->Instance->TXBCIE;

      /* Disable the Tx Buffer Cancellation Finished Interrupt */
      CLEAR_BIT(hfdcan->Instance->TXBCIE, AbortedBuffers);

      /* Clear the Transmission Cancellation flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TX_ABORT_COMPLETE);

      /* Transmission Cancellation Callback */
      HAL_FDCAN_TxBufferAbortCallback(hfdcan, AbortedBuffers);
    }
  }

  /* Clock calibration unit interrupts management *****************************/
  if(ClkCalibrationITs != 0U)
  {
    /* Disable the Clock Calibration interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, ClkCalibrationITs);

    /* Clear the Clock Calibration flags */
    __HAL_FDCAN_CLEAR_FLAG(hfdcan, ClkCalibrationITs);

    /* Clock Calibration Callback */
    HAL_FDCAN_ClockCalibrationCallback(hfdcan, ClkCalibrationITs);
  }

  /* Tx event FIFO interrupts management **************************************/
  if(TxEventFifoITs != 0U)
  {
    /* Disable the Tx Event FIFO interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, TxEventFifoITs);

    /* Clear the Tx Event FIFO flags */
    __HAL_FDCAN_CLEAR_FLAG(hfdcan, TxEventFifoITs);

    /* Tx Event FIFO Callback */
    HAL_FDCAN_TxEventFifoCallback(hfdcan, TxEventFifoITs);
  }

  /* Rx FIFO 0 interrupts management ******************************************/
  if(RxFifo0ITs != 0U)
  {
    /* Disable the Rx FIFO 0 interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, RxFifo0ITs);

    /* Clear the Rx FIFO 0 flags */
    __HAL_FDCAN_CLEAR_FLAG(hfdcan, RxFifo0ITs);

    /* Rx FIFO 0 Callback */
    HAL_FDCAN_RxFifo0Callback(hfdcan, RxFifo0ITs);
  }

  /* Rx FIFO 1 interrupts management ******************************************/
  if(RxFifo1ITs != 0U)
  {
    /* Disable the Rx FIFO 1 interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, RxFifo1ITs);

    /* Clear the Rx FIFO 1 flags */
    __HAL_FDCAN_CLEAR_FLAG(hfdcan, RxFifo1ITs);

    /* Rx FIFO 1 Callback */
    HAL_FDCAN_RxFifo1Callback(hfdcan, RxFifo1ITs);
  }

  /* Tx FIFO empty interrupt management ***************************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_TX_FIFO_EMPTY) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_TX_FIFO_EMPTY) != RESET)
    {
      /* Disable the Tx FIFO empty interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_TX_FIFO_EMPTY);

      /* Clear the Tx FIFO empty flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TX_FIFO_EMPTY);

      /* Tx FIFO empty Callback */
      HAL_FDCAN_TxFifoEmptyCallback(hfdcan);
    }
  }

  /* Transmission Complete interrupt management *******************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_TX_COMPLETE) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_TX_COMPLETE) != RESET)
    {
      /* Disable the Transmission Complete interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_TX_COMPLETE);

      /* List of transmitted monitored buffers */
      TransmittedBuffers = hfdcan->Instance->TXBTO;
      TransmittedBuffers &= hfdcan->Instance->TXBTIE;

      /* Disable the Tx Buffer Transmission Interrupt */
      CLEAR_BIT(hfdcan->Instance->TXBTIE, TransmittedBuffers);

      /* Clear the Transmission Complete flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TX_COMPLETE);

      /* Transmission Complete Callback */
      HAL_FDCAN_TxBufferCompleteCallback(hfdcan, TransmittedBuffers);
    }
  }

  /* Rx Buffer New Message interrupt management *******************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_RX_BUFFER_NEW_MESSAGE) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_RX_BUFFER_NEW_MESSAGE) != RESET)
    {
      /* Disable the Rx Buffer New Message interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_RX_BUFFER_NEW_MESSAGE);

      /* Clear the Rx Buffer New Message flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RX_BUFFER_NEW_MESSAGE);

      /* Rx Buffer New Message Callback */
      HAL_FDCAN_RxBufferNewMessageCallback(hfdcan);
    }
  }

  /* Timestamp Wraparound interrupt management ********************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_TIMESTAMP_WRAPAROUND) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_TIMESTAMP_WRAPAROUND) != RESET)
    {
      /* Disable the Timestamp Wraparound interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_TIMESTAMP_WRAPAROUND);

      /* Clear the Timestamp Wraparound flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TIMESTAMP_WRAPAROUND);

      /* Timestamp Wraparound Callback */
      HAL_FDCAN_TimestampWraparoundCallback(hfdcan);
    }
  }

  /* Timeout Occurred interrupt management ************************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_TIMEOUT_OCCURRED) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_TIMEOUT_OCCURRED) != RESET)
    {
      /* Disable the Timeout Occurred interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_TIMEOUT_OCCURRED);

      /* Clear the Timeout Occurred flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TIMEOUT_OCCURRED);

      /* Timeout Occurred Callback */
      HAL_FDCAN_TimeoutOccurredCallback(hfdcan);
    }
  }

  /* Message RAM access failure interrupt management **************************/
  if(__HAL_FDCAN_GET_FLAG(hfdcan, FDCAN_FLAG_RAM_ACCESS_FAILURE) != RESET)
  {
    if(__HAL_FDCAN_GET_IT_SOURCE(hfdcan, FDCAN_IT_RAM_ACCESS_FAILURE) != RESET)
    {
      /* Disable the Timeout Occurred interrupt */
      __HAL_FDCAN_DISABLE_IT(hfdcan, FDCAN_IT_RAM_ACCESS_FAILURE);

      /* Clear the Timeout Occurred flag */
      __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RAM_ACCESS_FAILURE);

      /* Update error code */
      hfdcan->ErrorCode |= HAL_FDCAN_ERROR_RAM_ACCESS;
    }
  }

  /* Error interrupts management **********************************************/
  if(ErrStatus != 0U)
  {
    /* Disable the Error interrupts */
    __HAL_FDCAN_DISABLE_IT(hfdcan, ErrStatus);

    /* Clear the Error flags */
    __HAL_FDCAN_CLEAR_FLAG(hfdcan, ErrStatus);

    /* Update error code */
    hfdcan->ErrorCode |= ErrStatus;
  }

  if((hfdcan->Instance == FDCAN1) && \
     ((hfdcan->ttcan->TTOCF & FDCAN_TTOCF_OM) != 0))
  {
    TTSchedSyncITs = hfdcan->ttcan->TTIR & FDCAN_TT_SCHEDULE_SYNC_MASK;
    TTSchedSyncITs &= hfdcan->ttcan->TTIE;
    TTTimeMarkITs = hfdcan->ttcan->TTIR & FDCAN_TT_TIME_MARK_MASK;
    TTTimeMarkITs &= hfdcan->ttcan->TTIE;
    TTGlobTimeITs = hfdcan->ttcan->TTIR & FDCAN_TT_GLOBAL_TIME_MASK;
    TTGlobTimeITs &= hfdcan->ttcan->TTIE;
    TTDistErrors = hfdcan->ttcan->TTIR & FDCAN_TT_DISTURBING_ERROR_MASK;
    TTDistErrors &= hfdcan->ttcan->TTIE;
    TTFatalErrors = hfdcan->ttcan->TTIR & FDCAN_TT_FATAL_ERROR_MASK;
    TTFatalErrors &= hfdcan->ttcan->TTIE;

    /* TT Schedule Synchronization interrupts management **********************/
    if(TTSchedSyncITs != 0U)
    {
      /* Disable the TT Schedule Synchronization interrupts */
      __HAL_FDCAN_TT_DISABLE_IT(hfdcan, TTSchedSyncITs);

      /* Clear the TT Schedule Synchronization flags */
      __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, TTSchedSyncITs);

      /* TT Schedule Synchronization Callback */
      HAL_FDCAN_TT_ScheduleSyncCallback(hfdcan, TTSchedSyncITs);
    }

    /* TT Time Mark interrupts management *************************************/
    if(TTTimeMarkITs != 0U)
    {
      /* Disable the TT Time Mark interrupts */
      __HAL_FDCAN_TT_DISABLE_IT(hfdcan, TTTimeMarkITs);

      /* Clear the TT Time Mark flags */
      __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, TTTimeMarkITs);

      /* TT Time Mark Callback */
      HAL_FDCAN_TT_TimeMarkCallback(hfdcan, TTTimeMarkITs);
    }

    /* TT Stop Watch interrupt management *************************************/
    if(__HAL_FDCAN_TT_GET_FLAG(hfdcan, FDCAN_TT_FLAG_STOP_WATCH) != RESET)
    {
      if(__HAL_FDCAN_TT_GET_IT_SOURCE(hfdcan, FDCAN_TT_IT_STOP_WATCH) != RESET)
      {
        /* Disable the TT Stop Watch interrupt */
        __HAL_FDCAN_TT_DISABLE_IT(hfdcan, FDCAN_TT_IT_STOP_WATCH);

        /* Retrieve Stop watch Time and Cycle count */
        SWTime = ((hfdcan->ttcan->TTCPT & FDCAN_TTCPT_SWV) >> 16);
        SWCycleCount = hfdcan->ttcan->TTCPT & FDCAN_TTCPT_CCV;

        /* Clear the TT Stop Watch flag */
        __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, FDCAN_TT_FLAG_STOP_WATCH);

        /* TT Stop Watch Callback */
        HAL_FDCAN_TT_StopWatchCallback(hfdcan, SWTime, SWCycleCount);
      }
    }

    /* TT Global Time interrupts management ***********************************/
    if(TTGlobTimeITs != 0U)
    {
      /* Disable the TT Global Time interrupts */
      __HAL_FDCAN_TT_DISABLE_IT(hfdcan, TTGlobTimeITs);

      /* Clear the TT Global Time flags */
      __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, TTGlobTimeITs);

      /* TT Global Time Callback */
      HAL_FDCAN_TT_GlobalTimeCallback(hfdcan, TTGlobTimeITs);
    }

    /* TT Disturbing Error interrupts management ******************************/
    if(TTDistErrors != 0U)
    {
      /* Disable the TT Disturbing Error interrupts */
      __HAL_FDCAN_TT_DISABLE_IT(hfdcan, TTDistErrors);

      /* Clear the TT Disturbing Error flags */
      __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, TTDistErrors);

      /* Update error code */
      hfdcan->ErrorCode |= TTDistErrors;
    }

    /* TT Fatal Error interrupts management ***********************************/
    if(TTFatalErrors != 0U)
    {
      /* Disable the TT Fatal Error interrupts */
      __HAL_FDCAN_TT_DISABLE_IT(hfdcan, TTFatalErrors);

      /* Clear the TT Fatal Error flags */
      __HAL_FDCAN_TT_CLEAR_FLAG(hfdcan, TTFatalErrors);

      /* Update error code */
      hfdcan->ErrorCode |= TTFatalErrors;
    }
  }

  if(hfdcan->ErrorCode != HAL_FDCAN_ERROR_NONE)
  {
    /* Error Callback */
    HAL_FDCAN_ErrorCallback(hfdcan);
  }
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group6 Callback functions
 *  @brief   FDCAN Callback functions
 *
@verbatim
  ==============================================================================
                          ##### Callback functions #####
  ==============================================================================
    [..]
    This subsection provides the following callback functions:
      (+) HAL_FDCAN_ClockCalibrationCallback
      (+) HAL_FDCAN_TxEventFifoCallback
      (+) HAL_FDCAN_RxFifo0Callback
      (+) HAL_FDCAN_RxFifo1Callback
      (+) HAL_FDCAN_TxFifoEmptyCallback
      (+) HAL_FDCAN_TxBufferCompleteCallback
      (+) HAL_FDCAN_TxBufferAbortCallback
      (+) HAL_FDCAN_RxBufferNewMessageCallback
      (+) HAL_FDCAN_HighPriorityMessageCallback
      (+) HAL_FDCAN_TimestampWraparoundCallback
      (+) HAL_FDCAN_TimeoutOccurredCallback
      (+) HAL_FDCAN_ErrorCallback
      (+) HAL_FDCAN_TTSchedSyncCallback
      (+) HAL_FDCAN_TTTimeMarkCallback
      (+) HAL_FDCAN_TTStopWatchCallback
      (+) HAL_FDCAN_TTGlobalTimeCallback

@endverbatim
  * @{
  */

/**
  * @brief  Clock Calibration callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ClkCalibrationITs: indicates which Clock Calibration interrupts are signalled.
  *                            This parameter can be any combination of @arg FDCAN_Clock_Calibration_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_ClockCalibrationCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ClkCalibrationITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(ClkCalibrationITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_ClockCalibrationCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Event callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TxEventFifoITs: indicates which Tx Event FIFO interrupts are signalled.
  *                         This parameter can be any combination of @arg FDCAN_Tx_Event_Fifo_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(TxEventFifoITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TxEventFifoCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx FIFO 0 callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxFifo0ITs: indicates which Rx FIFO 0 interrupts are signalled.
  *                     This parameter can be any combination of @arg FDCAN_Rx_Fifo0_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(RxFifo0ITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_RxFifo0Callback could be implemented in the user file
   */
}

/**
  * @brief  Rx FIFO 1 callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxFifo1ITs: indicates which Rx FIFO 1 interrupts are signalled.
  *                     This parameter can be any combination of @arg FDCAN_Rx_Fifo1_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(RxFifo1ITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_RxFifo1Callback could be implemented in the user file
   */
}

/**
  * @brief  Tx FIFO Empty callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TxFifoEmptyCallback could be implemented in the user file
   */
}

/**
  * @brief  Transmission Complete callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  BufferIndexes: Indexes of the transmitted buffers.
  *                        This parameter can be any combination of @arg FDCAN_Tx_location.
  * @retval None
  */
__weak void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(BufferIndexes);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TxBufferCompleteCallback could be implemented in the user file
   */
}

/**
  * @brief  Transmission Cancellation callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  BufferIndexes: Indexes of the aborted buffers.
  *                        This parameter can be any combination of @arg FDCAN_Tx_location.
  * @retval None
  */
__weak void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(BufferIndexes);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TxBufferAbortCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Buffer New Message callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_RxBufferNewMessageCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_RxBufferNewMessageCallback could be implemented in the user file
   */
}

/**
  * @brief  Timestamp Wraparound callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_TimestampWraparoundCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TimestampWraparoundCallback could be implemented in the user file
   */
}

/**
  * @brief  Timeout Occurred callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_TimeoutOccurredCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TimeoutOccurredCallback could be implemented in the user file
   */
}

/**
  * @brief  High Priority Message callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_HighPriorityMessageCallback could be implemented in the user file
   */
}

/**
  * @brief  Error callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval None
  */
__weak void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_ErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  TT Schedule Synchronization callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TTSchedSyncITs: indicates which TT Schedule Synchronization interrupts are signalled.
  *                         This parameter can be any combination of @arg FDCAN_TTScheduleSynchronization_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_TT_ScheduleSyncCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TTSchedSyncITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(TTSchedSyncITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TTSchedSyncCallback could be implemented in the user file
   */
}

/**
  * @brief  TT Time Mark callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TTTimeMarkITs: indicates which TT Schedule Synchronization interrupts are signalled.
  *                        This parameter can be any combination of @arg FDCAN_TTTimeMark_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_TT_TimeMarkCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TTTimeMarkITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(TTTimeMarkITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TTTimeMarkCallback could be implemented in the user file
   */
}

/**
  * @brief  TT Stop Watch callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  SWTime: Time Value captured at the Stop Watch Trigger pin (fdcan1_swt) falling/rising
  *                 edge (as configured via HAL_FDCAN_TTConfigStopWatch).
  *                 This parameter is a number between 0 and 0xFFFF.
  * @param  SWCycleCount: Cycle count value captured together with SWTime.
  *                       This parameter is a number between 0 and 0x3F.
  * @retval None
  */
__weak void HAL_FDCAN_TT_StopWatchCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t SWTime, uint32_t SWCycleCount)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(SWTime);
  UNUSED(SWCycleCount);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TTStopWatchCallback could be implemented in the user file
   */
}

/**
  * @brief  TT Global Time callback.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  TTGlobTimeITs: indicates which TT Global Time interrupts are signalled.
  *                        This parameter can be any combination of @arg FDCAN_TTGlobalTime_Interrupts.
  * @retval None
  */
__weak void HAL_FDCAN_TT_GlobalTimeCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TTGlobTimeITs)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hfdcan);
  UNUSED(TTGlobTimeITs);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_FDCAN_TTGlobalTimeCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup FDCAN_Exported_Functions_Group7 Peripheral State functions
 *  @brief   FDCAN Peripheral State functions
 *
@verbatim
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================
    [..]
    This subsection provides functions allowing to :
      (+) HAL_FDCAN_GetState()  : Return the FDCAN state.
      (+) HAL_FDCAN_GetError()  : Return the FDCAN error code if any.

@endverbatim
  * @{
  */
/**
  * @brief  Return the FDCAN state
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval HAL state
  */
HAL_FDCAN_StateTypeDef HAL_FDCAN_GetState(FDCAN_HandleTypeDef* hfdcan)
{
  /* Return FDCAN state */
  return hfdcan->State;
}

/**
  * @brief  Return the FDCAN error code
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @retval FDCAN Error Code
  */
uint32_t HAL_FDCAN_GetError(FDCAN_HandleTypeDef *hfdcan)
{
  /* Return FDCAN error code */
  return hfdcan->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup FDCAN_Private_Functions
  * @{
  */

/**
  * @brief  Calculate each RAM block start address and size
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *                 the configuration information for the specified FDCAN.
  * @retval HAL status
 */
static HAL_StatusTypeDef FDCAN_CalcultateRamBlockAddresses(FDCAN_HandleTypeDef *hfdcan)
{
  uint32_t RAMcounter;

  hfdcan->msgRam.StandardFilterSA = hfdcan->Init.MessageRAMOffset;

  /* Standard filter list start address */
  MODIFY_REG(hfdcan->Instance->SIDFC, FDCAN_SIDFC_FLSSA, (hfdcan->msgRam.StandardFilterSA << 2));

  /* Standard filter elements number */
  MODIFY_REG(hfdcan->Instance->SIDFC, FDCAN_SIDFC_LSS, (hfdcan->Init.StdFiltersNbr << 16));

  /* Extended filter list start address */
  hfdcan->msgRam.ExtendedFilterSA = hfdcan->msgRam.StandardFilterSA + hfdcan->Init.StdFiltersNbr;
  MODIFY_REG(hfdcan->Instance->XIDFC, FDCAN_XIDFC_FLESA, (hfdcan->msgRam.ExtendedFilterSA << 2));

  /* Extended filter elements number */
  MODIFY_REG(hfdcan->Instance->XIDFC, FDCAN_XIDFC_LSE, (hfdcan->Init.ExtFiltersNbr << 16));

  /* Rx FIFO 0 start address */
  hfdcan->msgRam.RxFIFO0SA = hfdcan->msgRam.ExtendedFilterSA + (hfdcan->Init.ExtFiltersNbr * 2);
  MODIFY_REG(hfdcan->Instance->RXF0C, FDCAN_RXF0C_F0SA, (hfdcan->msgRam.RxFIFO0SA << 2));

  /* Rx FIFO 0 elements number */
  MODIFY_REG(hfdcan->Instance->RXF0C, FDCAN_RXF0C_F0S, (hfdcan->Init.RxFifo0ElmtsNbr << 16));

  /* Rx FIFO 1 start address */
  hfdcan->msgRam.RxFIFO1SA = hfdcan->msgRam.RxFIFO0SA + (hfdcan->Init.RxFifo0ElmtsNbr * hfdcan->Init.RxFifo0ElmtSize);
  MODIFY_REG(hfdcan->Instance->RXF1C, FDCAN_RXF1C_F1SA, (hfdcan->msgRam.RxFIFO1SA << 2));

  /* Rx FIFO 1 elements number */
  MODIFY_REG(hfdcan->Instance->RXF1C, FDCAN_RXF1C_F1S, (hfdcan->Init.RxFifo1ElmtsNbr << 16));

  /* Rx buffer list start address */
  hfdcan->msgRam.RxBufferSA = hfdcan->msgRam.RxFIFO1SA + (hfdcan->Init.RxFifo1ElmtsNbr * hfdcan->Init.RxFifo1ElmtSize);
  MODIFY_REG(hfdcan->Instance->RXBC, FDCAN_RXBC_RBSA, (hfdcan->msgRam.RxBufferSA << 2));

  /* Tx event FIFO start address */
  hfdcan->msgRam.TxEventFIFOSA = hfdcan->msgRam.RxBufferSA + (hfdcan->Init.RxBuffersNbr * hfdcan->Init.RxBufferSize);
  MODIFY_REG(hfdcan->Instance->TXEFC, FDCAN_TXEFC_EFSA, (hfdcan->msgRam.TxEventFIFOSA << 2));

  /* Tx event FIFO elements number */
  MODIFY_REG(hfdcan->Instance->TXEFC, FDCAN_TXEFC_EFS, (hfdcan->Init.TxEventsNbr << 16));

  /* Tx buffer list start address */
  hfdcan->msgRam.TxBufferSA = hfdcan->msgRam.TxEventFIFOSA + (hfdcan->Init.TxEventsNbr * 2);
  MODIFY_REG(hfdcan->Instance->TXBC, FDCAN_TXBC_TBSA, (hfdcan->msgRam.TxBufferSA << 2));

  /* Dedicated Tx buffers number */
  MODIFY_REG(hfdcan->Instance->TXBC, FDCAN_TXBC_NDTB, (hfdcan->Init.TxBuffersNbr << 16));

  /* Tx FIFO/queue start address */
  hfdcan->msgRam.TxFIFOQSA = hfdcan->msgRam.TxBufferSA + (hfdcan->Init.TxBuffersNbr * hfdcan->Init.TxElmtSize);

  /* Tx FIFO/queue elements number */
  MODIFY_REG(hfdcan->Instance->TXBC, FDCAN_TXBC_TFQS, (hfdcan->Init.TxFifoQueueElmtsNbr << 24));

  hfdcan->msgRam.StandardFilterSA = SRAMCAN_BASE + (hfdcan->Init.MessageRAMOffset * 4);
  hfdcan->msgRam.ExtendedFilterSA = hfdcan->msgRam.StandardFilterSA + (hfdcan->Init.StdFiltersNbr * 4);
  hfdcan->msgRam.RxFIFO0SA = hfdcan->msgRam.ExtendedFilterSA + (hfdcan->Init.ExtFiltersNbr * 2 * 4);
  hfdcan->msgRam.RxFIFO1SA = hfdcan->msgRam.RxFIFO0SA + (hfdcan->Init.RxFifo0ElmtsNbr * hfdcan->Init.RxFifo0ElmtSize * 4);
  hfdcan->msgRam.RxBufferSA = hfdcan->msgRam.RxFIFO1SA + (hfdcan->Init.RxFifo1ElmtsNbr * hfdcan->Init.RxFifo1ElmtSize * 4);
  hfdcan->msgRam.TxEventFIFOSA = hfdcan->msgRam.RxBufferSA + (hfdcan->Init.RxBuffersNbr * hfdcan->Init.RxBufferSize * 4);
  hfdcan->msgRam.TxBufferSA = hfdcan->msgRam.TxEventFIFOSA + (hfdcan->Init.TxEventsNbr * 2 * 4);
  hfdcan->msgRam.TxFIFOQSA = hfdcan->msgRam.TxBufferSA + (hfdcan->Init.TxBuffersNbr * hfdcan->Init.TxElmtSize * 4);

  hfdcan->msgRam.EndAddress = hfdcan->msgRam.TxFIFOQSA + (hfdcan->Init.TxFifoQueueElmtsNbr * hfdcan->Init.TxElmtSize * 4);

  if(hfdcan->msgRam.EndAddress > 0x4000B5FC) /* Last address of the Message RAM */
  {
    /* Update error code.
       Message RAM overflow */
    hfdcan->ErrorCode |= HAL_FDCAN_ERROR_PARAM;

    return HAL_ERROR;
  }
  else
  {
    /* Flush the allocated Message RAM area */
    for(RAMcounter = hfdcan->msgRam.StandardFilterSA; RAMcounter < hfdcan->msgRam.EndAddress; RAMcounter += 4)
    {
      *(__IO uint32_t *)(RAMcounter) = 0x00000000;
    }
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Copy Tx message to the message RAM.
  * @param  hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
  *                 the configuration information for the specified FDCAN.
  * @param  pTxHeader: pointer to a FDCAN_TxHeaderTypeDef structure.
  * @param  pTxData: pointer to a buffer containing the payload of the Tx frame.
  * @param  BufferIndex: index of the buffer to be configured.
  * @retval HAL status
 */
static HAL_StatusTypeDef FDCAN_CopyMessageToRAM(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData, uint32_t BufferIndex)
{
  uint32_t TxElementW1;
  uint32_t TxElementW2;
  uint32_t *TxAddress;
  uint32_t ByteCounter;

  /* Build first word of Tx header element */
  if(pTxHeader->IdType == FDCAN_STANDARD_ID)
  {
    TxElementW1 = (pTxHeader->ErrorStateIndicator |
                   FDCAN_STANDARD_ID |
                   pTxHeader->TxFrameType |
                   (pTxHeader->Identifier << 18));
  }
  else /* pTxHeader->IdType == FDCAN_EXTENDED_ID */
  {
    TxElementW1 = (pTxHeader->ErrorStateIndicator |
                   FDCAN_EXTENDED_ID |
                   pTxHeader->TxFrameType |
                   pTxHeader->Identifier);
  }

  /* Build second word of Tx header element */
  TxElementW2 = ((pTxHeader->MessageMarker << 24) |
                 pTxHeader->TxEventFifoControl |
                 pTxHeader->FDFormat |
                 pTxHeader->BitRateSwitch |
                 pTxHeader->DataLength);

  /* Calculate Tx element address */
  TxAddress = (uint32_t *)(hfdcan->msgRam.TxBufferSA + (BufferIndex * hfdcan->Init.TxElmtSize * 4));

  /* Write Tx element header to the message RAM */
  *TxAddress++ = TxElementW1;
  *TxAddress++ = TxElementW2;

  /* Write Tx payload to the message RAM */
  for(ByteCounter = 0; ByteCounter < DLCtoBytes[pTxHeader->DataLength >> 16]; ByteCounter += 4)
  {
    *TxAddress++ = ((pTxData[ByteCounter+3] << 24) |
                    (pTxData[ByteCounter+2] << 16) |
                    (pTxData[ByteCounter+1] << 8) |
                    pTxData[ByteCounter]);
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */
#endif /* HAL_FDCAN_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
