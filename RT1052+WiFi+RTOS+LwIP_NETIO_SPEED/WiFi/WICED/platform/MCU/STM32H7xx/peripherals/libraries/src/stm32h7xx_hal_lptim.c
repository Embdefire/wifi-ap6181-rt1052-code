/**
  ******************************************************************************
  * @file    stm32h7xx_hal_lptim.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-August-2017
  * @brief   LPTIM HAL module driver.
  *    
  *          This file provides firmware functions to manage the following 
  *          functionalities of the Low Power Timer (LPTIM) peripheral:
  *           + Initialization and de-initialization functions.
  *           + Start/Stop operation functions in polling mode.
  *           + Start/Stop operation functions in interrupt mode.
  *           + Reading operation functions.
  *           + Peripheral State functions.
  *         
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      The LPTIM HAL driver can be used as follows:

      (#)Initialize the LPTIM low level resources by implementing the
        HAL_LPTIM_MspInit():
         (++) Enable the LPTIM interface clock using __HAL_RCC_LPTIMx_CLK_ENABLE().
         (++) In case of using interrupts (e.g. HAL_LPTIM_PWM_Start_IT()):
             (+++) Configure the LPTIM interrupt priority using HAL_NVIC_SetPriority().
             (+++) Enable the LPTIM IRQ handler using HAL_NVIC_EnableIRQ().
             (+++) In LPTIM IRQ handler, call HAL_LPTIM_IRQHandler().
    
      (#)Initialize the LPTIM HAL using HAL_LPTIM_Init(). This function
         configures mainly:
         (++) The instance: LPTIM1 or LPTIM2.
         (++) Clock: the counter clock.
                 (+++) Source   : it can be either the ULPTIM input (IN1) or one of
                              the internal clock; (APB, LSE, LSI or MSI).
                 (+++) Prescaler: select the clock divider.
         (++)  UltraLowPowerClock : To be used only if the ULPTIM is selected
               as counter clock source.
                 (+++) Polarity:   polarity of the active edge for the counter unit
                               if the ULPTIM input is selected.
                 (+++) SampleTime: clock sampling time to configure the clock glitch
                               filter.              
         (++) Trigger: How the counter start.
                 (+++) Source: trigger can be software or one of the hardware triggers.
                 (+++) ActiveEdge : only for hardware trigger.
                 (+++) SampleTime : trigger sampling time to configure the trigger
                                glitch filter.
         (++) OutputPolarity : 2 opposite polarities are possibles.
         (++) UpdateMode: specifies whether the update of the autoreload and
              the compare values is done immediately or after the end of current
              period.
         (++) Input1Source: Source selected for input1 (GPIO or comparator output).
         (++) Input2Source: Source selected for input2 (GPIO or comparator output). 
              Input2 is used only for encoder feature so is used only for LPTIM1 instance.
    
      (#)Six modes are available:
      
         (++) PWM Mode: To generate a PWM signal with specified period and pulse,
         call HAL_LPTIM_PWM_Start() or HAL_LPTIM_PWM_Start_IT() for interruption
         mode.
         
         (++) One Pulse Mode: To generate pulse with specified width in response
         to a stimulus, call HAL_LPTIM_OnePulse_Start() or
         HAL_LPTIM_OnePulse_Start_IT() for interruption mode.
         
         (++) Set once Mode: In this mode, the output changes the level (from
         low level to high level if the output polarity is configured high, else
         the opposite) when a compare match occurs. To start this mode, call 
         HAL_LPTIM_SetOnce_Start() or HAL_LPTIM_SetOnce_Start_IT() for
         interruption mode.
         
         (++) Encoder Mode: To use the encoder interface call
         HAL_LPTIM_Encoder_Start() or HAL_LPTIM_Encoder_Start_IT() for 
         interruption mode. Only available for LPTIM1 instance.
         
         (++) Time out Mode: an active edge on one selected trigger input rests
         the counter. The first trigger event will start the timer, any
         successive trigger event will reset the counter and the timer will
         restart. To start this mode call HAL_LPTIM_TimeOut_Start_IT() or 
         HAL_LPTIM_TimeOut_Start_IT() for interruption mode.
         
         (++) Counter Mode: counter can be used to count external events on
         the LPTIM Input1 or it can be used to count internal clock cycles.
         To start this mode, call HAL_LPTIM_Counter_Start() or 
         HAL_LPTIM_Counter_Start_IT() for interruption mode.             

    
      (#) User can stop any process by calling the corresponding API:
          HAL_LPTIM_Xxx_Stop() or HAL_LPTIM_Xxx_Stop_IT() if the process is
          already started in interruption mode.
         
       (#)De-initialize the LPTIM peripheral using HAL_LPTIM_DeInit().

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

/** @defgroup LPTIM LPTIM
  * @brief LPTIM HAL module driver.
  * @{
  */

