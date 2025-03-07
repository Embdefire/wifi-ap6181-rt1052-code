/**
  ******************************************************************************
  * @file    stm32h7xx_hal_dac.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-August-2017
  * @brief   DAC HAL module driver.
  *         This file provides firmware functions to manage the following 
  *         functionalities of the Digital to Analog Converter (DAC) peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State and Errors functions      
  *     
  *
  @verbatim      
  ==============================================================================
                      ##### DAC Peripheral features #####
  ==============================================================================
    [..]        
      *** DAC Channels ***
      ====================  
    [..]  
    STM32H7 devices integrate two 12-bit Digital Analog Converters.

    The 2 converters (i.e. channel1 & channel2) can be used independently or simultaneously (dual mode):
      (#) DAC channel1 with DAC_OUT1 (PA4) as output or connected to on-chip 
          peripherals (ex. OPAMPs, comparators).
      (#) DAC channel2 with DAC_OUT2 (PA5) as output or connected to on-chip 
          peripherals (ex. OPAMPs, comparators).
      
      *** DAC Triggers ***
      ====================
    [..]
    Digital to Analog conversion can be non-triggered using DAC_TRIGGER_NONE
    and DAC_OUT1/DAC_OUT2 is available once writing to DHRx register. 
    [..] 
    Digital to Analog conversion can be triggered by:
      (#) External event: EXTI Line 9 (any GPIOx_PIN_9) using DAC_TRIGGER_EXT_IT9.
          The used pin (GPIOx_PIN_9) must be configured in input mode.
  
      
      (#) Timers TRGO:TIM1,TIM2,TIM4, TIM5, TIM6, TIM7,TIM8 and TIM15 
          (DAC_TRIGGER_T1_TRGO, DAC_TRIGGER_T2_TRGO...)

      (#) Timers TRGO: HRTIM1,LPTIM1,LPTIM2
      (DAC_TRIGGER_HR1_TRGO1,DAC_TRIGGER_HR1_TRGO2,DAC_TRIGGER_LP1_OUT,DAC_TRIGGER_LP2_OUT)
      (#) Software using DAC_TRIGGER_SOFTWARE
  
      *** DAC Buffer mode feature ***
      =============================== 
      [..] 
      Each DAC channel integrates an output buffer that can be used to 
      reduce the output impedance, and to drive external loads directly
      without having to add an external operational amplifier.
      To enable, the output buffer use  
      sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
      [..]           
      (@) Refer to the device datasheet for more details about output 
          impedance value with and without output buffer.

      *** DAC connect feature ***
      =============================== 
      [..] 
      Each DAC channel can be connected internally. 
      To connect, use
      sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
                                               
      *** GPIO configurations guidelines ***
      ====================================
      [..] 
      When a DAC channel is used (ex channel1 on PA4) and the other is not 
      (ex channel2 on PA5 is configured in Analog and disabled).
      Channel1 may disturb channel2 as coupling effect.
      Note that there is no coupling on channel2 as soon as channel2 is turned on.
      Coupling on adjacent channel could be avoided as follows:
      when unused PA5 is configured as INPUT PULL-UP or DOWN. 
      PA5 is configured in ANALOG just before it is turned on.     
                                               
      *** DAC Sample and Hold feature ***
      ===================================
      [..] 
      For each converter, 2 modes are supported: normal mode and 
      "sample and hold" mode (i.e. low power mode).
      In the sample and hold mode, the DAC core converts data, then holds the 
      converted voltage on a capacitor. When not converting, the DAC cores and 
      buffer are completely turned off between samples and the DAC output is 
      tri-stated, therefore  reducing the overall power consumption. A new 
      stabilization period is needed before each new conversion.
      [..]
      The sample and hold allow setting internal or external voltage @
      low power consumption cost (output value can be at any given rate either 
      by CPU or DMA).
      [..]
      The Sample and hold block and registers uses either LSI & run in 
      several power modes: run mode, sleep mode & stop mode.
        
      To enable Sample and Hold mode ,enable LSI using HAL_RCC_OscConfig with 
    RCC_OSCILLATORTYPE_LSI & RCC_LSI_ON parameters.

      Use DAC_InitStructure.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_ENABLE
         & DAC_ChannelConfTypeDef.DAC_SampleAndHoldConfig.DAC_SampleTime,
           DAC_HoldTime & DAC_RefreshTime.
    

       *** DAC calibration feature ***
       =================================== 
      [..] 
       (#)  The 2 converters (channel1 & channel2) provide calibration capabilities.  
       (++) Calibration aims at correcting some offset of output buffer.
       (++) The DAC uses either factory calibration settings OR user defined 
           calibration (trimming) settings (i.e. trimming mode).
       (++) The user defined settings can be figured out using self calibration 
           handled by HAL_DACEx_SelfCalibrate.
       (++) HAL_DACEx_SelfCalibrate:
       (+++) Runs automatically the calibration.
       (+++) Enables the user trimming mode
       (+++) Updates a structure with trimming values with fresh calibration 
            results. 
            The user may store the calibration results for larger 
            (ex monitoring the trimming as a function of temperature 
            for instance)

       *** DAC wave generation feature ***
       =================================== 
       [..]     
       Both DAC channels can be used to generate:
         (#) Noise wave 
         (#) Triangle wave
            
       *** DAC data format ***
       =======================
       [..]   
       The DAC data format can be:
         (#) 8-bit right alignment using DAC_ALIGN_8B_R
         (#) 12-bit left alignment using DAC_ALIGN_12B_L
         (#) 12-bit right alignment using DAC_ALIGN_12B_R
  
       *** DAC data value to voltage correspondence ***  
       ================================================ 
       [..] 
       The analog output voltage on each DAC channel pin is determined
       by the following equation: 
       [..]
       DAC_OUTx = VREF+ * DOR / 4095
       (+) with  DOR is the Data Output Register
       [..]
          VREF+ is the input voltage reference (refer to the device datasheet)
       [..]
        e.g. To set DAC_OUT1 to 0.7V:
       (+) Assuming that VREF+ = 3.3V, DAC_OUT1 = (3.3 * 868) / 4095 = 0.7V
  
       *** DMA requests  ***
       =====================
       [..]    
       A DMA request can be generated when an external trigger (but not
       a software trigger) occurs if DMA requests are enabled using
       HAL_DAC_Start_DMA().
       DMA requests are mapped as following:      
      (#) DAC channel1: mapped on DMA_REQUEST_DAC1  
      (#) DAC channel2: mapped on DMA_REQUEST_DAC2
      [..]  
    -@- For Dual mode and specific signal (Triangle and noise) generation please 
        refer to Extended Features Driver description        
      
                      ##### How to use this driver #####
  ==============================================================================
    [..]          
      (+) DAC APB clock must be enabled to get write access to DAC
          registers using HAL_DAC_Init()
      (+) Configure DAC_OUTx (DAC_OUT1: PA4, DAC_OUT2: PA5) in analog mode.
      (+) Configure the DAC channel using HAL_DAC_ConfigChannel() function.
      (+) Enable the DAC channel using HAL_DAC_Start() or HAL_DAC_Start_DMA() functions.

     *** Calibration mode IO operation ***
     ======================================
     [..]    
       (+) Retrieve the factory trimming (calibration settings) using HAL_DACEx_GetTrimOffset()
       (+) Run the calibration using HAL_DACEx_SelfCalibrate() 
       (+) Update the trimming while DAC running using HAL_DACEx_SetUserTrimming()

     *** Polling mode IO operation ***
     =================================
     [..]    
       (+) Start the DAC peripheral using HAL_DAC_Start() 
       (+) To read the DAC last data output value, use the HAL_DAC_GetValue() function.
       (+) Stop the DAC peripheral using HAL_DAC_Stop()
       
     *** DMA mode IO operation ***    
     ==============================
     [..]    
       (+) Start the DAC peripheral using HAL_DAC_Start_DMA(), at this stage the user specify the length 
           of data to be transferred at each end of conversion.
       (+) At the middle of data transfer HAL_DAC_ConvHalfCpltCallbackCh1()or HAL_DACEx_ConvHalfCpltCallbackCh2()  
           function is executed and user can add his own code by customization of function pointer 
           HAL_DAC_ConvHalfCpltCallbackCh1() or HAL_DACEx_ConvHalfCpltCallbackCh2().
       (+) At The end of data transfer HAL_DAC_ConvCpltCallbackCh1()or HAL_DACEx_ConvHalfCpltCallbackCh2()  
           function is executed and user can add his own code by customization of function pointer 
           HAL_DAC_ConvCpltCallbackCh1() or HAL_DACEx_ConvHalfCpltCallbackCh2().
       (+) In case of transfer Error, HAL_DAC_ErrorCallbackCh1() function is executed and user can 
            add his own code by customization of function pointer HAL_DAC_ErrorCallbackCh1.
       (+) In case of DMA underrun, DAC interruption triggers and execute internal function HAL_DAC_IRQHandler.
           HAL_DAC_DMAUnderrunCallbackCh1()or HAL_DACEx_DMAUnderrunCallbackCh2()  
           function is executed and user can add his own code by customization of function pointer 
           HAL_DAC_DMAUnderrunCallbackCh1() or HAL_DACEx_DMAUnderrunCallbackCh2()and
           add his own code by customization of function pointer HAL_DAC_ErrorCallbackCh1().
       (+) Stop the DAC peripheral using HAL_DAC_Stop_DMA()
                    
     *** DAC HAL driver macros list ***
     ============================================= 
     [..]
       Below the list of most used macros in DAC HAL driver.
       
      (+) __HAL_DAC_ENABLE : Enable the DAC peripheral
      (+) __HAL_DAC_DISABLE : Disable the DAC peripheral
      (+) __HAL_DAC_CLEAR_FLAG: Clear the DAC's pending flags
      (+) __HAL_DAC_GET_FLAG: Get the selected DAC's flag status
      
     [..]
      (@) You can refer to the DAC HAL driver header file for more useful macros  
   
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

/** @defgroup DAC DAC 
  * @brief DAC driver modules
  * @{
  */ 

