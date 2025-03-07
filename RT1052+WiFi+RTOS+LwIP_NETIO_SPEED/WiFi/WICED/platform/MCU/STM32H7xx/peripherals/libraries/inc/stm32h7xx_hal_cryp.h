/**
  ******************************************************************************
  * @file    stm32h7xx_hal_cryp.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-August-2017
  * @brief   Header file of CRYP HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32H7xx_HAL_CRYP_H
#define __STM32H7xx_HAL_CRYP_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "../../../../STM32H7xx/peripherals/libraries/inc/stm32h7xx_hal_def.h"

/** @addtogroup STM32H7xx_HAL_Driver
  * @{
  */
#if defined (CRYP)
/** @addtogroup CRYP
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup CRYP_Exported_Types CRYP Exported Types
  * @{
  */

/**
  * @brief CRYP Init Structure definition
  */

typedef struct
{
  uint32_t DataType;                   /*!< 32-bit data, 16-bit data, 8-bit data or 1-bit string.
                                        This parameter can be a value of @ref CRYP_Data_Type */
  uint32_t KeySize;                    /*!< Used only in AES mode : 128, 192 or 256 bit key length in CRYP1. 
                                            This parameter can be a value of @ref CRYP_Key_Size */
  uint32_t* pKey;                      /*!< The key used for encryption/decryption */
  uint32_t* pInitVect;                 /*!< The initialization vector used also as initialization
                                         counter in CTR mode */
  uint32_t Algorithm;                  /*!<  DES/ TDES Algorithm ECB/CBC 
                                        AES Algorithm ECB/CBC/CTR/GCM or CCM
                                        This parameter can be a value of @ref CRYP_Algorithm_Mode */
  uint32_t* Header;                    /*!< used only in AES GCM and CCM Algorithm for authentication,
                                        GCM : also known as Additional Authentication Data
                                        CCM : named B1 composed of the associated data length and Associated Data. */
  uint32_t HeaderSize;                /*!< The size of header buffer in word  */
  uint32_t* B0;                       /*!< B0 is first authentication block used only  in AES CCM mode */
}CRYP_ConfigTypeDef;


/**
  * @brief  CRYP State Structure definition
  */
    
typedef enum
{
  HAL_CRYP_STATE_RESET             = 0x00U,  /*!< CRYP not yet initialized or disabled  */
  HAL_CRYP_STATE_READY             = 0x01U,  /*!< CRYP initialized and ready for use    */
  HAL_CRYP_STATE_BUSY              = 0x02U  /*!< CRYP BUSY, internal processing is ongoing  */
}HAL_CRYP_STATETypeDef;
  

/**
  * @brief  CRYP handle Structure definition
  */
  
typedef struct
{

      CRYP_TypeDef                      *Instance;            /*!< CRYP registers base address */

      CRYP_ConfigTypeDef                Init;             /*!< CRYP required parameters */ 
                                                            
      uint32_t                          *pCrypInBuffPtr;  /*!< Pointer to CRYP processing (encryption, decryption,...) buffer */

      uint32_t                          *pCrypOutBuffPtr; /*!< Pointer to CRYP processing (encryption, decryption,...) buffer */

      __IO uint16_t                     CrypHeaderCount;   /*!< Counter of header data */
      
      __IO uint16_t                     CrypInCount;      /*!< Counter of input data */

      __IO uint16_t                     CrypOutCount;     /*!< Counter of output data */
      
      uint16_t                          Size;           /*!< length of input data in word */

      uint32_t                          Phase;            /*!< CRYP peripheral phase */

      DMA_HandleTypeDef                 *hdmain;          /*!< CRYP In DMA handle parameters */

      DMA_HandleTypeDef                 *hdmaout;         /*!< CRYP Out DMA handle parameters */

      HAL_LockTypeDef                   Lock;             /*!< CRYP locking object */

      __IO  HAL_CRYP_STATETypeDef       State;            /*!< CRYP peripheral state */
      
      __IO uint32_t                     ErrorCode;        /*!< CRYP peripheral error code */
      
}CRYP_HandleTypeDef;


