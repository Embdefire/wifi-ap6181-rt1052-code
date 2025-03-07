/**
  ******************************************************************************
  * @file    stm32h7xx_hal_eth_ex.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-August-2017
  * @brief   ETH HAL Extended module driver.
  *
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

/** @defgroup ETHEx ETHEx 
  * @brief ETH HAL Extended module driver
  * @{
  */

#ifdef HAL_ETH_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup ETHEx_Private_Constants ETHEx Private Constants
  * @{
  */
#define ETH_MACL4CR_MASK     (ETH_MACL3L4CR_L4PEN | ETH_MACL3L4CR_L4SPM | \
                              ETH_MACL3L4CR_L4SPIM | ETH_MACL3L4CR_L4DPM | \
                              ETH_MACL3L4CR_L4DPIM)

#define ETH_MACL3CR_MASK     (ETH_MACL3L4CR_L3PEN | ETH_MACL3L4CR_L3SAM | \
                              ETH_MACL3L4CR_L3SAIM | ETH_MACL3L4CR_L3DAM | \
                              ETH_MACL3L4CR_L3DAIM | ETH_MACL3L4CR_L3HSBM | \
                              ETH_MACL3L4CR_L3HDBM)

#define ETH_MACRXVLAN_MASK (ETH_MACVTR_EIVLRXS | ETH_MACVTR_EIVLS | \
                            ETH_MACVTR_ERIVLT | ETH_MACVTR_EDVLP | \
                            ETH_MACVTR_VTHM | ETH_MACVTR_EVLRXS | \
                            ETH_MACVTR_EVLS | ETH_MACVTR_DOVLTC | \
                            ETH_MACVTR_ERSVLM | ETH_MACVTR_ESVL | \
                            ETH_MACVTR_VTIM | ETH_MACVTR_ETV)

#define ETH_MACTXVLAN_MASK (ETH_MACVIR_VLTI | ETH_MACVIR_CSVL | \
                            ETH_MACVIR_VLP | ETH_MACVIR_VLC)
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/
/** @defgroup ETHEx_Exported_Functions ETH Extended Exported Functions
  * @{
  */
  
/** @defgroup ETHEx_Exported_Functions_Group1 Extended features functions
  * @brief    Extended features functions
 *
@verbatim
 ===============================================================================
                      ##### Extended features functions #####
 ===============================================================================
    [..] This section provides functions allowing to:
      (+) Configure ARP offload module
      (+) Configure L3 and L4 filters
      (+) Configure Extended VLAN features
      (+) Configure Energy Efficient Ethernet module

@endverbatim
  * @{
  */
  
/**
  * @brief  Enables ARP Offload. 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETHEx_EnableARPOffload(ETH_HandleTypeDef *heth)
{
  SET_BIT(heth->Instance->MACCR, ETH_MACCR_ARP);
}

/**
  * @brief  Disables ARP Offload. 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETHEx_DisableARPOffload(ETH_HandleTypeDef *heth)
{
  CLEAR_BIT(heth->Instance->MACCR, ETH_MACCR_ARP);
}

/**
  * @brief  Set the ARP Match IP address 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  *  @param IpAddress: IP Address to be matched for incoming ARP requests
  * @retval None
  */
void HAL_ETHEx_SetARPAddressMatch(ETH_HandleTypeDef *heth, uint32_t IpAddress)
{
  WRITE_REG(heth->Instance->MACARPAR, IpAddress);
}