#ifdef HAL_DAC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup DAC_Private_Constants DAC Private Constants
  * @{
  */
#define TIMEOUT_DAC_CALIBCONFIG    ((uint32_t)1)  /* 1ms                  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @defgroup DAC_Private_Functions DAC Private Functions
  * @{
  */
static void DAC_DMAConvCpltCh1(DMA_HandleTypeDef *hdma);
static void DAC_DMAErrorCh1(DMA_HandleTypeDef *hdma);
static void DAC_DMAHalfConvCpltCh1(DMA_HandleTypeDef *hdma); 

/**
  * @}
  */
/* Exported functions ---------------------------------------------------------*/

/** @defgroup DAC_Exported_Functions DAC Exported Functions
  * @{
  */

/** @defgroup DAC_Exported_Functions_Group1 Initialization and de-initialization functions 
 *  @brief    Initialization and Configuration functions 
 *
@verbatim    
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize and configure the DAC. 
      (+) De-initialize the DAC. 
         
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the DAC peripheral according to the specified parameters
  *         in the DAC_InitStruct and initialize the associated handle.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Init(DAC_HandleTypeDef* hdac)
{ 
  /* Check DAC handle */
  if(hdac == NULL)
  {
     return HAL_ERROR;
  }
  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));
  
  if(hdac->State == HAL_DAC_STATE_RESET)
  {  
  /* Allocate lock resource and initialize it */
    hdac->Lock = HAL_UNLOCKED;
    /* Init the low level hardware */
    HAL_DAC_MspInit(hdac);
  }
  
  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_BUSY;
  
  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;
  
  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Deinitialize the DAC peripheral registers to their default reset values.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_DeInit(DAC_HandleTypeDef* hdac)
{
  /* Check DAC handle */
  if(hdac == NULL)
  {
     return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  /* DeInit the low level hardware */
  HAL_DAC_MspDeInit(hdac);

  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the DAC MSP.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{ 
   /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);
    /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspInit could be implemented in the user file
   */ 
}

/**
  * @brief  DeInitialize the DAC MSP.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.  
  * @retval None
  */
__weak void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);
    /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspDeInit could be implemented in the user file
   */ 
}