#ifdef HAL_LPTIM_MODULE_ENABLED
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup LPTIM_Exported_Functions LPTIM Exported Functions
  * @{
  */

/** @defgroup LPTIM_Exported_Functions_Group1 Initialization/de-initialization functions 
 *  @brief    Initialization and Configuration functions. 
 *
@verbatim    
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the LPTIM according to the specified parameters in the
          LPTIM_InitTypeDef and initialize the associated handle.
      (+) DeInitialize the LPTIM peripheral.
      (+) Initialize the LPTIM MSP.
      (+) DeInitialize the LPTIM MSP. 
 
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the LPTIM according to the specified parameters in the
  *         LPTIM_InitTypeDef and creates the associated handle.
  * @param  hlptim: LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim)
{
  uint32_t tmpcfgr = 0;

  /* Check the LPTIM handle allocation */
  if(hlptim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  assert_param(IS_LPTIM_CLOCK_SOURCE(hlptim->Init.Clock.Source));
  assert_param(IS_LPTIM_CLOCK_PRESCALER(hlptim->Init.Clock.Prescaler));  
  if ((hlptim->Init.Clock.Source) ==  LPTIM_CLOCKSOURCE_ULPTIM)
  {
    assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));
    assert_param(IS_LPTIM_CLOCK_SAMPLE_TIME(hlptim->Init.UltraLowPowerClock.SampleTime));
  }  
  assert_param(IS_LPTIM_TRG_SOURCE(hlptim->Init.Trigger.Source));
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    assert_param(IS_LPTIM_TRIG_SAMPLE_TIME(hlptim->Init.Trigger.SampleTime));
    assert_param(IS_LPTIM_EXT_TRG_POLARITY(hlptim->Init.Trigger.ActiveEdge));
  }  
  assert_param(IS_LPTIM_OUTPUT_POLARITY(hlptim->Init.OutputPolarity));  
  assert_param(IS_LPTIM_UPDATE_MODE(hlptim->Init.UpdateMode));
  assert_param(IS_LPTIM_COUNTER_SOURCE(hlptim->Init.CounterSource));
  
  if(hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hlptim->Lock = HAL_UNLOCKED;

    /* Init the low level hardware */
    HAL_LPTIM_MspInit(hlptim);
  }
  
  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;
  
  /* Get the LPTIMx CFGR value */
  tmpcfgr = hlptim->Instance->CFGR;
  
  if ((hlptim->Init.Clock.Source) ==  LPTIM_CLOCKSOURCE_ULPTIM)
  {
    tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_CKPOL | LPTIM_CFGR_CKFLT));
  }
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    tmpcfgr &= (uint32_t)(~ (LPTIM_CFGR_TRGFLT | LPTIM_CFGR_TRIGSEL));
  }
    
  /* Clear CKSEL, PRESC, TRIGEN, TRGFLT, WAVPOL, PRELOAD & COUNTMODE bits */
  tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_CKSEL | LPTIM_CFGR_TRIGEN | LPTIM_CFGR_PRELOAD |
                          LPTIM_CFGR_WAVPOL | LPTIM_CFGR_PRESC | LPTIM_CFGR_COUNTMODE ));
  
  /* Set initialization parameters */
  tmpcfgr |= (hlptim->Init.Clock.Source    |
              hlptim->Init.Clock.Prescaler |
              hlptim->Init.OutputPolarity  |
              hlptim->Init.UpdateMode      |
              hlptim->Init.CounterSource);
  
  if ((hlptim->Init.Clock.Source) ==  LPTIM_CLOCKSOURCE_ULPTIM)
  {
    tmpcfgr |=  (hlptim->Init.UltraLowPowerClock.Polarity |
                hlptim->Init.UltraLowPowerClock.SampleTime);
  } 
  
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable External trigger and set the trigger source */
    tmpcfgr |= (hlptim->Init.Trigger.Source     |
                hlptim->Init.Trigger.ActiveEdge |
                hlptim->Init.Trigger.SampleTime);
  }
  
  /* Write to LPTIMx CFGR */
  hlptim->Instance->CFGR = tmpcfgr;

 /* Configure LPTIM input sources */
  if((hlptim->Instance == LPTIM1) || (hlptim->Instance == LPTIM2))
  {
    /* Check LPTIM1/2 Input1 and Input2 sources */
    assert_param(IS_LPTIM_INPUT1_SOURCE(hlptim->Instance,hlptim->Init.Input1Source));
    assert_param(IS_LPTIM_INPUT2_SOURCE(hlptim->Instance,hlptim->Init.Input2Source));
    
    /* Configure LPTIM1/2 Input1 and Input2 sources */
    hlptim->Instance->CFGR2 = (hlptim->Init.Input1Source | hlptim->Init.Input2Source);
  }
  else
  {
  if(hlptim->Instance == LPTIM3)
    {
      /* Check LPTIM2 Input1 source */
      assert_param(IS_LPTIM_INPUT1_SOURCE(hlptim->Instance,hlptim->Init.Input1Source));
    
      /* Configure LPTIM2 Input1 source */
      hlptim->Instance->CFGR2 = hlptim->Init.Input1Source;
    }
  }
  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the LPTIM peripheral. 
  * @param  hlptim: LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the LPTIM handle allocation */
  if(hlptim == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;
  
  /* Disable the LPTIM Peripheral Clock */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* DeInit the low level hardware: CLOCK, NVIC.*/
  HAL_LPTIM_MspDeInit(hlptim);
  
  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_RESET;
  
  /* Release Lock */
  __HAL_UNLOCK(hlptim);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the LPTIM MSP.
  * @param  hlptim: LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize LPTIM MSP.
  * @param  hlptim: LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_MspDeInit could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group2 LPTIM Start-Stop operation functions  
 *  @brief   Start-Stop operation functions. 
 *
@verbatim   
  ==============================================================================
                ##### LPTIM Start Stop operation functions #####
  ==============================================================================  
    [..]  This section provides functions allowing to:
      (+) Start the PWM mode.
      (+) Stop the PWM mode.
      (+) Start the One pulse mode.
      (+) Stop the One pulse mode.
      (+) Start the Set once mode.
      (+) Stop the Set once mode.
      (+) Start the Encoder mode.
      (+) Stop the Encoder mode.
      (+) Start the Timeout mode.
      (+) Stop the Timeout mode.      
      (+) Start the Counter mode.
      (+) Stop the Counter mode.
      

@endverbatim
  * @{
  */
    
/**
  * @brief  Start the LPTIM PWM generation.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
 
  /* Reset WAVE bit to set PWM mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM PWM generation.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM PWM generation in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
 
  /* Reset WAVE bit to set PWM mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;
  
  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }  
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM PWM generation in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_PWM_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
    /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }  

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM One pulse generation.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Reset WAVE bit to set one pulse mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM One pulse generation.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM One pulse generation in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Reset WAVE bit to set one pulse mode */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_WAVE;
  
  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM One pulse generation in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM in Set once mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Set WAVE bit to enable the set once mode */
  hlptim->Instance->CFGR |= LPTIM_CFGR_WAVE;
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM Set once mode in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Pulse : Specifies the compare value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Pulse));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Set WAVE bit to enable the set once mode */
  hlptim->Instance->CFGR |= LPTIM_CFGR_WAVE;
  
  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Enable Compare write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then enable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Enable external trigger interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  }  
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the pulse value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Pulse);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_SINGLE(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Disable Compare write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPOK);
  
  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* If external trigger source is used, then disable external trigger interrupt */
  if ((hlptim->Init.Trigger.Source) !=  LPTIM_TRIGSOURCE_SOFTWARE)
  {
    /* Disable external trigger interrupt */
    __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_EXTTRIG);
  } 
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Starts the Encoder interface.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t          tmpcfgr;

  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(hlptim->Init.Clock.Source == LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC);
  assert_param(hlptim->Init.Clock.Prescaler == LPTIM_PRESCALER_DIV1);
  assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));

  /* Encoder feature is only available for LPTIM1 instance */
  if ((hlptim->Instance == LPTIM1) || (hlptim->Instance == LPTIM2))
  {
    /* Set the LPTIM state */
    hlptim->State= HAL_LPTIM_STATE_BUSY;

    /* Get the LPTIMx CFGR value */
    tmpcfgr = hlptim->Instance->CFGR;

    /* Clear CKPOL bits */
    tmpcfgr &= (uint32_t)(~LPTIM_CFGR_CKPOL);

    /* Set Input polarity */
    tmpcfgr |=  hlptim->Init.UltraLowPowerClock.Polarity;

    /* Write to LPTIMx CFGR */
    hlptim->Instance->CFGR = tmpcfgr;

    /* Set ENC bit to enable the encoder interface */
    hlptim->Instance->CFGR |= LPTIM_CFGR_ENC;

    /* Enable the Peripheral */
    __HAL_LPTIM_ENABLE(hlptim);

    /* Load the period value in the autoreload register */
    __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

    /* Start timer in continuous mode */
    __HAL_LPTIM_START_CONTINUOUS(hlptim);

    /* Change the TIM state*/
    hlptim->State= HAL_LPTIM_STATE_READY;
  }
  else
  {
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Stop the Encoder interface.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Reset ENC bit to disable the encoder interface */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_ENC;
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Encoder interface in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t          tmpcfgr;

  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(hlptim->Init.Clock.Source == LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC);
  assert_param(hlptim->Init.Clock.Prescaler == LPTIM_PRESCALER_DIV1);
  assert_param(IS_LPTIM_CLOCK_POLARITY(hlptim->Init.UltraLowPowerClock.Polarity));

  /* Encoder feature is only available for LPTIM1 instance */
  if (hlptim->Instance == LPTIM1)
  {
    /* Set the LPTIM state */
    hlptim->State= HAL_LPTIM_STATE_BUSY;

    /* Configure edge sensitivity for encoder mode */
    /* Get the LPTIMx CFGR value */
    tmpcfgr = hlptim->Instance->CFGR;

    /* Clear CKPOL bits */
    tmpcfgr &= (uint32_t)(~LPTIM_CFGR_CKPOL);

    /* Set Input polarity */
    tmpcfgr |=  hlptim->Init.UltraLowPowerClock.Polarity;

    /* Write to LPTIMx CFGR */
    hlptim->Instance->CFGR = tmpcfgr;

    /* Set ENC bit to enable the encoder interface */
    hlptim->Instance->CFGR |= LPTIM_CFGR_ENC;

    /* Enable "switch to down direction" interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_DOWN);

    /* Enable "switch to up direction" interrupt */
    __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_UP);  

    /* Enable the Peripheral */
    __HAL_LPTIM_ENABLE(hlptim);

    /* Load the period value in the autoreload register */
    __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

    /* Start timer in continuous mode */
    __HAL_LPTIM_START_CONTINUOUS(hlptim);

    /* Change the TIM state*/
    hlptim->State= HAL_LPTIM_STATE_READY;
  }
  else
  {
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Stop the Encoder interface in nterrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Encoder_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Reset ENC bit to disable the encoder interface */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_ENC;
  
  /* Disable "switch to down direction" interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_DOWN);
  
  /* Disable "switch to up direction" interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_UP); 
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Timeout function.
  * @note   The first trigger event will start the timer, any successive
  *         trigger event will reset the counter and the timer restarts.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Timeout : Specifies the TimeOut value to rest the counter.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Timeout)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Timeout));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
 
  /* Set TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR |= LPTIM_CFGR_TIMOUT;
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the Timeout value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Timeout);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Timeout function.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Reset TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_TIMOUT;
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Timeout function in interrupt mode.
  * @note   The first trigger event will start the timer, any successive
  *         trigger event will reset the counter and the timer restarts.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @param  Timeout : Specifies the TimeOut value to rest the counter.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Timeout)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
  assert_param(IS_LPTIM_PULSE(Timeout));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
 
  /* Set TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR |= LPTIM_CFGR_TIMOUT;
  
  /* Enable Compare match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Load the Timeout value in the compare register */
  __HAL_LPTIM_COMPARE_SET(hlptim, Timeout);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Timeout function in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_TimeOut_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Reset TIMOUT bit to enable the timeout function */
  hlptim->Instance->CFGR &= ~LPTIM_CFGR_TIMOUT;
  
  /* Disable Compare match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Counter mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* If clock source is not ULPTIM clock and counter source is external, then it must not be prescaled */
  if((hlptim->Init.Clock.Source != LPTIM_CLOCKSOURCE_ULPTIM) && (hlptim->Init.CounterSource == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    /* Check if clock is prescaled */
    assert_param(IS_LPTIM_CLOCK_PRESCALERDIV1(hlptim->Init.Clock.Prescaler));
    /* Set clock prescaler to 0 */
    hlptim->Instance->CFGR &= ~LPTIM_CFGR_PRESC;
  }

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Counter mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the Counter mode in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @param  Period : Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));
               
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* If clock source is not ULPTIM clock and counter source is external, then it must not be prescaled */
  if((hlptim->Init.Clock.Source != LPTIM_CLOCKSOURCE_ULPTIM) && (hlptim->Init.CounterSource == LPTIM_COUNTERSOURCE_EXTERNAL))
  {
    /* Check if clock is prescaled */
    assert_param(IS_LPTIM_CLOCK_PRESCALERDIV1(hlptim->Init.Clock.Prescaler));
    /* Set clock prescaler to 0 */
    hlptim->Instance->CFGR &= ~LPTIM_CFGR_PRESC;
  }
  
  /* Enable Autoreload write complete interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);
  
  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the Counter mode in interrupt mode.
  * @param  hlptim : LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Counter_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;
  
  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);
  
  /* Disable Autoreload write complete interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARROK);
  
  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);
  
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group3 LPTIM Read operation functions 
 *  @brief  Read operation functions.
 *
@verbatim   
  ==============================================================================
                  ##### LPTIM Read operation functions #####
  ==============================================================================  
[..]  This section provides LPTIM Reading functions.
      (+) Read the counter value.
      (+) Read the period (Auto-reload) value.
      (+) Read the pulse (Compare)value.
@endverbatim
  * @{
  */