/**
  * @brief  Configures the L4 Filter, this function allow to:
  *         set the layer 4 protocol to be matched (TCP or UDP)
  *         enable/disable L4 source/destination port perfect/inverse match.      
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  Filter: L4 filter to configured, this parameter must be one of the following
  *           ETH_L4_FILTER_0
  *           ETH_L4_FILTER_1
  * @param  pL4FilterConfig: pointer to a ETH_L4FilterConfigTypeDef structure 
  *         that contains L4 filter configuration. 
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_SetL4FilterConfig(ETH_HandleTypeDef *heth, uint32_t Filter , ETH_L4FilterConfigTypeDef *pL4FilterConfig)
{
  uint32_t l4filteraddr;
  uint32_t l4filterval = 0;
  
  if(pL4FilterConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  l4filteraddr = (uint32_t)(&(heth->Instance->MACL3L4C0R) + Filter);
  
  l4filterval = (pL4FilterConfig->Protocol |
             pL4FilterConfig->SrcPortFilterMatch |
             pL4FilterConfig->DestPortFilterMatch);
  
  /* Write configuration to (MACL3L4C0R + filter )register */
  MODIFY_REG((*(__IO uint32_t *)l4filteraddr), ETH_MACL4CR_MASK ,l4filterval);
  
  /* Get address of  (MACL4A0R + filter) register */
  l4filteraddr = (uint32_t)(&(heth->Instance->MACL4A0R) + Filter);
  
  l4filterval = (pL4FilterConfig->SourcePort |
             (pL4FilterConfig->DestinationPort << 16));
  
  /* Write configuration to (MACL4A0R + filter )register */
  MODIFY_REG((*(__IO uint32_t *)l4filteraddr), (ETH_MACL4AR_L4DP | ETH_MACL4AR_L4SP) , l4filterval);
  
  /* Enable L4 filter */
  SET_BIT(heth->Instance->MACPFR, ETH_MACPFR_IPFE);
  
  return HAL_OK;
}

/**
  * @brief  Configures the L4 Filter, this function allow to:
  *         set the layer 4 protocol to be matched (TCP or UDP)
  *         enable/disable L4 source/destination port perfect/inverse match.      
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  Filter: L4 filter to configured, this parameter must be one of the following
  *           ETH_L4_FILTER_0
  *           ETH_L4_FILTER_1
  * @param  pL4FilterConfig: pointer to a ETH_L4FilterConfigTypeDef structure 
  *         that contains L4 filter configuration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_GetL4FilterConfig(ETH_HandleTypeDef *heth, uint32_t Filter, ETH_L4FilterConfigTypeDef *pL4FilterConfig)
{
  uint32_t l4filterval = *((__IO uint32_t *)(&(heth->Instance->MACL3L4C0R) + Filter));
  
  if(pL4FilterConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Get configuration to (MACL3L4C0R + filter )register */
  pL4FilterConfig->Protocol = READ_BIT(l4filterval, ETH_MACL3L4CR_L4PEN);
  pL4FilterConfig->DestPortFilterMatch = READ_BIT(l4filterval, (ETH_MACL3L4CR_L4DPM | ETH_MACL3L4CR_L4DPIM));
  pL4FilterConfig->SrcPortFilterMatch = READ_BIT(l4filterval, (ETH_MACL3L4CR_L4SPM | ETH_MACL3L4CR_L4SPIM));
  
  /* Get address of  (MACL4A0R + filter) register */
  l4filterval = *((__IO uint32_t *)(&(heth->Instance->MACL4A0R) + Filter));
  
  /* Get configuration to (MACL3L4C0R + filter )register */
  pL4FilterConfig->DestinationPort = (READ_BIT(l4filterval, ETH_MACL4AR_L4DP) >> 16);
  pL4FilterConfig->SourcePort = READ_BIT(l4filterval, ETH_MACL4AR_L4SP);
  
  return HAL_OK;
}