/**
  * @}
  */

/** @defgroup DAC_Exported_Functions_Group2 IO operation functions
 *  @brief    IO operation functions 
 *
@verbatim   
  ==============================================================================
             ##### IO operation functions #####
  ==============================================================================  
    [..]  This section provides functions allowing to:
      (+) Start conversion.
      (+) Stop conversion.
      (+) Start conversion and enable DMA transfer.
      (+) Stop conversion and disable DMA transfer.
      (+) Set the specified data holding register value for DAC channel.
      
                     
@endverbatim
  * @{
  */

/**
  * @brief  Enable DAC and start conversion of channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef* hdac, uint32_t Channel)
{

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  
  /* Process locked */
  __HAL_LOCK(hdac);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;
  
  /* Enable the Peripheral */
  __HAL_DAC_ENABLE(hdac, Channel);
  
  if(Channel == DAC_CHANNEL_1)
  {
    
    /* Check if software trigger enabled */
    if((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_CR_TEN1)
    {
      /* Enable the selected DAC software conversion */
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
    }
  }
  else
  {  
    /* Check if software trigger enabled */
    if((hdac->Instance->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == DAC_CR_TEN2)
    {
      /* Enable the selected DAC software conversion*/
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG2);
    }
  }
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Process unlocked */
  __HAL_UNLOCK(hdac);
    
  /* Return function status */
  return HAL_OK;
}
  