/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup CRYP_Exported_Constants CRYP Exported Constants
  * @{
  */

/** @defgroup CRYP_Error_Definition   CRYP Error Definition
  * @{
  */
#define HAL_CRYP_ERROR_NONE              0x00000000U  /*!< No error        */
#define HAL_CRYP_ERROR_WRITE             0x00000001U  /*!< Write error     */
#define HAL_CRYP_ERROR_READ              0x00000002U  /*!< Read error      */
#define HAL_CRYP_ERROR_DMA               0x00000004U  /*!< DMA error       */  
#define HAL_CRYP_ERROR_BUSY              0x00000008U  /*!< Busy flag error */
#define HAL_CRYP_ERROR_TIMEOUT           0x00000010U  /*!< Timeout error */
#define HAL_CRYP_ERROR_NOT_SUPPORTED     0x00000020U  /*!< Not supported mode */
#define HAL_CRYP_ERROR_AUTH_TAG_SEQUENCE 0x00000040U  /*!< Sequence are not respected only for GCM or CCM */
/** 
  * @}
  */
    
    
/** @defgroup CRYP_Algorithm_Mode CRYP Algorithm Mode
  * @{
  */

#define CRYP_DES_ECB     CRYP_CR_ALGOMODE_DES_ECB
#define CRYP_DES_CBC     CRYP_CR_ALGOMODE_DES_CBC
#define CRYP_TDES_ECB    CRYP_CR_ALGOMODE_TDES_ECB
#define CRYP_TDES_CBC    CRYP_CR_ALGOMODE_TDES_CBC 
#define CRYP_AES_ECB     CRYP_CR_ALGOMODE_AES_ECB
#define CRYP_AES_CBC     CRYP_CR_ALGOMODE_AES_CBC
#define CRYP_AES_CTR     CRYP_CR_ALGOMODE_AES_CTR
#define CRYP_AES_GCM     CRYP_CR_ALGOMODE_AES_GCM    
#define CRYP_AES_CCM     CRYP_CR_ALGOMODE_AES_CCM 

/** 
  * @}
  */                   

/** @defgroup CRYP_Key_Size CRYP Key Size
  * @{
  */

#define CRYP_KEYSIZE_128B         0x00000000U
#define CRYP_KEYSIZE_192B         CRYP_CR_KEYSIZE_0
#define CRYP_KEYSIZE_256B         CRYP_CR_KEYSIZE_1

/**                                
  * @}
  */

/** @defgroup CRYP_Data_Type CRYP Data Type
  * @{
  */

#define CRYP_DATATYPE_32B         0x00000000U
#define CRYP_DATATYPE_16B         CRYP_CR_DATATYPE_0
#define CRYP_DATATYPE_8B          CRYP_CR_DATATYPE_1
#define CRYP_DATATYPE_1B          CRYP_CR_DATATYPE
    
/**                                
  * @}
  */

/** @defgroup CRYP_Interrupt  CRYP Interrupt
  * @{
  */

#define CRYP_IT_INI       CRYP_IMSCR_INIM   /*!< Input FIFO Interrupt */
#define CRYP_IT_OUTI      CRYP_IMSCR_OUTIM  /*!< Output FIFO Interrupt */

/**
  * @}
  */

/** @defgroup CRYP_Flags CRYP Flags
  * @{
  */

/* Flags in the SR register */
#define CRYP_FLAG_IFEM    CRYP_SR_IFEM  /*!< Input FIFO is empty */
#define CRYP_FLAG_IFNF    CRYP_SR_IFNF  /*!< Input FIFO is not Full */
#define CRYP_FLAG_OFNE    CRYP_SR_OFNE  /*!< Output FIFO is not empty */
#define CRYP_FLAG_OFFU    CRYP_SR_OFFU  /*!< Output FIFO is Full */
#define CRYP_FLAG_BUSY    CRYP_SR_BUSY  /*!< The CRYP core is currently processing a block of data 
                                             or a key preparation (for AES decryption). */
/* Flags in the RISR register */
#define CRYP_FLAG_OUTRIS  0x01000002U  /*!< Output FIFO service raw interrupt status */
#define CRYP_FLAG_INRIS   0x01000001U  /*!< Input FIFO service raw interrupt status*/  

/**
  * @}
  */
    

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup CRYP_Exported_Macros CRYP Exported Macros
  * @{
  */

/**
  * @brief  Enable/Disable the CRYP peripheral.
  * @param  __HANDLE__: specifies the CRYP handle.
  * @retval None
  */

#define __HAL_CRYP_ENABLE(__HANDLE__)  ((__HANDLE__)->Instance->CR |=  CRYP_CR_CRYPEN)
#define __HAL_CRYP_DISABLE(__HANDLE__) ((__HANDLE__)->Instance->CR &=  ~CRYP_CR_CRYPEN) 

/** @brief  Check whether the specified CRYP status flag is set or not.
  * @param  __FLAG__: specifies the flag to check.                
  *         This parameter can be one of the following values for CRYP:
  *            @arg CRYP_FLAG_BUSY: The CRYP core is currently processing a block of data 
  *                                 or a key preparation (for AES decryption). 
  *            @arg CRYP_FLAG_IFEM: Input FIFO is empty
  *            @arg CRYP_FLAG_IFNF: Input FIFO is not full
  *            @arg CRYP_FLAG_INRIS: Input FIFO service raw interrupt is pending
  *            @arg CRYP_FLAG_OFNE: Output FIFO is not empty
  *            @arg CRYP_FLAG_OFFU: Output FIFO is full
  *            @arg CRYP_FLAG_OUTRIS: Input FIFO service raw interrupt is pending 
 * @retval The state of __FLAG__ (TRUE or FALSE).
  */
#define CRYP_FLAG_MASK  0x0000001FU
                                                     
#define __HAL_CRYP_GET_FLAG(__HANDLE__, __FLAG__) ((((uint8_t)((__FLAG__) >> 24)) == 0x01U)?((((__HANDLE__)->Instance->RISR) & ((__FLAG__) & CRYP_FLAG_MASK)) == ((__FLAG__) & CRYP_FLAG_MASK)): \
                                                 ((((__HANDLE__)->Instance->RISR) & ((__FLAG__) & CRYP_FLAG_MASK)) == ((__FLAG__) & CRYP_FLAG_MASK)))

/** @brief  Check whether the specified CRYP interrupt is set or not.
  * @param  __HANDLE__: specifies the CRYP handle.
  * @param  __INTERRUPT__: specifies the interrupt to check.
  *         This parameter can be one of the following values for CRYP:
  *            @arg CRYP_IT_INI: Input FIFO service masked interrupt status
  *            @arg CRYP_IT_OUTI: Output FIFO service masked interrupt status
  * @retval The state of __INTERRUPT__ (TRUE or FALSE).
  */
  
#define __HAL_CRYP_GET_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->MISR & (__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief  Enable the CRYP interrupt. 
  * @param  __HANDLE__: specifies the CRYP handle.  
  * @param  __INTERRUPT__: CRYP Interrupt.
  *         This parameter can be one of the following values for CRYP:
  *            @ CRYP_IT_INI : Input FIFO service interrupt mask.
  *            @ CRYP_IT_OUTI : Output FIFO service interrupt mask.CRYP interrupt.
  * @retval None
  */

#define __HAL_CRYP_ENABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IMSCR) |= (__INTERRUPT__))

/**
  * @brief  Disable the CRYP interrupt.
  * @param  __HANDLE__: specifies the CRYP handle.
  * @param  __INTERRUPT__: CRYP Interrupt.
  *         This parameter can be one of the following values for CRYP:
  *            @ CRYP_IT_INI : Input FIFO service interrupt mask.
  *            @ CRYP_IT_OUTI : Output FIFO service interrupt mask.CRYP interrupt.
  * @retval None
  */

#define __HAL_CRYP_DISABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IMSCR) &= ~(__INTERRUPT__))

/**
  * @}
  */

/* Include CRYP HAL Extended module */
#include "../../../../STM32H7xx/peripherals/libraries/inc/stm32h7xx_hal_cryp_ex.h"
    
/* Exported functions --------------------------------------------------------*/
/** @defgroup CRYP_Exported_Functions CRYP Exported Functions
  * @{
  */

/** @addtogroup CRYP_Exported_Functions_Group1
  * @{
  */    
HAL_StatusTypeDef HAL_CRYP_Init(CRYP_HandleTypeDef *hcryp);
HAL_StatusTypeDef HAL_CRYP_DeInit(CRYP_HandleTypeDef *hcryp);
void HAL_CRYP_MspInit(CRYP_HandleTypeDef *hcryp);
void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef *hcryp);
HAL_StatusTypeDef HAL_CRYP_SetConfig(CRYP_HandleTypeDef *hcryp, CRYP_ConfigTypeDef *pConf );
HAL_StatusTypeDef HAL_CRYP_GetConfig(CRYP_HandleTypeDef *hcryp, CRYP_ConfigTypeDef *pConf );