/**
  * @brief  Configures the L3 Filter, this function allow to:
  *         set the layer 3 protocol to be matched (IPv4 or IPv6)
  *         enable/disable L3 source/destination port perfect/inverse match.      
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  Filter: L3 filter to configured, this parameter must be one of the following
  *           ETH_L3_FILTER_0
  *           ETH_L3_FILTER_1          
  * @param  pL3FilterConfig: pointer to a ETH_L3FilterConfigTypeDef structure 
  *         that contains L3 filter configuration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_SetL3FilterConfig(ETH_HandleTypeDef *heth, uint32_t Filter, ETH_L3FilterConfigTypeDef *pL3FilterConfig)
{
  __IO uint32_t *l3filteraddr;
  uint32_t l3filterval = 0;
  
  if(pL3FilterConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  l3filteraddr = (__IO uint32_t *)(&(heth->Instance->MACL3L4C0R) + Filter);
 
  l3filterval = (pL3FilterConfig->Protocol | 
                 pL3FilterConfig->SrcAddrFilterMatch |
                   pL3FilterConfig->DestAddrFilterMatch |
                     (pL3FilterConfig->SrcAddrHigherBitsMatch << 6) | 
                       (pL3FilterConfig->DestAddrHigherBitsMatch << 11));
  
  /* Write configuration to (MACL3L4C0R + filter )register */
  MODIFY_REG(*l3filteraddr, ETH_MACL3CR_MASK, l3filterval);
  
  /* Check if IPv6 protocol is selected */
  if(pL3FilterConfig->Protocol)
  {
    /* Set the IPv6 address match */
    /* Set Bits[31:0] of 128-bit IP addr */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A0R0R) + Filter)) = pL3FilterConfig->Ip6Addr[0];
    /* Set Bits[63:32] of 128-bit IP addr */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A1R0R) + Filter)) = pL3FilterConfig->Ip6Addr[1];
    /* update Bits[95:64] of 128-bit IP addr */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A2R0R) + Filter)) = pL3FilterConfig->Ip6Addr[2];
    /* update Bits[127:96] of 128-bit IP addr */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A3R0R) + Filter)) = pL3FilterConfig->Ip6Addr[3];
  }
  else /* IPv4 protocol is selected */
  {
    /* Set the IPv4 source address match */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A0R0R) + Filter)) = pL3FilterConfig->Ip4SrcAddr;
    /* Set the IPv4 destination address match */
    *((__IO uint32_t *)(&(heth->Instance->MACL3A1R0R) + Filter)) = pL3FilterConfig->Ip4DestAddr;
  }
  
  return HAL_OK;
}

/**
  * @brief  Configures the L3 Filter, this function allow to:
  *         set the layer 3 protocol to be matched (IPv4 or IPv6)
  *         enable/disable L3 source/destination port perfect/inverse match.      
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  Filter: L3 filter to configured, this parameter must be one of the following
  *           ETH_L3_FILTER_0
  *           ETH_L3_FILTER_1          
  * @param  pL3FilterConfig: pointer to a ETH_L3FilterConfigTypeDef structure 
  *         that will contain the L3 filter configuration. 
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_GetL3FilterConfig(ETH_HandleTypeDef *heth, uint32_t Filter, ETH_L3FilterConfigTypeDef *pL3FilterConfig)
{
  uint32_t l3filterval = *((__IO uint32_t *)(&(heth->Instance->MACL3L4C0R) + Filter));
  
  if(pL3FilterConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  pL3FilterConfig->Protocol = READ_BIT(l3filterval, ETH_MACL3L4CR_L3PEN);
  pL3FilterConfig->SrcAddrFilterMatch = READ_BIT(l3filterval, (ETH_MACL3L4CR_L3SAM | ETH_MACL3L4CR_L3SAIM));
  pL3FilterConfig->DestAddrFilterMatch = READ_BIT(l3filterval, (ETH_MACL3L4CR_L3DAM | ETH_MACL3L4CR_L3DAIM));
  pL3FilterConfig->SrcAddrHigherBitsMatch = (READ_BIT(l3filterval, ETH_MACL3L4CR_L3HSBM) >> 6);
  pL3FilterConfig->DestAddrHigherBitsMatch = (READ_BIT(l3filterval, ETH_MACL3L4CR_L3HDBM) >> 11);
  
  if(pL3FilterConfig->Protocol)
  {
    pL3FilterConfig->Ip6Addr[0] = *((__IO uint32_t *)(&(heth->Instance->MACL3A0R0R) + Filter));
    pL3FilterConfig->Ip6Addr[1] = *((__IO uint32_t *)(&(heth->Instance->MACL3A1R0R) + Filter));
    pL3FilterConfig->Ip6Addr[2] = *((__IO uint32_t *)(&(heth->Instance->MACL3A2R0R) + Filter));
    pL3FilterConfig->Ip6Addr[3] = *((__IO uint32_t *)(&(heth->Instance->MACL3A3R0R) + Filter));		
  }
  else
  {
    pL3FilterConfig->Ip4SrcAddr = *((__IO uint32_t *)(&(heth->Instance->MACL3A0R0R) + Filter)); 
    pL3FilterConfig->Ip4DestAddr = *((__IO uint32_t *)(&(heth->Instance->MACL3A1R0R) + Filter));	
  }
  
  return HAL_OK; 
}

/**
  * @brief  Enables L3 and L4 filtering process.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module       
  * @retval None.
  */