/**
  * @brief  Disable DAC and stop conversion of channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop(DAC_HandleTypeDef* hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  
  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Enable DAC and start conversion of channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  pData: The destination peripheral Buffer address.
  * @param  Length: The length of data to be transferred from memory to DAC peripheral
  * @param  Alignment: Specifies the data alignment for DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start_DMA(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t* pData, uint32_t Length, uint32_t Alignment)
{
  uint32_t tmpreg = 0;
    
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));
  
  /* Process locked */
  __HAL_LOCK(hdac);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;
  
  if(Channel == DAC_CHANNEL_1)
  {
    /* Set the DMA transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferCpltCallback = DAC_DMAConvCpltCh1;
    
    /* Set the DMA half transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh1;
       
    /* Set the DMA error callback for channel1 */
    hdac->DMA_Handle1->XferErrorCallback = DAC_DMAErrorCh1;
    
    /* Enable the selected DAC channel1 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);
    
    /* Case of use of channel 1 */
    switch(Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R1;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L1;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R1;
        break;
      default:
        break;
    }
  }
  else
  {
    /* Set the DMA transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferCpltCallback = DAC_DMAConvCpltCh2;
    
    /* Set the DMA half transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh2;
       
    /* Set the DMA error callback for channel2 */
    hdac->DMA_Handle2->XferErrorCallback = DAC_DMAErrorCh2;
    
    /* Enable the selected DAC channel2 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);
    
    /* Case of use of channel 2 */
    switch(Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R2;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L2;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R2;
        break;
      default:
        break;
    }
  }
  
  /* Enable the DMA Stream */
  if(Channel == DAC_CHANNEL_1)
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR1);
    
    /* Enable the DMA Stream */
    HAL_DMA_Start_IT(hdac->DMA_Handle1, (uint32_t)pData, tmpreg, Length);
  } 
  else
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR2);
    
    /* Enable the DMA Stream */
    HAL_DMA_Start_IT(hdac->DMA_Handle2, (uint32_t)pData, tmpreg, Length);
  }

  /* Process Unlocked */
  __HAL_UNLOCK(hdac);
  /* Enable the Peripheral */
  __HAL_DAC_ENABLE(hdac, Channel);
  
  
  
  /* Return function status */
  return HAL_OK;
}
 