/**
  * @}
  */ 

/** @addtogroup CRYP_Exported_Functions_Group2
  * @{
  */ 

/* encryption/decryption ***********************************/
HAL_StatusTypeDef HAL_CRYP_Encrypt(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output, uint32_t Timeout);
HAL_StatusTypeDef HAL_CRYP_Decrypt(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output, uint32_t Timeout);
HAL_StatusTypeDef HAL_CRYP_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output);
HAL_StatusTypeDef HAL_CRYP_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output);
HAL_StatusTypeDef HAL_CRYP_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output);
HAL_StatusTypeDef HAL_CRYP_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint32_t *Input, uint16_t Size, uint32_t *Output);

/**
  * @}
  */ 


/** @addtogroup CRYP_Exported_Functions_Group3
  * @{
  */  
/* Interrupt Handler functions  **********************************************/
void HAL_CRYP_IRQHandler(CRYP_HandleTypeDef *hcryp);
HAL_CRYP_STATETypeDef HAL_CRYP_GetState(CRYP_HandleTypeDef *hcryp);
void HAL_CRYP_InCpltCallback(CRYP_HandleTypeDef *hcryp);
void HAL_CRYP_OutCpltCallback(CRYP_HandleTypeDef *hcryp);
void HAL_CRYP_ErrorCallback(CRYP_HandleTypeDef *hcryp);
uint32_t HAL_CRYP_GetError(CRYP_HandleTypeDef *hcryp);