/**
  * @brief  Return the current counter value.
  * @param  hlptim: LPTIM handle
  * @retval Counter value.
  */
uint32_t HAL_LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
{
    /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  return (hlptim->Instance->CNT);
}

/**
  * @brief  Return the current Autoreload (Period) value.
  * @param  hlptim: LPTIM handle
  * @retval Autoreload value.
  */
uint32_t HAL_LPTIM_ReadAutoReload(LPTIM_HandleTypeDef *hlptim)
{
    /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  return (hlptim->Instance->ARR);
}

/**
  * @brief  Return the current Compare (Pulse) value.
  * @param  hlptim: LPTIM handle
  * @retval Compare value.
  */
uint32_t HAL_LPTIM_ReadCompare(LPTIM_HandleTypeDef *hlptim)
{
    /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  
  return (hlptim->Instance->CMP);
}

/**
  * @}
  */



/** @defgroup LPTIM_Exported_Functions_Group4 LPTIM IRQ handler and callbacks
 *  @brief  LPTIM  IRQ handler.
 *
@verbatim   
  ==============================================================================
                      ##### LPTIM IRQ handler and callbacks  #####
  ==============================================================================  
[..]  This section provides LPTIM IRQ handler and callback functions called within 
      the IRQ handler.

@endverbatim
  * @{
  */