void HAL_ETHEx_EnableL3L4Filtering(ETH_HandleTypeDef *heth)
{
  /* Enable L3/L4 filter */
  SET_BIT(heth->Instance->MACPFR, ETH_MACPFR_IPFE);
}

/**
  * @brief  Disables L3 and L4 filtering process.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module       
  * @retval None.
  */
void HAL_ETHEx_DisableL3L4Filtering(ETH_HandleTypeDef *heth)
{
  /* Disable L3/L4 filter */
  CLEAR_BIT(heth->Instance->MACPFR, ETH_MACPFR_IPFE);  
}

/**
  * @brief  Get the VLAN Configuration for Receive Packets.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module 
  * @param  pVlanConfig: pointer to a ETH_RxVLANConfigTypeDef structure 
  *         that will contain the VLAN filter configuration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_GetRxVLANConfig(ETH_HandleTypeDef *heth, ETH_RxVLANConfigTypeDef *pVlanConfig)
{
  if(pVlanConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  pVlanConfig->InnerVLANTagInStatus = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_EIVLRXS) >> 31);
  pVlanConfig->StripInnerVLANTag  = READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_EIVLS);
  pVlanConfig->InnerVLANTag = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_ERIVLT) >> 27);  
  pVlanConfig->DoubleVLANProcessing = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_EDVLP) >> 26);  
  pVlanConfig->VLANTagHashTableMatch = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_VTHM) >> 25); 
  pVlanConfig->VLANTagInStatus = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_EVLRXS) >> 24);
  pVlanConfig->StripVLANTag = READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_EVLS); 
  pVlanConfig->VLANTypeCheck = READ_BIT(heth->Instance->MACVTR, (ETH_MACVTR_DOVLTC | ETH_MACVTR_ERSVLM | ETH_MACVTR_ESVL));
  pVlanConfig->VLANTagInverceMatch = (FunctionalState)(READ_BIT(heth->Instance->MACVTR, ETH_MACVTR_VTIM) >> 17); 
               
  return HAL_OK;
}

/**
  * @brief  Set the VLAN Configuration for Receive Packets. 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module   
  * @param  pVlanConfig: pointer to a ETH_RxVLANConfigTypeDef structure 
  *         that contains VLAN filter configuration.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ETHEx_SetRxVLANConfig(ETH_HandleTypeDef *heth, ETH_RxVLANConfigTypeDef *pVlanConfig)
{
  uint32_t rxvlanval = 0;
  
  if(pVlanConfig == NULL)
  {
    return HAL_ERROR;
  }
 
  rxvlanval = ((uint32_t)(pVlanConfig->InnerVLANTagInStatus << 31) |
               pVlanConfig->StripInnerVLANTag |
                 (uint32_t)(pVlanConfig->InnerVLANTag << 27) |
                   (uint32_t)(pVlanConfig->DoubleVLANProcessing << 26) |
                     (uint32_t)(pVlanConfig->VLANTagHashTableMatch << 25) |
                       (uint32_t)(pVlanConfig->VLANTagInStatus << 24) |
                         pVlanConfig->StripVLANTag |
                           pVlanConfig->VLANTypeCheck |
                             (uint32_t)(pVlanConfig->VLANTagInverceMatch << 17));
  
  /* Write config to MACVTR */
  MODIFY_REG(heth->Instance->MACVTR, ETH_MACRXVLAN_MASK, rxvlanval);

  return HAL_OK;
}