/**
  * @}
  */ 
  
/**
  * @}
  */

/* Private macros --------------------------------------------------------*/
/** @defgroup CRYP_Private_Macros   CRYP Private Macros
  * @{
  */

/** @defgroup CRYP_IS_CRYP_Definitions CRYP Private macros to check input parameters
  * @{
  */

#define IS_CRYP_ALGORITHM(ALGORITHM) (((ALGORITHM) == CRYP_DES_ECB)   || \
                                   ((ALGORITHM)  == CRYP_DES_CBC)   || \
                                   ((ALGORITHM)  == CRYP_TDES_ECB)  || \
                                   ((ALGORITHM)  == CRYP_TDES_CBC)  || \
                                   ((ALGORITHM)  == CRYP_AES_ECB)   || \
                                   ((ALGORITHM)  == CRYP_AES_CBC)   || \
                                   ((ALGORITHM)  == CRYP_AES_CTR)   || \
                                   ((ALGORITHM)  == CRYP_AES_GCM)   || \
                                   ((ALGORITHM)  == CRYP_AES_CCM))

#define IS_CRYP_KEYSIZE(KEYSIZE)(((KEYSIZE) == CRYP_KEYSIZE_128B)   || \
                                 ((KEYSIZE) == CRYP_KEYSIZE_192B)   || \
                                 ((KEYSIZE) == CRYP_KEYSIZE_256B))

#define IS_CRYP_DATATYPE(DATATYPE)(((DATATYPE) == CRYP_DATATYPE_32B)   || \
                                   ((DATATYPE) == CRYP_DATATYPE_16B) || \
                                   ((DATATYPE) == CRYP_DATATYPE_8B) || \
                                   ((DATATYPE) == CRYP_DATATYPE_1B))

/**
  * @}
  */

/**
  * @}
  */


/* Private constants ---------------------------------------------------------*/
/** @defgroup CRYP_Private_Constants CRYP Private Constants
  * @{
  */

/**
  * @}
  */ 
/* Private defines -----------------------------------------------------------*/
/** @defgroup CRYP_Private_Defines CRYP Private Defines
  * @{
  */

/**
  * @}
  */ 
          
/* Private variables ---------------------------------------------------------*/
/** @defgroup CRYP_Private_Variables CRYP Private Variables
  * @{
  */

/**
  * @}
  */ 
/* Private functions prototypes ----------------------------------------------*/
/** @defgroup CRYP_Private_Functions_Prototypes CRYP Private Functions Prototypes
  * @{
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup CRYP_Private_Functions CRYP Private Functions
  * @{
  */

/**
  * @}
  */
 
     
/**
  * @}
  */ 


#endif /* CRYP */ 
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif /* __STM32H7xx_HAL_CRYP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