/**
  * @brief  Disable DAC and stop conversion of channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected   
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop_DMA(DAC_HandleTypeDef* hdac, uint32_t Channel)
{
   HAL_StatusTypeDef status = HAL_OK;
    
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  
  /* Disable the selected DAC channel DMA request */
    hdac->Instance->CR &= ~(DAC_CR_DMAEN1 << Channel);
    
  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);
  
  /* Disable the DMA stream */
  /* Channel1 is used */
  if (Channel == DAC_CHANNEL_1)
  {
    /* Disable the DMA stream */
    status = HAL_DMA_Abort(hdac->DMA_Handle1); 
    /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR1);
  }
  else /* Channel2 is used for */
  {
    /* Disable the DMA stream */
    status = HAL_DMA_Abort(hdac->DMA_Handle2);   
  /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR2);
  }
  
  /* Check if DMA Channel effectively disabled */
  if (status != HAL_OK)
  {
    /* Update DAC state machine to error */
    hdac->State = HAL_DAC_STATE_ERROR;      
  }
  else
  {
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_READY;
  }
  
  /* Return function status */
  return status;
}

/**
  * @brief  Handle DAC interrupt request
  *         This function uses the interruption of DMA
  *         underrun.  
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
void HAL_DAC_IRQHandler(DAC_HandleTypeDef* hdac)
{
  if(__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR1))
  { 
    /* Check underrun flag of DAC channel 1 */
    if(__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR1))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;
      
      /* Set DAC error code to chanel1 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH1);
      
      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac,DAC_FLAG_DMAUDR1);
      
      /* Disable the selected DAC channel1 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);
      
      /* Error callback */ 
      HAL_DAC_DMAUnderrunCallbackCh1(hdac);
    }
  }
  if(__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR2))
  {
    /* Check underrun flag of DAC channel 1 */
    if(__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR2))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;
      
      /* Set DAC error code to channel2 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH2);
      
      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac,DAC_FLAG_DMAUDR2);
      
      /* Disable the selected DAC channel1 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);
      
      /* Error callback */ 
      HAL_DACEx_DMAUnderrunCallbackCh2(hdac);
    }
  }
}

/**
  * @brief  Set the specified data holding register value for DAC channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected  
  * @param  Alignment: Specifies the data alignment.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @param  Data: Data to be loaded in the selected data holding register.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data)
{  
  __IO uint32_t tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));
  assert_param(IS_DAC_DATA(Data));
  
  tmp = (uint32_t)hdac->Instance; 
  if(Channel == DAC_CHANNEL_1)
  {
    tmp += DAC_DHR12R1_ALIGNMENT(Alignment);
  }
  else
  {
    tmp += DAC_DHR12R2_ALIGNMENT(Alignment);
  }

  /* Set the DAC channel selected data holding register */
  *(__IO uint32_t *) tmp = Data;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Conversion complete callback in non-blocking mode for Channel1 
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Conversion half DMA transfer callback in non-blocking mode for Channel1 
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvHalfCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Error DAC callback for Channel1.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ErrorCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  DMA underrun DAC callback for channel1.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_DMAUnderrunCallbackCh1 could be implemented in the user file
   */
}

/**
  * @}
  */
  
/** @defgroup DAC_Exported_Functions_Group3 Peripheral Control functions
 *  @brief    Peripheral Control functions 
 *
@verbatim   
  ==============================================================================
             ##### Peripheral Control functions #####
  ==============================================================================  
    [..]  This section provides functions allowing to:
      (+) Configure channels. 
      (+) Get result of conversion.
      
@endverbatim
  * @{
  */


/**
  * @brief  Return the last data output value of the selected DAC channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval The selected DAC channel data output value.
  */