/**
  * @brief  Handle LPTIM interrupt request.
  * @param  hlptim: LPTIM handle
  * @retval None
  */
void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef *hlptim)
{
  /* Compare match interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_CMPM) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_CMPM) != RESET)
    {
      /* Clear Compare match flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_CMPM);
      
      /* Compare match Callback */
      HAL_LPTIM_CompareMatchCallback(hlptim);      
    }
  }
  
  /* Autoreload match interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARRM) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARRM) != RESET)
    {
      /* Clear Autoreload match flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARRM);
      
      /* Autoreload match Callback */
      HAL_LPTIM_AutoReloadMatchCallback(hlptim);      
    }
  }
  
  /* Trigger detected interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_EXTTRIG) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_EXTTRIG) != RESET)
    {
      /* Clear Trigger detected flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_EXTTRIG);
      
      /* Trigger detected callback */
      HAL_LPTIM_TriggerCallback(hlptim);      
    }
  }
  
  /* Compare write interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_CMPOK) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_CMPOK) != RESET)
    {
      /* Clear Compare write flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_CMPOK);
      
      /* Compare write Callback */
      HAL_LPTIM_CompareWriteCallback(hlptim);      
    }
  }
  
  /* Autoreload write interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARROK) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARROK) != RESET)
    {
      /* Clear Autoreload write flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARROK);
      
      /* Autoreload write Callback */
      HAL_LPTIM_AutoReloadWriteCallback(hlptim);      
    }
  }
  
  /* Direction counter changed from Down to Up interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_UP) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_UP) != RESET)
    {
      /* Clear Direction counter changed from Down to Up flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_UP);
      
      /* Direction counter changed from Down to Up Callback */
      HAL_LPTIM_DirectionUpCallback(hlptim);      
    }
  }
  
  /* Direction counter changed from Up to Down interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_DOWN) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_DOWN) != RESET)
    {
      /* Clear Direction counter changed from Up to Down flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_DOWN);
      
      /* Direction counter changed from Up to Down Callback */
      HAL_LPTIM_DirectionDownCallback(hlptim);      
    }
  }
}