/**
  * @brief  Set the VLAN Hash Table 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module    
  * @param  VLANHashTable: VLAN hash table 16 bit value
  * @retval None
  */
void HAL_ETHEx_SetVLANHashTable(ETH_HandleTypeDef *heth, uint32_t VLANHashTable)
{
  MODIFY_REG(heth->Instance->MACVHTR, ETH_MACVHTR_VLHT, VLANHashTable);       
}

/**
  * @brief  Get the VLAN Configuration for Transmit Packets.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module  
  * @param  VLANTag: Selects the vlan tag, this parameter must be one of the following
  *                 ETH_OUTER_TX_VLANTAG
  *                 ETH_INNER_TX_VLANTAG
  * @param  pVlanConfig: pointer to a ETH_TxVLANConfigTypeDef structure 
  *         that will contain the Tx VLAN filter configuration.
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_ETHEx_GetTxVLANConfig(ETH_HandleTypeDef *heth, uint32_t VLANTag ,ETH_TxVLANConfigTypeDef *pVlanConfig)
{
  uint32_t vlanreg;
  
  if (pVlanConfig == NULL)
  {
    return HAL_ERROR;
  }
  
  if(VLANTag == ETH_INNER_TX_VLANTAG)
  {
    vlanreg = heth->Instance->MACIVIR;
  }
  else
  {
    vlanreg = heth->Instance->MACVIR;
  }
  
  
  pVlanConfig->SourceTxDesc = (FunctionalState)(READ_BIT(vlanreg, ETH_MACVIR_VLTI) >> 20);
  pVlanConfig->SVLANType = (FunctionalState)(READ_BIT(vlanreg, ETH_MACVIR_CSVL) >> 19); 
  pVlanConfig->VLANTagControl = READ_BIT(vlanreg, (ETH_MACVIR_VLP | ETH_MACVIR_VLC));
  
  return HAL_OK;;
}

/**
  * @brief  Set the VLAN Configuration for Transmit Packets. 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module     
  * @param  VLANTag: Selects the vlan tag, this parameter must be one of the following
  *                 ETH_OUTER_TX_VLANTAG
  *                 ETH_INNER_TX_VLANTAG
  * @param  pVlanConfig: pointer to a ETH_TxVLANConfigTypeDef structure 
  *         that contains Tx VLAN filter configuration.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_ETHEx_SetTxVLANConfig(ETH_HandleTypeDef *heth, uint32_t VLANTag ,ETH_TxVLANConfigTypeDef *pVlanConfig)
{
  uint32_t vlanconfigval = 0;
  uint32_t vlanreg;
  
  if(VLANTag == ETH_INNER_TX_VLANTAG)
  {
    vlanreg = (uint32_t)&(heth->Instance->MACIVIR);
    /* Enable Double VLAN processing */
    SET_BIT(heth->Instance->MACVTR, ETH_MACVTR_EDVLP);
  }
  else
  {
    vlanreg = (uint32_t)&(heth->Instance->MACVIR);
  }
  
  vlanconfigval = ((uint32_t)(pVlanConfig->SourceTxDesc << 20) |
                   (uint32_t)(pVlanConfig->SVLANType << 19) |
                     pVlanConfig->VLANTagControl);
  
  MODIFY_REG(*((uint32_t *)vlanreg), ETH_MACTXVLAN_MASK, vlanconfigval);
  
  return HAL_OK;
}