uint32_t HAL_DAC_GetValue(DAC_HandleTypeDef* hdac, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  
  /* Returns the DAC channel data output register value */
  if(Channel == DAC_CHANNEL_1)
  {
    return hdac->Instance->DOR1;
  }
  else
  {
    return hdac->Instance->DOR2;
  }
}
/**
  * @brief  Configure the selected DAC channel.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig: DAC configuration structure.
  * @param  Channel: The selected DAC channel. 
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef* hdac, DAC_ChannelConfTypeDef* sConfig, uint32_t Channel)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0;
  uint32_t tickstart = 0;
   
  /* Check the DAC parameters */
  assert_param(IS_DAC_TRIGGER(sConfig->DAC_Trigger));
  assert_param(IS_DAC_OUTPUT_BUFFER_STATE(sConfig->DAC_OutputBuffer));
  assert_param(IS_DAC_CHIP_CONNECTION(sConfig->DAC_ConnectOnChipPeripheral));
  assert_param(IS_DAC_TRIMMING(sConfig->DAC_UserTrimming));
  if ((sConfig->DAC_UserTrimming) == DAC_TRIMMING_USER)
  {
    assert_param(IS_DAC_TRIMMINGVALUE(sConfig->DAC_TrimmingValue));               
  }
  assert_param(IS_DAC_SAMPLEANDHOLD(sConfig->DAC_SampleAndHold));
  if ((sConfig->DAC_SampleAndHold) == DAC_SAMPLEANDHOLD_ENABLE)
  {
    assert_param(IS_DAC_SAMPLETIME(sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime));
    assert_param(IS_DAC_HOLDTIME(sConfig->DAC_SampleAndHoldConfig.DAC_HoldTime));
    assert_param(IS_DAC_REFRESHTIME(sConfig->DAC_SampleAndHoldConfig.DAC_RefreshTime)); 
  }
  assert_param(IS_DAC_CHANNEL(Channel));
 
  /* Process locked */
  __HAL_LOCK(hdac);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;
  
  if(sConfig->DAC_SampleAndHold == DAC_SAMPLEANDHOLD_ENABLE)
  /* Sample on old configuration */ 
  {
    /* SampleTime */
    if (Channel == DAC_CHANNEL_1)
    {
      /* Get timeout */
      tickstart = HAL_GetTick();
      
      
      /* SHSR1 can be written when BWST1  equals RESET */
      while (((hdac->Instance->SR) & DAC_SR_BWST1)!= RESET)
      {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > TIMEOUT_DAC_CALIBCONFIG)
        {
          /* Update error code */
          SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_TIMEOUT);

          /* Change the DMA state */
          hdac->State = HAL_DAC_STATE_TIMEOUT;

          return HAL_TIMEOUT;
        }
      }
      HAL_Delay(1);
      hdac->Instance->SHSR1 = sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime;
    }
    else /* Channel 2 */
    {
      /* SHSR2 can be written when BWST2 equals RESET */

      while (((hdac->Instance->SR) & DAC_SR_BWST2)!= RESET)
      {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > TIMEOUT_DAC_CALIBCONFIG)
        {
          /* Update error code */
          SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_TIMEOUT);

          /* Change the DMA state */
          hdac->State = HAL_DAC_STATE_TIMEOUT;

          return HAL_TIMEOUT;
        }
      }
      HAL_Delay(1);
      hdac->Instance->SHSR2 = sConfig->DAC_SampleAndHoldConfig.DAC_SampleTime;
    }
    /* HoldTime */
    hdac->Instance->SHHR = (sConfig->DAC_SampleAndHoldConfig.DAC_HoldTime)<<Channel;
    /* RefreshTime */
    hdac->Instance->SHRR = (sConfig->DAC_SampleAndHoldConfig.DAC_RefreshTime)<<Channel;
  }
    
  if(sConfig->DAC_UserTrimming == DAC_TRIMMING_USER)
  /* USER TRIMMING */
  {
  /* Get the DAC CCR value */
  tmpreg1 = hdac->Instance->CCR;
  /* Clear trimming value */
  tmpreg1 &= ~(((uint32_t)(DAC_CCR_OTRIM1)) << Channel);
  /* Configure for the selected trimming offset */
  tmpreg2 = sConfig->DAC_TrimmingValue;
  /* Calculate CCR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << Channel;
  /* Write to DAC CCR */
  hdac->Instance->CCR = tmpreg1;
  }
  /* else factory trimming is used (factory setting are available at reset)*/
  /* SW Nothing has nothing to do */
  
  /* Get the DAC MCR value */
  tmpreg1 = hdac->Instance->MCR;
  /* Clear DAC_MCR_MODE2_0, DAC_MCR_MODE2_1 and DAC_MCR_MODE2_2 bits */
  tmpreg1 &= ~(((uint32_t)(DAC_MCR_MODE1)) << Channel); 
  /* Configure for the selected DAC channel: mode, buffer output & on chip peripheral connect */
  tmpreg2 = (sConfig->DAC_SampleAndHold | sConfig->DAC_OutputBuffer | sConfig->DAC_ConnectOnChipPeripheral);
  /* Calculate MCR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << Channel;
  /* Write to DAC MCR */
  hdac->Instance->MCR = tmpreg1;
  
  /* DAC in normal operating mode hence clear DAC_CR_CENx bit */
  CLEAR_BIT (hdac->Instance->CR, DAC_CR_CEN1 << Channel);
  
  /* Get the DAC CR value */
  tmpreg1 = hdac->Instance->CR;
  /* Clear TENx, TSELx, WAVEx and MAMPx bits */
  tmpreg1 &= ~(((uint32_t)(DAC_CR_MAMP1 | DAC_CR_WAVE1 | DAC_CR_TSEL1 | DAC_CR_TEN1)) << Channel); 
  /* Configure for the selected DAC channel: trigger */
  /* Set TSELx and TENx bits according to DAC_Trigger value */
  tmpreg2 = (sConfig->DAC_Trigger);
  /* Calculate CR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << Channel;

  /* Write to DAC CR */
  hdac->Instance->CR = tmpreg1;
      
  /* Disable wave generation */
  hdac->Instance->CR &= ~(DAC_CR_WAVE1 << Channel);
  
  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;
  
  /* Process unlocked */
  __HAL_UNLOCK(hdac);
  
  /* Return function status */
  return HAL_OK;
}