/**
  * @brief  Compare match callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_CompareMatchCallback could be implemented in the user file
   */  
}

/**
  * @brief  Autoreload match callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadMatchCallback could be implemented in the user file
   */  
}

/**
  * @brief  Trigger detected callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_TriggerCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_TriggerCallback could be implemented in the user file
   */  
}

/**
  * @brief  Compare write callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_CompareWriteCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_CompareWriteCallback could be implemented in the user file
   */  
}

/**
  * @brief  Autoreload write callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadWriteCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadWriteCallback could be implemented in the user file
   */  
}

/**
  * @brief  Direction counter changed from Down to Up callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_DirectionUpCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_DirectionUpCallback could be implemented in the user file
   */  
}

/**
  * @brief  Direction counter changed from Up to Down callback in non-blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_DirectionDownCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_LPTIM_DirectionDownCallback could be implemented in the user file
   */  
}

/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group5 Peripheral State functions 
 *  @brief   Peripheral State functions. 
 *
@verbatim   
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================  
    [..]
    This subsection permits to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Returns the LPTIM state.
  * @param  hlptim: LPTIM handle
  * @retval HAL state
  */
HAL_LPTIM_StateTypeDef HAL_LPTIM_GetState(LPTIM_HandleTypeDef *hlptim)
{
  /* Return LPTIM handle state */
  return hlptim->State;
}

/**
  * @}
  */


/**
  * @}
  */

#endif /* HAL_LPTIM_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