/**
  * @brief  Set the VLAN Tag Identifier for Transmit Packets. 
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module     
  * @param  VLANTag: Selects the vlan tag, this parameter must be one of the following
  *                 ETH_OUTER_TX_VLANTAG
  *                 ETH_INNER_TX_VLANTAG
  * @param  VLANIdentifier: VLAN Identifier 16 bit value
  * @retval None
  */
void HAL_ETHEx_SetTxVLANIdentifier(ETH_HandleTypeDef *heth, uint32_t VLANTag ,uint32_t VLANIdentifier)
{
  uint32_t vlanreg;
  
  if(VLANTag == ETH_INNER_TX_VLANTAG)
  {
    vlanreg = (uint32_t)&(heth->Instance->MACIVIR);
  }
  else
  {
    vlanreg = (uint32_t)&(heth->Instance->MACVIR);
  }
  
  MODIFY_REG(*((uint32_t *)vlanreg), ETH_MACVIR_VLT, VLANIdentifier);
}

/**
  * @brief  Enables the VLAN Tag Filtering process.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module       
  * @retval None.
  */
void HAL_ETHEx_EnableVLANProcessing(ETH_HandleTypeDef *heth)
{
  /* Enable VLAN processing */
  SET_BIT(heth->Instance->MACPFR, ETH_MACPFR_VTFE);
}

/**
  * @brief  Disables the VLAN Tag Filtering process.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module       
  * @retval None.
  */
void HAL_ETHEx_DisableVLANProcessing(ETH_HandleTypeDef *heth)
{
  /* Disable VLAN processing */
  CLEAR_BIT(heth->Instance->MACPFR, ETH_MACPFR_VTFE);  
}

/**
  * @brief  Enters the Low Power Idle (LPI) mode
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @param  TxAutomate: Enable/Disbale automate enter/exit LPI mode.
  * @param  TxClockStop: Enable/Disbale Tx clock stop in LPI mode.
  * @retval None
  */
void HAL_ETHEx_EnterLPIMode(ETH_HandleTypeDef *heth, FunctionalState TxAutomate, FunctionalState TxClockStop)
{
  uint32_t lpiconfig;
  
  lpiconfig = (((uint32_t)TxAutomate << 19) | 
               ((uint32_t)TxClockStop << 21) |
                 ETH_MACLCSR_LPIEN);

  /* Enable LPI Interrupts */
  __HAL_ETH_MAC_ENABLE_IT(heth, ETH_MACIER_LPIIE);
  
  /* Write to LPI Control register: Enter low power mode */
  MODIFY_REG(heth->Instance->MACLCSR, (ETH_MACLCSR_LPIEN | ETH_MACLCSR_LPITXA | ETH_MACLCSR_LPITCSE), lpiconfig);
}

/**
  * @brief  Exits the Low Power Idle (LPI) mode.
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETHEx_ExitLPIMode(ETH_HandleTypeDef *heth)
{
  /* Clear the LPI Config and exit low power mode */
  CLEAR_BIT(heth->Instance->MACLCSR, (ETH_MACLCSR_LPIEN | ETH_MACLCSR_LPITXA | ETH_MACLCSR_LPITCSE));
  
  /* Enable LPI Interrupts */
  __HAL_ETH_MAC_DISABLE_IT(heth, ETH_MACIER_LPIIE);
}


/**
  * @brief  Returns the ETH MAC LPI event
  * @param  heth: pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval ETH MAC WakeUp event
  */
uint32_t HAL_ETHEx_GetMACLPIEvent(ETH_HandleTypeDef *heth)
{
  return heth->MACLPIEvent;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_ETH_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