/**
  * @}
  */

/** @defgroup DAC_Exported_Functions_Group4 Peripheral State and Errors functions
 *  @brief   Peripheral State and Errors functions 
 *
@verbatim   
  ==============================================================================
            ##### Peripheral State and Errors functions #####
  ==============================================================================  
    [..]
    This subsection provides functions allowing to
      (+) Check the DAC state.
      (+) Check the DAC Errors.
        
@endverbatim
  * @{
  */

/**
  * @brief  return the DAC handle state
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL state
  */
HAL_DAC_StateTypeDef HAL_DAC_GetState(DAC_HandleTypeDef* hdac)
{
  /* Return DAC handle state */
  return hdac->State;
}

/**
  * @brief  Return the DAC error code
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval DAC Error Code
  */
uint32_t HAL_DAC_GetError(DAC_HandleTypeDef *hdac)
{
  return hdac->ErrorCode;
}

/**
  * @}
  */

/**
  * @}
  */


/** @addtogroup DAC_Private_Functions
  * @{
  */

/**
  * @brief  DMA conversion complete callback. 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void DAC_DMAConvCpltCh1(DMA_HandleTypeDef *hdma)   
{
  DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  
  HAL_DAC_ConvCpltCallbackCh1(hdac); 
  
  hdac->State= HAL_DAC_STATE_READY;
}

/**
  * @brief  DMA half transfer complete callback. 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void DAC_DMAHalfConvCpltCh1(DMA_HandleTypeDef *hdma)   
{
    DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
    /* Conversion complete callback */
    HAL_DAC_ConvHalfCpltCallbackCh1(hdac); 
}

/**
  * @brief  DMA error callback 
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void DAC_DMAErrorCh1(DMA_HandleTypeDef *hdma)   
{
  DAC_HandleTypeDef* hdac = ( DAC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
    
  /* Set DAC error code to DMA error */
  hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
    
  HAL_DAC_ErrorCallbackCh1(hdac); 
    
  hdac->State= HAL_DAC_STATE_READY;
}

/**
  * @}
  */

/**
  * @}
  */
#endif /* HAL_DAC_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
