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
 * Manages DCT writing to on chip flash
 *
 *  Bootloader DOES NOT update DCT layout
 *  Only a new Application will update the DCT to the new layout.
 *  It is done automagically in this file!
 *
 *  Any new platform_dct_data_h will have the original Bootloader DCT layout for
 platform_dct_header_t
 *  Other sub-structures will be based on usage
 *      Some sub-structs were optional, all are now mandatory
 *      Optional sub-structure use is defined in the field
 *          platform_dct_version_t::wiced_dct_config_flag_t data_dct_usage_flags
 *
 *  When building an Update Application for SDK-3.5.2, 3.6.0, 3.6.1, 3.6.2
 *      CRC and sequence numbers are in the platform_dct_header_t (platform_dct_header_sdk_3__2_t)
 *          and must be valid in that structure for the bootloader from the original SDK build
 *
 * All other Update builds (and new SDK-3.6.4+ builds), these fields are in the platform_dct_version_t structure
 *
 * The Bootloader from the original SDK build will preserve all fields *after* the platform_dct_header_t structure
 *      when ever it writes a new DCT.
 *
 */
#include "string.h"
#include "wwd_assert.h"
#include "wiced_result.h"
#include "platform_dct.h"
#include "waf_platform.h"
#include "wiced_framework.h"
#if !defined(NO_WICED_API)
#include "wiced_rtos.h"
#endif
#include "wiced_dct_common.h"
#include "wiced_apps_common.h"
#include "platform_mcu_peripheral.h"
#include "platform_peripheral.h"
#include "platform_ocf.h"
#include "platform_config.h"
/******************************************************
 *                      Macros
 ******************************************************/

#ifndef SECTOR_SIZE
#define SECTOR_SIZE ((uint32_t)(4096))
#endif

#ifndef DCT_CRC32_CALCULATION_SIZE_ON_STACK
#define DCT_CRC32_CALCULATION_SIZE_ON_STACK     128
#endif

#define OCF_BUFFER_SIZE 4

/******************************************************
 *                    Constants
 ******************************************************/

/* PLATFORM_DCT_COPY1_END_ADDRESS defined in waf_platform.h */
#ifndef PLATFORM_DCT_COPY1_SIZE
#define PLATFORM_DCT_COPY1_SIZE (PLATFORM_DCT_COPY1_END_ADDRESS - PLATFORM_DCT_COPY1_START_ADDRESS)
#endif

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
 *               Function Declarations
 ******************************************************/
/*
 * Get the CRC value for the data range in FLASH
 *
 * NOTE: the dct_start_addr MUST be the start of the DCT, otherwise the structure will not be lined up !
 * we need to read the entire DCT area, but make sure we set the value of the crc32 field
 * in platform_dct_header_t to 0x00 while figuring the CRC
 *
 * NOTE: We cannot malloc a buffer in Bootloader
 *       Define in bootloader.mk or ota2_bootloader.mk
 *  bootloader.mk:
 *      NoOS_START_STACK   := 16000
 *      GLOBAL_DEFINES     += DCT_CRC32_CALCULATION_SIZE_ON_STACK=1024          #128
 *
 *  ota2_bootloader.mk
 *      # stack needs to be big enough to handle the CRC32 calculation buffer
 *      ifeq ($(filter $(PLATFORM),BCM943909WCD1_3.B0 BCM943909WCD1_3.B1 BCM943909B0FCBU BCM943907WAE_1.B0 BCM943907WAE_1.B1 BCM943907APS.B0 BCM943907APS.B1 BCM943907AEVAL2F_1.B0 BCM943907AEVAL2F_1.B1 BCM943907AEVAL1F_1.B0 BCM943907AEVAL1F_1.B1),)
 *      NoOS_START_STACK   := 16000
 *      GLOBAL_DEFINES     += DCT_CRC32_CALCULATION_SIZE_ON_STACK=256
 *      else
 *      NoOS_START_STACK   := 20000
 *      GLOBAL_DEFINES     += DCT_CRC32_CALCULATION_SIZE_ON_STACK=4096
 *      endif
 *
 *  ota2_example.mk:
 *      GLOBAL_DEFINES     += APPLICATION_STACK_SIZE=16000
 *      # stack needs to be big enough to handle the CRC32 calculation buffer
 *      GLOBAL_DEFINES     += DCT_CRC32_CALCULATION_SIZE_ON_STACK=4096
 *
 *       crc calculation times based on size of temp buffer
 *     Buff Size       ms
 *       64           256
 *      256            64
 *     2048             8
 *     4096             4
 *
 * @param   dct_start_addr  - start of range of FLASH to generate CRC
 * @param   dct_end_addr    - end of range of FLASH to generate CRC (this addr is one byte beyond the crc generation)
 * @param   header_in_range - WICED_DCT_HEADER_IN_RANGE if the dct_header and/or the dct_version are in the stated memory range
 * @param   crc_start       - cumulative CRC (or CRC32_INIT_VALUE)
 *
 * returns calculated CRC
 */
static uint32_t wiced_dct_generate_crc_from_flash( uint32_t dct_start_addr, uint32_t dct_end_addr, wiced_dct_header_in_address_range_t header_in_range, uint32_t crc_start );
static uint32_t wiced_dct_get_crc_from_flash( uint32_t dct_start_addr, uint32_t dct_end_addr, wiced_dct_header_in_address_range_t header_in_range, uint32_t crc_start );
static wiced_result_t wiced_dct_copy_ocf( uint32_t dest_loc, uint32_t src_loc, uint32_t size );

extern int32_t ef_read_buffer_in_byte( uint32_t offset, uint8_t* buffer, uint32_t length );
extern int32_t ef_write_buffer_in_word( uint32_t offset, uint8_t* buffer, uint32_t length );
extern int32_t ef_erase_multiple_pages( uint32_t offset, uint32_t length );

extern wiced_result_t wiced_hal_eflash_write( uint32_t offset, uint8_t* p_buffer, uint32_t length );
extern wiced_result_t wiced_hal_eflash_read( uint32_t offset, uint8_t* p_buffer, uint32_t length );
extern wiced_result_t wiced_hal_eflash_erase( uint32_t offset, uint32_t length );

/******************************************************
 *               Variables Definitions
 ******************************************************/
static const uint32_t DCT_section_offsets[ ] =
{ [DCT_APP_SECTION] = sizeof(platform_dct_data_t),
  [DCT_SECURITY_SECTION]        = OFFSETOF( platform_dct_data_t, security_credentials ),
  [DCT_MFG_INFO_SECTION]        = OFFSETOF( platform_dct_data_t, mfg_info ),
  [DCT_WIFI_CONFIG_SECTION]     = OFFSETOF( platform_dct_data_t, wifi_config ),
  [DCT_ETHERNET_CONFIG_SECTION] = OFFSETOF( platform_dct_data_t, ethernet_config ),
  [DCT_NETWORK_CONFIG_SECTION]  = OFFSETOF( platform_dct_data_t, network_config ),

  /* these were optional before SDK-3.6.4 */
  [DCT_BT_CONFIG_SECTION]       = OFFSETOF( platform_dct_data_t, bt_config ),
  [DCT_P2P_CONFIG_SECTION]      = OFFSETOF( platform_dct_data_t, p2p_config ),
  [DCT_OTA2_CONFIG_SECTION]     = OFFSETOF( platform_dct_data_t, ota2_config ),

  [DCT_VERSION_SECTION]         = OFFSETOF( platform_dct_data_t, dct_version ),
  [DCT_MISC_SECTION]            = OFFSETOF( platform_dct_data_t, dct_misc ),
  [DCT_INTERNAL_SECTION] = 0,
  };

#if !defined(BOOTLOADER)
static uint32_t curr_verified_dct_addr = (uint32_t)GET_CURRENT_ADDRESS_FAILED;        /* filled in after verifying current DCT, changed after a write */
#endif


/******************************************************
 *               Function Definitions
 ******************************************************/



/*
 * NOTE:
 */
static uint32_t wiced_dct_generate_crc_from_flash( uint32_t dct_start_addr, uint32_t dct_end_addr, wiced_dct_header_in_address_range_t header_in_range, uint32_t crc_start )
{
    /* NOTE: the dct_start_addr MUST be the start of the DCT, otherwise the structure will not be lined up !
     * we need to read the entire DCT area, but make sure we set the value of the crc32 field
     * in platform_dct_header_t to 0x00 while figuring the CRC
     */

    /* cannot malloc a buffer in Bootloader *
     *
     * NOTE: crc calculation times based on size of temp buffer
     *     Buff Size       ms
     *       64           256
     *      256            64
     *     2048             8
     *     4096             4
     */
    uint8_t temp_buff[ DCT_CRC32_CALCULATION_SIZE_ON_STACK ];
    uint32_t chunk_size;
    uint32_t calculated_dct_crc32_value;
    uint32_t curr_addr;
    uint32_t crc_offset;

    crc_offset = 0;

    if ( header_in_range == WICED_DCT_CRC_IN_HEADER )
    {
#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
        crc_offset = OFFSETOF(platform_dct_data_t, dct_header) + OFFSETOF(platform_dct_header_t, crc32);
#endif
    }
    else if ( header_in_range == WICED_DCT_CRC_IN_VERSION )
    {
        /* if DCT is from an older SDK, and we haven't updated to latest,
         * this will not be right. Which is OK, because the older SDK doesn't have a version structure
         * However, if we have updated the DCT to the current SDK, this will be correct.
         */
        crc_offset = OFFSETOF(platform_dct_data_t, dct_version) + OFFSETOF( platform_dct_version_t, crc32 );
    }

    curr_addr = dct_start_addr;
    calculated_dct_crc32_value = crc_start;
    while ( curr_addr < dct_end_addr )
    {
        chunk_size = dct_end_addr - curr_addr;
        if ( chunk_size > sizeof( temp_buff ) )
        {
            chunk_size = sizeof( temp_buff );
        }

        if ( ocf_buffer_read( curr_addr, temp_buff, chunk_size ) != 0 )
        {
            return 0;
        }
        if ( header_in_range != WICED_DCT_CRC_NOT_IN_RANGE )
        {
            if ( ( ( curr_addr - dct_start_addr ) < crc_offset ) &&
               ( ( ( curr_addr - dct_start_addr ) + chunk_size ) > ( crc_offset + sizeof(uint32_t) ) ) )
            {
                uint32_t* crc_ptr;

                /* we want the crc32 value to be 0x00 when computing the value! */
                crc_ptr = (uint32_t*) ( &temp_buff[ ( crc_offset - ( curr_addr - dct_start_addr ) ) & ( (uint32_t) DCT_CRC32_CALCULATION_SIZE_ON_STACK - (uint32_t) 1 ) ] );
                *crc_ptr = 0;

                header_in_range = WICED_DCT_CRC_NOT_IN_RANGE;
            }
        }

        /* compute the crc */
        calculated_dct_crc32_value = crc32( temp_buff, chunk_size, calculated_dct_crc32_value );
        curr_addr += chunk_size;
    }

    return calculated_dct_crc32_value;
}

uint32_t wiced_dct_get_crc_from_flash( uint32_t dct_start_addr, uint32_t dct_end_addr, wiced_dct_header_in_address_range_t header_in_range, uint32_t crc_start )
{
    CRC_TYPE crc32;

    crc32 = wiced_dct_generate_crc_from_flash( dct_start_addr, dct_end_addr, header_in_range, crc_start );

    return crc32;
}

/*
 * Validate the DCT and determine the DCT's SDK version
 *
 *                      - device_address (DCT1 or DCT2 COPY FLASH address)
 *
 * returns  valid DCT:      the SDK version of platform_dct_version_t
 *          invalid DCT:    DCT_BOOTLOADER_SDK_UNKNOWN
 */
wiced_dct_sdk_ver_t wiced_dct_validate_and_determine_version( uint32_t device_start_address, uint32_t device_end_address, int* initial_write, int* sequence )
{
    platform_dct_header_t dct_header;
    platform_dct_version_t dct_version;
    CRC_TYPE dct_crc32;
    CRC_TYPE saved_crc32;
    wiced_dct_sdk_ver_t this_dct_version;

    /* assume bad DCT */
    this_dct_version = DCT_BOOTLOADER_SDK_UNKNOWN;

    if ( initial_write != NULL )
    {
        *initial_write = 0;
    }
    if ( sequence != NULL )
    {
        *sequence = 0;
    }

    /* read in the header - it always starts at the beginning of the DCT DATA */
    if ( ocf_buffer_read( device_start_address, (uint8_t *) &dct_header, sizeof(platform_dct_header_t) ) != 0 )
    {
        goto _version_test_done;
    }

    /* check if header makes sense */
    if ( wiced_dct_minimal_check_dct_header( &dct_header ) != WICED_SUCCESS )
    {
        goto _version_test_done;
    }

    /* header made sense - read the version struct - it ONLY works with the latest platform_dct_data_t */
    if ( ocf_buffer_read( device_start_address + (uint32_t) OFFSETOF( platform_dct_data_t, dct_version ), (uint8_t *) &dct_version, sizeof(platform_dct_version_t) ) != 0 )
    {
        goto _version_test_done;
    }

    /* while the Bootloader DCT DATA may not have the dct_version, we may have updated the DCT already
     * does the dct_version structure make sense ?
     */
    if ( wiced_dct_minimal_check_dct_version( &dct_version ) != WICED_SUCCESS )
    {
        /* no valid version struct - it is either DCT_BOOTLOADER_SDK_VERSION or UNKNOWN
         * if there is a crc in the header for this SDK, check it.
         */
#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
        if ((dct_header.initial_write == 1) && (dct_header.crc32 == 0))
        {
            /* good, return with old SDK version */
            this_dct_version = DCT_BOOTLOADER_SDK_VERSION;
            goto _version_test_done;
        }
        else
        {
            /* get a copy of the crc from the header then compute the crc */
            saved_crc32 = dct_header.crc32;

            dct_header.write_incomplete = 0; /* compute CRC with this as 0x00 */
            dct_header.crc32 = 0; /* compute CRC with this as 0x00 */
            dct_header.initial_write = 0; /* compute CRC with this as 0x00 */

            dct_crc32 = crc32( (uint8_t*)&dct_header, sizeof(platform_dct_header_t), CRC32_INIT_VALUE );
            dct_crc32 = wiced_dct_generate_crc_from_flash( device_start_address + OFFSETOF(platform_dct_data_t, dct_header) + sizeof(platform_dct_header_t),
                    device_end_address, WICED_DCT_CRC_NOT_IN_RANGE, dct_crc32 );
            if (dct_crc32 == saved_crc32)
            {
                /* good, return with old SDK version */
                this_dct_version = DCT_BOOTLOADER_SDK_VERSION;
                goto _version_test_done;
            }
        }
#else
        this_dct_version = DCT_BOOTLOADER_SDK_VERSION;
        goto _version_test_done;
#endif
    }
    else
    {
        /* we have a minimally valid version struct - so it's either CURRENT or UNKNOWN
         * check for CRC (if not in header, old SDK & current have in dct_version struct)
         */
#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
        if ((dct_header.initial_write == 1) && (dct_header.crc32 == 0))
        {
            /* good, return with current SDK version */
            if (initial_write != NULL)
            {
                *initial_write = (int)dct_header.initial_write;
            }
            if (sequence != NULL)
            {
                *sequence = (int)dct_header.sequence;
            }
            this_dct_version = DCT_BOOTLOADER_SDK_CURRENT;
            goto _version_test_done;
        }
        else
        {
            /* get a copy of the crc from the header then compute the crc */
            saved_crc32 = dct_header.crc32;

            dct_header.write_incomplete = 0; /* compute CRC with this as 0x00 */
            dct_header.crc32 = 0; /* compute CRC with this as 0x00 */
            dct_header.initial_write = 0; /* compute CRC with this as 0x00 */

            dct_crc32 = crc32( (uint8_t*)&dct_header, sizeof(platform_dct_header_t), CRC32_INIT_VALUE );
            dct_crc32 = wiced_dct_generate_crc_from_flash( device_start_address + OFFSETOF(platform_dct_data_t, dct_header) + sizeof(platform_dct_header_t),
                    device_end_address, WICED_DCT_CRC_NOT_IN_RANGE, dct_crc32 );
            if (dct_crc32 == saved_crc32)
            {
                /* good, return with current SDK version */
                if (initial_write != NULL)
                {
                    *initial_write = (int)dct_header.initial_write;
                }
                if (sequence != NULL)
                {
                    *sequence = (int)dct_header.sequence;
                }
                this_dct_version = DCT_BOOTLOADER_SDK_CURRENT;
                goto _version_test_done;
            }
        }
#else
        /* CRC is in version struct check it! */
        if ( ( dct_version.initial_write == 1 ) && ( dct_version.crc32 == 0 ) )
        {
            /* good, return with current SDK version */
            if ( initial_write != NULL )
            {
                *initial_write = (int) dct_version.initial_write;
            }
            if ( sequence != NULL )
            {
                *sequence = dct_version.sequence;
            }
            this_dct_version = dct_version.version;
            goto _version_test_done;
        }
        else
        {
            /* get a copy of the crc from the header then compute the crc */
            saved_crc32 = dct_version.crc32;
            dct_header.write_incomplete = 0; /* compute CRC with this as 0x00 */
            dct_version.crc32 = 0; /* compute CRC with this as 0x00 */
            dct_version.initial_write = 0; /* compute CRC with this as 0x00 */

            dct_crc32 = crc32( (uint8_t*) &dct_header, sizeof(platform_dct_header_t), CRC32_INIT_VALUE );
            dct_crc32 = wiced_dct_generate_crc_from_flash( device_start_address + OFFSETOF( platform_dct_data_t, dct_header ) + sizeof(platform_dct_header_t), device_start_address + OFFSETOF( platform_dct_data_t, dct_version ), WICED_DCT_CRC_NOT_IN_RANGE, dct_crc32 );
            dct_crc32 = crc32( (uint8_t*) &dct_version, sizeof(platform_dct_version_t), dct_crc32 );
            dct_crc32 = wiced_dct_generate_crc_from_flash( device_start_address + OFFSETOF( platform_dct_data_t, dct_version ) + sizeof(platform_dct_version_t), device_end_address, WICED_DCT_CRC_NOT_IN_RANGE, dct_crc32 );
            if ( dct_crc32 == saved_crc32 )
            {
                /* good, return with current SDK version */
                if ( initial_write != NULL )
                {
                    *initial_write = (int) dct_version.initial_write;
                }
                if ( sequence != NULL )
                {
                    *sequence = dct_version.sequence;
                }
                this_dct_version = dct_version.version;
                goto _version_test_done;
            }
        }
#endif
    }

_version_test_done:

    return this_dct_version;
}

/**
 * new_dct          - ptr to store the address of the new DCT in FLASH
 * new_dct_header   - ptr to dct_header structure to build
 * new_dct_version  - ptr to dct_version structure to build
 *
 * This function erases the non-current DCT (places address in *new_dct)
 */

static wiced_result_t wiced_dct_start_new_from_current( uint32_t *new_dct, platform_dct_header_t* new_dct_header, platform_dct_version_t* new_dct_version )
{
    uint32_t curr_dct;
    platform_dct_header_t curr_dct_header;
    platform_dct_version_t curr_dct_version;
    int ret;

    if ( ( new_dct == NULL ) || ( new_dct_header == NULL ) || ( new_dct_version == NULL ) )
    {
        return WICED_ERROR;
    }

    curr_dct = (uint32_t) wiced_dct_get_current_address( DCT_INTERNAL_SECTION );
    if ( curr_dct == (uint32_t) GET_CURRENT_ADDRESS_FAILED )
    {
        return WICED_ERROR;
    }

    /* Erase the non-current DCT */
    *new_dct = ( curr_dct == PLATFORM_DCT_COPY1_START_ADDRESS ) ?
               PLATFORM_DCT_COPY2_START_ADDRESS : PLATFORM_DCT_COPY1_START_ADDRESS;

    /* erase new DCT area */
    if ( wiced_dct_erase_non_current_dct( *new_dct ) != WICED_SUCCESS )
    {
        /* wiced_dct_erase_non_current_dct() failed for */
        return WICED_ERROR;
    }

    /* read the header and version from the current DCT */
    ret = ocf_buffer_read( curr_dct, (uint8_t*) &curr_dct_header, sizeof( curr_dct_header ) );
    ret |= ocf_buffer_read( curr_dct + (uint32_t) OFFSETOF( platform_dct_data_t, dct_version ), (uint8_t*) &curr_dct_version, sizeof( curr_dct_version ) );
    if ( ret != 0 )
    {
        return WICED_ERROR;
    }

    /* We know curr_dct is valid, copy current dct_header info */
    memcpy( new_dct_header, &curr_dct_header, sizeof(platform_dct_header_t) );

    /* check if the version data is valid - this may be an upgrade */
    memset( new_dct_version, 0x00, sizeof(platform_dct_version_t) );
    if ( curr_dct_version.magic_number == DCT_VERSION_MAGIC_NUMBER )
    {
        memcpy( new_dct_version, &curr_dct_version, sizeof(platform_dct_version_t) );
    }

    return WICED_SUCCESS;
}

/*
 * Finish writing the new DCT.
 *
 * sequence:
 * - copy platform_dct_data_t EXCEPT for the dct_version and dct_header
 * - compute CRC
 * - write dc_version
 * - write dc_header
 */
static wiced_result_t wiced_dct_finish_new_dct( uint32_t new_dct, platform_dct_header_t* new_dct_header, platform_dct_version_t* new_dct_version )
{
    uint32_t new_crc32;
    uint8_t  zero_byte = 0;
    uint32_t curr_dct = ( new_dct == PLATFORM_DCT_COPY1_START_ADDRESS ) ? PLATFORM_DCT_COPY2_START_ADDRESS : PLATFORM_DCT_COPY1_START_ADDRESS;

    /* generate the new CRC value
     * save these values - we need to calculate the CRC with them as 0x00
     */
    new_dct_header->write_incomplete = 0; /* compute CRC with this as 0x00 */
#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
    new_dct_header->crc32 = 0; /* compute CRC with this as 0x00 */
    new_dct_header->sequence = new_dct_header->sequence + 1;
    new_dct_header->initial_write = 0; /* compute CRC with this as 0x00 */
#else
    new_dct_header->is_current_dct = 1; /* we want to be current */
#endif
    new_dct_version->crc32 = 0; /* compute CRC with this as 0x00 */
    new_dct_version->initial_write = 0; /* compute CRC with this as 0x00 */
    new_dct_version->magic_number = DCT_VERSION_MAGIC_NUMBER;
    new_dct_version->data_dct_usage_flags = WICED_DCT_CONFIG_FLAGS;
    new_dct_version->version = DCT_BOOTLOADER_SDK_CURRENT;
    new_dct_version->sequence++; /* always increment the sequence */

    new_crc32 = crc32( (uint8_t*) new_dct_header, sizeof(platform_dct_header_t), CRC32_INIT_VALUE );
    new_crc32 = wiced_dct_generate_crc_from_flash( new_dct + OFFSETOF( platform_dct_data_t, dct_header ) + sizeof(platform_dct_header_t),
                                                   new_dct + OFFSETOF( platform_dct_data_t, dct_version ),
                                                               WICED_DCT_CRC_NOT_IN_RANGE, new_crc32 );
    new_crc32 = crc32( (uint8_t*) new_dct_version, sizeof(platform_dct_version_t), new_crc32 );
    new_crc32 = wiced_dct_generate_crc_from_flash( new_dct + OFFSETOF( platform_dct_data_t, dct_version ) + sizeof(platform_dct_version_t),
                                                    new_dct + PLATFORM_DCT_COPY1_SIZE,
                                                               WICED_DCT_CRC_NOT_IN_RANGE, new_crc32 );
#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
    new_dct_header->crc32 = new_crc32;
#else
    new_dct_version->crc32 = new_crc32;
#endif

    new_dct_header->write_incomplete = 1;  /* set to 1 so we know write was incomplete until we clear it
                                            * Note that the crc will be incorrect until this gets written to 0x00
                                            * which is the last thing we write.
                                            */
    /*
     * writing a single byte is not supported in OCF.
     * so making write_incomplete = 0, while writing the header
     */
    new_dct_header->write_incomplete = 0;

    /* Write the new DCT header & version data. */
    if ( ( ocf_buffer_write( new_dct + OFFSETOF( platform_dct_data_t, dct_header ), (uint8_t*) new_dct_header, sizeof(platform_dct_header_t) ) != 0 ) ||
          ( ocf_buffer_write( new_dct + OFFSETOF( platform_dct_data_t, dct_version ), (uint8_t*) new_dct_version, sizeof(platform_dct_version_t) ) != 0 ) )
    {
        return WICED_ERROR;
    }
#if 0
//    printf ("%s %d: Write the zero into write_incomplete in the new DCT \n",__func__,__LINE__);
     /* Write a zero into write_incomplete */
    if (ocf_buffer_write(new_dct + OFFSETOF(platform_dct_data_t, dct_header) + OFFSETOF(platform_dct_header_t, write_incomplete), &zero_byte, sizeof( zero_byte )) != 0)
    {
        return WICED_ERROR;
    }
#endif

#if !defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
     if ( ocf_buffer_write( curr_dct + OFFSETOF(platform_dct_data_t, dct_header) + OFFSETOF(platform_dct_header_t, is_current_dct), &zero_byte, sizeof( zero_byte )) != 0)
    {
        return WICED_ERROR;
    }
#endif

     /* Don't erase old one, we'll erase it right before we create a new one.
      * But we do need to write the zero into initial_write in the old DCT
      */
    if (ocf_buffer_write( curr_dct + OFFSETOF(platform_dct_data_t, dct_version) + OFFSETOF(platform_dct_version_t, initial_write), &zero_byte, sizeof( zero_byte )) != 0)
    {
        return WICED_ERROR;
    }

#if !defined(BOOTLOADER)
    /* save verified DCT location */
    curr_verified_dct_addr = new_dct;
#endif

    return WICED_SUCCESS;
}

#if !defined(BOOTLOADER)
/*
 * dct_source       - Start address of the DCT to read from to update to current SDK's DCT layout
 * dct_destination  - Start address of the DCT to write current SDK's DCT layout
 */
static wiced_result_t wiced_dct_external_dct_update( uint32_t dct_destination, uint32_t dct_source )
{
    wiced_result_t result = WICED_ERROR;

    /* dct_header will ALWAYS be platfrom_dct_header_t */
    platform_dct_header_t *dct_header_src, *dct_header_dst;

    platform_dct_version_t *dct_version_src, *dct_version_dst;

    uint8_t* allocated_src_buff = NULL;
    uint8_t* allocated_dst_buff = NULL;
    bootloader_dct_data_t* src_dct_buffer = NULL;
    platform_dct_data_t* dst_dct_buffer = NULL;

    if ( ( dct_source == 0 ) && ( dct_destination == 0 ) )
    {
        return WICED_BADARG;
    }

    allocated_src_buff = (uint8_t*) calloc( LARGEST_DCT_SUB_STRUCTURE_SIZE, 1 );
    allocated_dst_buff = (uint8_t*) calloc( LARGEST_DCT_SUB_STRUCTURE_SIZE, 1 );
    src_dct_buffer = (bootloader_dct_data_t*) allocated_src_buff;
    dst_dct_buffer = (platform_dct_data_t*) allocated_dst_buff;
    if ( ( allocated_src_buff == NULL ) || ( allocated_dst_buff == NULL ) )
    {
        goto _update_fail_malloc;
    }

    //WICED_DCT_MUTEX_GET();
    /* erase new DCT */
    if ( wiced_dct_erase_non_current_dct( dct_destination ) != WICED_SUCCESS )
    {
        /*failed to erase DCT - we can survive this, just continue */
    }

    platform_watchdog_kick();

    /* copy the Application's DCT data from old to new DCT in FLASH (resides after system DCT data) */
    if ( wiced_dct_copy_ocf( dct_destination + APPLICATION_DCT_DATA_SIZE, /* size of system DCT data from Applicaiton's perspective */
    dct_source + BOOTLOADER_DCT_DATA_SIZE, /* size of system DCT data from Bootloader's perspective */
    ( PLATFORM_DCT_COPY2_START_ADDRESS - PLATFORM_DCT_COPY1_START_ADDRESS ) - BIGGER_DCT_DATA_SIZE ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }

    /* Update the DCT sub structures & write them out. We do header & version last */

    /* mfg_info */
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, mfg_info ) ), (uint8_t*) src_dct_buffer, sizeof(platform_dct_mfg_info_t) ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_mfg_info_to_current( (platform_dct_mfg_info_t*) dst_dct_buffer, (platform_dct_mfg_info_t*) src_dct_buffer ) == WICED_SUCCESS )
        {
            if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, mfg_info ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_mfg_info_t) ) != WICED_SUCCESS )
            {
                goto _update_fail;
            }
        }
    }

    /* security_credentials
     * this is the largest section - separate the 3 fields into separate sections
     * private_key[ PRIVATE_KEY_SIZE ];    -- 2k
     * certificate[ CERTIFICATE_SIZE ];    -- 4k
     * cooee_key  [ COOEE_KEY_SIZE ];      -- 16
     */
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, private_key ) ), (uint8_t*) src_dct_buffer, PRIVATE_KEY_SIZE ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_security_private_key_to_current( (char*) dst_dct_buffer, (char*) src_dct_buffer ) == WICED_SUCCESS )
        {
            if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, private_key ), (uint8_t*) dst_dct_buffer, PRIVATE_KEY_SIZE ) != WICED_SUCCESS )
            {
                goto _update_fail;
            }
        }
    }
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, certificate ) ), (uint8_t*) src_dct_buffer, CERTIFICATE_SIZE ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_security_certificate_to_current( (char*) dst_dct_buffer, (char*) src_dct_buffer ) == WICED_SUCCESS )
        {
            if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, certificate ), (uint8_t*) dst_dct_buffer, CERTIFICATE_SIZE ) != WICED_SUCCESS )
            {
                goto _update_fail;
            }
        }
    }
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, cooee_key ) ), (uint8_t*) src_dct_buffer, COOEE_KEY_SIZE ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_security_cooee_key_to_current( (uint8_t*) dst_dct_buffer, (uint8_t*) src_dct_buffer ) == WICED_SUCCESS )
        {
            if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, security_credentials ) + OFFSETOF( platform_dct_security_t, cooee_key ), (uint8_t*) dst_dct_buffer, COOEE_KEY_SIZE ) != WICED_SUCCESS )
            {
                goto _update_fail;
            }
        }
    }

    /* wifi config */
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, wifi_config ) ), (uint8_t*) src_dct_buffer, sizeof(platform_dct_wifi_config_t) ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_wifi_config_to_current( (platform_dct_wifi_config_t*) dst_dct_buffer, (platform_dct_wifi_config_t*) src_dct_buffer ) == WICED_SUCCESS )
        {
            if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, wifi_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_wifi_config_t) ) != WICED_SUCCESS )
            {
                goto _update_fail;
            }
        }
    }

    /* ethernet config */
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
#ifndef BOOTLOADER_NO_ETHER_CONFIG
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, ethernet_config ) ), (uint8_t*) src_dct_buffer, sizeof(platform_dct_ethernet_config_t) ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
#endif
    if ( wiced_dct_update_ethernet_config_to_current( (platform_dct_ethernet_config_t*) dst_dct_buffer, (platform_dct_ethernet_config_t*) src_dct_buffer ) == WICED_SUCCESS )
    {
        if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, ethernet_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_ethernet_config_t) ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
#ifndef BOOTLOADER_NO_NETWORK_CONFIG
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, network_config ) ), (uint8_t*) src_dct_buffer, sizeof(bootloader_dct_network_config_t) ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
#endif
    if ( wiced_dct_update_network_config_to_current( (platform_dct_network_config_t*) dst_dct_buffer, (bootloader_dct_network_config_t*) src_dct_buffer ) == WICED_SUCCESS )
    {
        if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, network_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_network_config_t) ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
#ifndef BOOTLOADER_NO_BT_CONFIG
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, bt_config ) ), (uint8_t*) src_dct_buffer, sizeof(bootloader_dct_bt_config_t) ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
#endif
    if ( wiced_dct_update_bt_config_to_current( (platform_dct_bt_config_t*) dst_dct_buffer, (bootloader_dct_bt_config_t*) src_dct_buffer ) == WICED_SUCCESS )
    {
        if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, bt_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_bt_config_t) ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
#ifndef BOOTLOADER_NO_P2P_CONFIG
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, p2p_config ) ), (uint8_t*) src_dct_buffer, sizeof(platform_dct_p2p_config_t) ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
#endif
    if ( wiced_dct_update_p2p_config_to_current( (platform_dct_p2p_config_t*) dst_dct_buffer, (platform_dct_p2p_config_t*) src_dct_buffer ) == WICED_SUCCESS )
    {
        if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, p2p_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_p2p_config_t) ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
#ifndef BOOTLOADER_NO_OTA2_CONFIG
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, ota2_config ) ), (uint8_t*) src_dct_buffer, sizeof(bootloader_dct_ota2_config_t) ) == WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
#endif
    if ( wiced_dct_update_ota2_config_to_current( (platform_dct_ota2_config_t*) dst_dct_buffer, (bootloader_dct_ota2_config_t*) src_dct_buffer ) == WICED_SUCCESS )
    {
        if ( ocf_buffer_write( dct_destination + OFFSETOF( platform_dct_data_t, ota2_config ), (uint8_t*) dst_dct_buffer, sizeof(platform_dct_ota2_config_t) ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    /* We need both dct_header and dct_version in RAM for wiced_dct_finish_new_dct,
     * We'll use partial positioning in the allocated RAM to put the header and version
     */
    memset( src_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE );

    dct_header_src = (platform_dct_header_t*) src_dct_buffer;
    dct_header_dst = (platform_dct_header_t*) dst_dct_buffer;
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, dct_header ) ), (uint8_t*) dct_header_src, sizeof(platform_dct_header_t) ) == WICED_SUCCESS )
    {
        if ( wiced_dct_update_header_to_current( dct_header_dst, dct_header_src ) != WICED_SUCCESS )
        {
            goto _update_fail;
        }
    }

    dct_version_src = (platform_dct_version_t*) &dct_header_src[ 1 ]; /* use memory past the header info (src_dct_buffer is big enough ) */
    dct_version_dst = (platform_dct_version_t*) &dct_header_dst[ 1 ]; /* use memory past the header info (src_dct_buffer is big enough ) */
#ifndef BOOTLOADER_NO_VERSION
    if ( ocf_buffer_read( ( dct_source + OFFSETOF( bootloader_dct_data_t, dct_version ) ), (uint8_t*) dct_version_src, sizeof(platform_dct_version_t) ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#else
    memset( dct_version_src, 0x00, sizeof(platform_dct_version_t));
#endif
    if ( wiced_dct_update_version_to_current( dct_version_dst, dct_version_src ) != WICED_SUCCESS )
    {
        goto _update_fail;
    }
#ifndef BOOTLOADER_NO_MISC_CONFIG
    /* misc config */
    memset( dst_dct_buffer, 0x00, LARGEST_DCT_SUB_STRUCTURE_SIZE);
    if (ocf_buffer_read( (dct_source + OFFSETOF(bootloader_dct_data_t, dct_misc)), (uint8_t*)src_dct_buffer, sizeof(platform_dct_misc_config_t)) == WICED_SUCCESS)
    {
        if (wiced_dct_update_misc_config_to_current((platform_dct_misc_config_t*)dst_dct_buffer, (platform_dct_misc_config_t*)src_dct_buffer) == WICED_SUCCESS)
        {
            if ( ocf_buffer_write ( dct_destination + OFFSETOF(platform_dct_data_t, dct_misc), (uint8_t*)dst_dct_buffer, sizeof(platform_dct_misc_config_t) ) != WICED_SUCCESS)
            {
                goto _update_fail;
            }
        }
    }
#endif
    platform_watchdog_kick();

    /* finish the DCT update */
    result = wiced_dct_finish_new_dct( dct_destination, dct_header_dst, dct_version_dst );

_update_fail:
    //WICED_DCT_MUTEX_RELEASE();

_update_fail_malloc:

    if ( allocated_src_buff != NULL )
    {
        free( allocated_src_buff );
    }
    if ( allocated_dst_buff != NULL )
    {
        free( allocated_dst_buff );
    }

    return result;
}
#endif  /* !defined(BOOTLOADER) */

static int wiced_dct_get_initial_write(uint32_t device_start_address)
{
    int initial_write = 0;

#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
    {
        platform_dct_header_t   dct_header;
        /* read in the header - it always starts at the beginning of the DCT DATA */
        if (ocf_buffer_read(device_start_address, (uint8_t*)&dct_header, sizeof(platform_dct_header_t)) == 0)
        {
            if ((dct_header.initial_write == 1) && (dct_header.crc32 == 0))
            {
                initial_write = dct_header.initial_write;
            }
        }
    }
#else
    {
        platform_dct_version_t  dct_version;
        /* read the version struct */
        if (ocf_buffer_read(device_start_address + (uint32_t)OFFSETOF(platform_dct_data_t, dct_version), (uint8_t*)&dct_version, sizeof(platform_dct_version_t)) == 0)
        {
            if ((dct_version.initial_write == 1) && (dct_version.crc32 == 0))
            {
                initial_write = dct_version.initial_write;
            }
        }
    }
#endif

    return initial_write;
}

/*
 * Validate and return the current DCT address
 *
 * - determine the SDK version of the DCT
 * - determine which DCT is valid and most recent
 * - update the DCT to DCT_BOOTLOADER_SDK_CURRENT if needed
 * - return section to the current DCT address
 *
 * - if both DCTs invalid, create a valid DCT to use
 *
 */

void* wiced_dct_get_current_address( dct_section_t section )
{
    wiced_dct_sdk_ver_t dct1_sdk_version, dct2_sdk_version;
    int dct1_initial_write, dct2_initial_write;
    int dct1_sequence, dct2_sequence;
    wiced_bool_t use_dct1;
    uint32_t                platform_dct_copy1_end_address = PLATFORM_DCT_COPY1_END_ADDRESS;
    uint32_t                platform_dct_copy2_end_address = PLATFORM_DCT_COPY2_END_ADDRESS;

    dct1_initial_write = wiced_dct_get_initial_write(PLATFORM_DCT_COPY1_START_ADDRESS);
    dct2_initial_write = wiced_dct_get_initial_write(PLATFORM_DCT_COPY2_START_ADDRESS);

#if !defined(BOOTLOADER)
    if ((dct1_initial_write == 0) && (dct2_initial_write == 0))
    {
          if (curr_verified_dct_addr == PLATFORM_DCT_COPY1_START_ADDRESS)
          {
              return (void*) ( PLATFORM_DCT_COPY1_START_ADDRESS + DCT_section_offsets[section] );
          }
          if (curr_verified_dct_addr == PLATFORM_DCT_COPY2_START_ADDRESS)
          {
              return (void*) ( PLATFORM_DCT_COPY2_START_ADDRESS + DCT_section_offsets[section] );
          }
    }
#endif

    dct1_sdk_version = wiced_dct_validate_and_determine_version(PLATFORM_DCT_COPY1_START_ADDRESS, platform_dct_copy1_end_address, &dct1_initial_write, &dct1_sequence);
    dct2_sdk_version = wiced_dct_validate_and_determine_version(PLATFORM_DCT_COPY2_START_ADDRESS, platform_dct_copy2_end_address, &dct2_initial_write, &dct2_sequence);

    /* check if one of the DCTs is valid */
    use_dct1 = WICED_TRUE;
    if ( ( dct1_sdk_version == DCT_BOOTLOADER_SDK_UNKNOWN ) && ( dct2_sdk_version == DCT_BOOTLOADER_SDK_UNKNOWN ) )
    {
        goto _both_dcts_invalid;
    }
    else if ( ( dct1_sdk_version != DCT_BOOTLOADER_SDK_UNKNOWN ) && ( dct2_sdk_version != DCT_BOOTLOADER_SDK_UNKNOWN ) )
    {
        /* Both DCTs are valid
         * determine most recent using version # (and sequence # if versions are the same)
         */
        if ( dct1_sdk_version < dct2_sdk_version )
        {
            use_dct1 = WICED_FALSE;
        }
        else if ( dct1_sdk_version == dct2_sdk_version )
        {
            /* if one of the version data has initial_write == 1, it is the correct one to use! */
            if ( ( dct2_initial_write != 0 ) && ( dct2_initial_write != 0xFF ) )
            {
                use_dct1 = WICED_FALSE;
            }
            else if ( ( dct1_initial_write != 0 ) && ( dct1_initial_write != 0xFF ) )
            {
                use_dct1 = WICED_TRUE;
            }
            else if ( dct1_sequence < dct2_sequence )
            {
                use_dct1 = WICED_FALSE;
            }
        }
    }
    else if ( dct1_sdk_version == DCT_BOOTLOADER_SDK_UNKNOWN )
    {
        /* dct2 is valid, use that */
        use_dct1 = WICED_FALSE;
    }

    /* make sure the DCT we are using is updated to the current SDK so the application can use it properly */
    if ( use_dct1 == WICED_TRUE )
    {
#if !defined(BOOTLOADER)

        if ( dct1_sdk_version != DCT_BOOTLOADER_SDK_CURRENT )
        {
            wiced_result_t result = wiced_dct_external_dct_update( PLATFORM_DCT_COPY2_START_ADDRESS, PLATFORM_DCT_COPY1_START_ADDRESS );
            if ( result == WICED_SUCCESS )
            {
                /* we just updated DCT1 to DCT2, use DCT2 */
                if ( wiced_dct_erase_non_current_dct( PLATFORM_DCT_COPY1_START_ADDRESS ) != WICED_SUCCESS )
                {
                    /* erase failed */
                }
                /* set current_dct for fast-out next time */
                curr_verified_dct_addr = PLATFORM_DCT_COPY2_START_ADDRESS;
                return (void*) ( PLATFORM_DCT_COPY2_START_ADDRESS + DCT_section_offsets[ section ] );
            }
        }
        /* set current_dct for fast-out next time */
        curr_verified_dct_addr = PLATFORM_DCT_COPY1_START_ADDRESS;
#endif
        return (void*) ( PLATFORM_DCT_COPY1_START_ADDRESS + DCT_section_offsets[ section ] );
    }
    else
    {
#if !defined(BOOTLOADER)
        if ( dct2_sdk_version != DCT_BOOTLOADER_SDK_CURRENT )
        {
            wiced_result_t result = wiced_dct_external_dct_update( PLATFORM_DCT_COPY1_START_ADDRESS, PLATFORM_DCT_COPY2_START_ADDRESS );

            if ( result == WICED_SUCCESS )
            {
                /* we just updated DCT2 to DCT1, use DCT1 */
                if ( wiced_dct_erase_non_current_dct( PLATFORM_DCT_COPY2_START_ADDRESS ) != WICED_SUCCESS )
                {
                    /*  erase_non_current_dct( DCT2 ) failed */
                }
                /* set current_dct for fast-out next time */
                curr_verified_dct_addr = PLATFORM_DCT_COPY1_START_ADDRESS;
                return (void*) ( PLATFORM_DCT_COPY1_START_ADDRESS + DCT_section_offsets[ section ] );
            }
        }
        /* set current_dct for fast-out next time */
        curr_verified_dct_addr = PLATFORM_DCT_COPY2_START_ADDRESS;
#endif
        return (void*) ( PLATFORM_DCT_COPY2_START_ADDRESS + DCT_section_offsets[ section ] );
    }

    _both_dcts_invalid:

    wiced_assert( "BOTH DCTs ARE INVALID!", 0 != 0 );

#if !defined(BOOTLOADER)
    /* set current_dct to bad for full test next time */
    curr_verified_dct_addr = (uint32_t)GET_CURRENT_ADDRESS_FAILED;
#endif

    /* TODO: try to create a new DCT - difficult as we have no app info */
    {
        uint16_t i;
        platform_dct_header_t dct_header;
        platform_dct_version_t dct_version;

        char zero_byte = 0;

        memset( &dct_header, 0x00, sizeof(platform_dct_header_t) );
        memset( &dct_version, 0x00, sizeof(platform_dct_version_t) );

        dct_header.magic_number = BOOTLOADER_MAGIC_NUMBER;
        dct_header.write_incomplete = 1;

        dct_version.magic_number = DCT_VERSION_MAGIC_NUMBER;
        dct_version.data_dct_usage_flags = WICED_DCT_CONFIG_FLAGS;
        dct_version.sequence = 1;
        dct_version.initial_write = 1; /* we don't need to compute the CRC here - initial_write precludes this test */

#if defined(DCT_BOOTLOADER_CRC_IS_IN_HEADER)
        dct_header.initial_write = 1; /* we don't need to compute the CRC here - initial_write precludes this test */
#else
        dct_header.is_current_dct = 1;
#endif

        /* TODO: generate boot_details or all is for nothing ! */
#if (DCT_BOOTLOADER_SDK_VERSION < DCT_BOOTLOADER_SDK_3_1_2)
        /* this SDK does not have apps_locations in bootloader_dct_header_t (platform_dct_header_t for the SDK) */
#else
        for (i=0; i < DCT_MAX_APP_COUNT; i++)
        {
            dct_header.apps_locations[ i ].id       = EXTERNAL_FIXED_LOCATION;
            dct_header.apps_locations[ i ].detail.external_fixed.location    = SFLASH_APPS_HEADER_LOC + sizeof(app_header_t) * i;
        }
#endif  /* (DCT_BOOTLOADER_SDK_VERSION < DCT_BOOTLOADER_SDK_3_1_2) */

#if defined( BOOTLOADER_LOAD_MAIN_APP_FROM_FILESYSTEM )
        dct_header.boot_detail.entry_point = 0;
        dct_header.boot_detail.load_details.valid = 1;
        dct_header.boot_detail.load_details.load_once = 0;
        dct_header.boot_detail.load_details.source.id = EXTERNAL_FILESYSTEM_FILE;
        dct_header.boot_detail.load_details.source.detail.filesystem_filename = "app.elf";
        dct_header.boot_detail.load_details.destination.id = INTERNAL;
#elif defined( BOOTLOADER_LOAD_MAIN_APP_FROM_EXTERNAL_LOCATION )
        dct_header.boot_detail.entry_point = 0;
        dct_header.boot_detail.load_details.valid = 1;
        dct_header.boot_detail.load_details.load_once = 0;
        dct_header.boot_detail.load_details.source.id = EXTERNAL_FIXED_LOCATION;
        dct_header.boot_detail.load_details.source.detail.external_fixed.location = SFLASH_APPS_HEADER_LOC + sizeof(app_header_t) * DCT_APP0_INDEX;
        dct_header.boot_detail.load_details.destination.id = INTERNAL;
#else
        dct_header.boot_detail.entry_point = 0;
        dct_header.boot_detail.load_details.valid = 0;
        dct_header.boot_detail.load_details.source.id = NONE;
        dct_header.boot_detail.load_details.source.detail.external_fixed.location = 0;
        dct_header.boot_detail.load_details.destination.id = INTERNAL;
#endif /* ifdef BOOTLOADER_LOAD_MAIN_APP_FROM_FILESYSTEM */

        /* Erase and init dct1 */
        if ( wiced_dct_erase_non_current_dct( PLATFORM_DCT_COPY1_START_ADDRESS ) != WICED_SUCCESS )
        {
            return GET_CURRENT_ADDRESS_FAILED;
        }
        /* write out the header with write_incomplete set */
        if ( ( ocf_buffer_write( PLATFORM_DCT_COPY1_START_ADDRESS, (uint8_t*) &dct_header, sizeof(platform_dct_header_t) ) == 0 ) &&
        /* write out the new version header */
        ( ocf_buffer_write( PLATFORM_DCT_COPY1_START_ADDRESS + OFFSETOF( platform_dct_data_t, dct_version ), (uint8_t*) &dct_version, sizeof(platform_dct_version_t) ) == 0 ) &&
        /* Mark new DCT as complete */
        ( ocf_buffer_write( PLATFORM_DCT_COPY1_START_ADDRESS + OFFSETOF( platform_dct_header_t, write_incomplete ), (uint8_t*) &zero_byte, sizeof( zero_byte ) ) == 0 ) )
        {
            /* erase dct2 */
            wiced_dct_erase_non_current_dct( PLATFORM_DCT_COPY2_START_ADDRESS );
            return (void*) ( PLATFORM_DCT_COPY1_START_ADDRESS + DCT_section_offsets[ section ] );
        }
    }
    return GET_CURRENT_ADDRESS_FAILED;
}

wiced_result_t wiced_dct_read_directly( void* dest, uint32_t source, uint32_t size )
{
    wiced_result_t  result = WICED_ERROR;
    /* check valid pointer */
    if ( dest == NULL || size == 0 )
    {
        return result;
    }

    //WICED_DCT_MUTEX_GET();

    if ( ocf_buffer_read( source, dest, size ) == 0 )
    {
        result = WICED_SUCCESS;
    }
    //WICED_DCT_MUTEX_RELEASE();
    return result;
}

wiced_result_t wiced_dct_read_with_copy( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size )
{
    uint32_t curr_dct;
    wiced_result_t result = WICED_ERROR;

    if ( ( info_ptr == NULL ) || ( size == 0 ) )
    {
        return result;
    }

    //WICED_DCT_MUTEX_GET();

    curr_dct = (uint32_t) wiced_dct_get_current_address( section );

    if ( curr_dct == (uint32_t) GET_CURRENT_ADDRESS_FAILED )
    {
        goto _read_with_copy_exit;
    }

    if ( ocf_buffer_read( curr_dct + offset, info_ptr, size ) == 0 )
    {
        result = WICED_SUCCESS;
    }

_read_with_copy_exit:
    //WICED_DCT_MUTEX_RELEASE();
    return result;
}

static wiced_result_t wiced_dct_copy_ocf( uint32_t dest_loc, uint32_t src_loc, uint32_t size )
{
    unsigned char buff[ 64 ];
    uint32_t write_size = 0;

    while ( size > 0 )
    {
        write_size = MIN( sizeof( buff ), size );

        if ( ocf_buffer_read( src_loc, buff, write_size ) != 0 )
        {
            return WICED_ERROR;
        }
        if ( ocf_buffer_write( dest_loc, buff, write_size ) != 0 )
        {
            return WICED_ERROR;
        }
        src_loc += write_size;
        dest_loc += write_size;
        size -= write_size;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_dct_erase_non_current_dct( uint32_t non_current_dct )
{
    if ( ef_erase_multiple_pages( non_current_dct, PLATFORM_DCT_COPY1_SIZE ) != 0 )
        return WICED_ERROR;
    else
        return WICED_SUCCESS;
}

wiced_result_t wiced_dct_write_boot_details( const boot_detail_t* new_boot_details )
{
    wiced_result_t          result = WICED_ERROR;
    uint32_t                copy_offset_start, bytes_to_copy;
    platform_dct_header_t   new_dct_header;
    uint32_t                new_dct, curr_dct;
    platform_dct_version_t  new_dct_version;
    boot_detail_t           old_boot_details;

    if ( new_boot_details == NULL )
    {
        return WICED_ERROR;
    }

    /* Check to see if the old boot details are the same - if so, don't write them! */
    if (wiced_dct_read_with_copy( &old_boot_details, DCT_INTERNAL_SECTION, OFFSETOF(platform_dct_data_t, dct_header) + OFFSETOF(platform_dct_header_t, boot_detail), sizeof(boot_detail_t) ) == WICED_SUCCESS)
    {
        if(memcmp(new_boot_details, &old_boot_details, sizeof(boot_detail_t)) == 0)
        {
            /* data is the same, return success */
            return WICED_SUCCESS;
        }
    }

    //WICED_DCT_MUTEX_GET();

    curr_dct = (uint32_t) wiced_dct_get_current_address( DCT_INTERNAL_SECTION );
    if ( curr_dct == (uint32_t) GET_CURRENT_ADDRESS_FAILED )
    {
        goto _write_boot_details_exit;
    }

    /* set up the new DCT (and get new_dct address) */
    if ( wiced_dct_start_new_from_current( &new_dct, &new_dct_header, &new_dct_version ) != WICED_SUCCESS )
    {
        goto _write_boot_details_exit;
    }

    /* update the boot detail info */
    memcpy( &new_dct_header.boot_detail, new_boot_details, sizeof(boot_detail_t) );

    /* Calculate how many bytes need to be written after the end of the header to the start of the dct_version structure */
    copy_offset_start = (OFFSETOF(platform_dct_data_t, dct_header) + sizeof(platform_dct_header_t));
    bytes_to_copy = OFFSETOF(platform_dct_data_t, dct_version) - copy_offset_start;
    if ( bytes_to_copy != 0 )
    {
        /* There is data after end of requested write - copy it from old DCT to new DCT */
        if ( wiced_dct_copy_ocf( new_dct + sizeof(platform_dct_header_t), curr_dct + sizeof(platform_dct_header_t), bytes_to_copy ) != WICED_SUCCESS )
        {
            goto _write_boot_details_exit;
        }
    }

    /* Calculate how many bytes need to be written after the system DCT structure to the end of the DCT area */
    copy_offset_start = (OFFSETOF(platform_dct_data_t, dct_version) + sizeof(platform_dct_version_t));
    bytes_to_copy = PLATFORM_DCT_COPY1_SIZE - copy_offset_start;
    if ( bytes_to_copy != 0 )
    {
        /* There is data after end of requested write - copy it from old DCT to new DCT */
        if ( wiced_dct_copy_ocf( new_dct + sizeof(platform_dct_data_t), curr_dct + sizeof(platform_dct_data_t), bytes_to_copy ) != WICED_SUCCESS )
        {
            goto _write_boot_details_exit;
        }
    }

    /* finish writes out the header and version info */
    result = wiced_dct_finish_new_dct(new_dct, &new_dct_header, &new_dct_version);

_write_boot_details_exit:
    //WICED_DCT_MUTEX_RELEASE();
    return result;
}

wiced_result_t wiced_dct_write_app_location( image_location_t *new_app_location, uint32_t app_index )
{
    wiced_result_t          result = WICED_ERROR;
    platform_dct_header_t   new_dct_header;
    uint32_t                new_dct, curr_dct;
    platform_dct_version_t  new_dct_version;
    uint32_t                copy_offset_start, bytes_to_copy;
    image_location_t        old_app_location;

    if ( ( app_index >= DCT_MAX_APP_COUNT ) || ( new_app_location == NULL ) )
    {
        return WICED_ERROR;
    }

    /* Check to see if the old app location data is the same - if so, don't write it! */
    if (wiced_dct_read_with_copy( &old_app_location, DCT_INTERNAL_SECTION,
                                  OFFSETOF(platform_dct_data_t, dct_header) + OFFSETOF(platform_dct_header_t, apps_locations) + (app_index * sizeof(image_location_t)),
                                  sizeof(image_location_t) ) == WICED_SUCCESS)
    {
        if(memcmp(new_app_location, &old_app_location, sizeof(image_location_t)) == 0)
        {
            /* data is the same, return success */
            return WICED_SUCCESS;
        }
    }

    //WICED_DCT_MUTEX_GET();
    curr_dct = (uint32_t) wiced_dct_get_current_address( DCT_INTERNAL_SECTION );
    if ( curr_dct == (uint32_t) GET_CURRENT_ADDRESS_FAILED )
    {
        goto _write_app_location_exit;
    }

    /* set up the new DCT (and get new_dct address) */
    if ( wiced_dct_start_new_from_current( &new_dct, &new_dct_header, &new_dct_version ) != WICED_SUCCESS )
    {
        goto _write_app_location_exit;
    }

#if (DCT_BOOTLOADER_SDK_VERSION < DCT_BOOTLOADER_SDK_3_1_2)
    /* this SDK does not have apps_locations in bootloader_dct_header_t (platform_dct_header_t for the SDK) */
#else
    /* use new app location info */
    memcpy( &new_dct_header.apps_locations[ app_index ], new_app_location, sizeof(image_location_t) );
#endif /* (DCT_BOOTLOADER_SDK_VERSION < DCT_BOOTLOADER_SDK_3_1_2) */

    /* Calculate how many bytes need to be written after the end of the header to the start of the dct_version structure */
    copy_offset_start = (OFFSETOF(platform_dct_data_t, dct_header) + sizeof(platform_dct_header_t));
    bytes_to_copy = OFFSETOF(platform_dct_data_t, dct_version) - copy_offset_start;
    if ( bytes_to_copy != 0 )
    {
        /* There is data after end of requested write - copy it from old DCT to new DCT */
        if ( wiced_dct_copy_ocf( new_dct + sizeof(platform_dct_header_t), curr_dct + sizeof(platform_dct_header_t), bytes_to_copy ) != WICED_SUCCESS )
        {
            goto _write_app_location_exit;
        }
    }

    /* Calculate how many bytes need to be written after the system DCT structure to the end of the DCT area */
    copy_offset_start = (OFFSETOF(platform_dct_data_t, dct_version) + sizeof(platform_dct_version_t));
    bytes_to_copy = PLATFORM_DCT_COPY1_SIZE - copy_offset_start;
    if ( bytes_to_copy != 0 )
    {
        /* There is data after end of requested write - copy it from old DCT to new DCT */
        if ( wiced_dct_copy_ocf( new_dct + sizeof(platform_dct_data_t), curr_dct + sizeof(platform_dct_data_t), bytes_to_copy ) != WICED_SUCCESS )
        {
            goto _write_app_location_exit;
        }
    }

    result = wiced_dct_finish_new_dct(new_dct, &new_dct_header, &new_dct_version);

_write_app_location_exit:
    //WICED_DCT_MUTEX_RELEASE();
    return result;
}

wiced_result_t wiced_dct_write( const void* data, dct_section_t section, uint32_t offset, uint32_t data_length )
{
    platform_dct_header_t new_dct_header;
    platform_dct_version_t new_dct_version;
    uint32_t copy_offset_start,bytes_to_copy;
    uint32_t section_start = DCT_section_offsets[ section ];
    uint32_t new_dct, curr_dct;
    wiced_result_t  result = WICED_ERROR;
    uint8_t extra_bytes=0;
    uint8_t extra_word[OCF_BUFFER_SIZE];
    uint8_t *input_data = (uint8_t*)data;

    /* Check if the data is outside the bounds to write */
    if ( ( section_start < sizeof(platform_dct_header_t) ) || ( ( section_start + offset + data_length ) > PLATFORM_DCT_COPY1_SIZE ) )
    {
        return WICED_ERROR;
    }

    if ( ( section == DCT_INTERNAL_SECTION ) || ( section == DCT_VERSION_SECTION ) )
    {
        /* user should not call to change these sections */
        return WICED_ERROR;
    }


    /* This is not available for the bootloader because you can't malloc in the bootloader */
#if !defined(BOOTLOADER)
    {
        char*                   write_check_buffer;
        /* Check that the data they are going to write is a change. if the data is the same, don't re-write the DCT
         * If we can't allocate enough RAM for the test, just go ahead and re-write the DCT.
         */

        /* read in the data */
        if (wiced_dct_read_lock((void**)&write_check_buffer, WICED_TRUE, section, offset, data_length) == WICED_SUCCESS)
        {
            wiced_bool_t need_to_write;
            need_to_write = (memcmp(write_check_buffer, data, data_length) == 0) ? WICED_FALSE : WICED_TRUE;
            wiced_dct_read_unlock(write_check_buffer, WICED_TRUE);
            if (need_to_write == WICED_FALSE)
            {
                /* data to write matches the data already written - don't update, just return success */
                return WICED_SUCCESS;
            }
        }
    }
#endif
    //WICED_DCT_MUTEX_GET();
    curr_dct = (uint32_t) wiced_dct_get_current_address( DCT_INTERNAL_SECTION );
    if ( curr_dct == (uint32_t) GET_CURRENT_ADDRESS_FAILED )
    {
        goto _dct_write_fail;
    }

    /* set up the new DCT (and get new_dct address) */
    if ( wiced_dct_start_new_from_current( &new_dct, &new_dct_header, &new_dct_version ) != WICED_SUCCESS )
    {
        goto _dct_write_fail;
    }

    /* erase new DCT */
    if ( wiced_hal_eflash_erase (new_dct,PLATFORM_DCT_COPY1_SIZE) != WICED_SUCCESS)
    {
        WPRINT_PLATFORM_ERROR (("eflash erase failed !!! \n"));
        goto _dct_write_fail;
    }


    /*
     * There are 3 parts to updating the data (other than platform_dct_header_t and platform_dct_version_t which
     *     are handled in the function wiced_dct_finish_new_dct() above)
     * 1) Data before the (section_start + offset) of the new data
     *      A) Some new data is between dct_header and dct_version
     *      B) Some new data is between dct_version and the end of platform_dct_data_t
     *      C) Some new data is after platform_dct_data_t
     * 2) the new data (section_start + offset) size: data_length
     *      A) Some new data is between dct_header and dct_version
     *      B) Some new data is between dct_version and the end of platform_dct_data_t
     *      C) Some new data is after platform_dct_data_t
     * 3) Data after the (section_start + offset+ data_length) of the new data, up to the end of PLATFORM_DCT_COPY1_SIZE
     *      A) Some new data is between dct_header and dct_version
     *      B) Some new data is between dct_version and the end of platform_dct_data_t
     *      C) Some new data is after platform_dct_data_t
     *
     * The code here is generic enough to handle all three cases without having to be maintained  B^)
     *
     */

     /* 1) Data before the (section_start + offset) of the new data */
    if (section_start < OFFSETOF(platform_dct_data_t, dct_version))
    {
        /* A) Some new data is between dct_header and dct_version    */
        copy_offset_start = sizeof(platform_dct_header_t);
        bytes_to_copy = (section_start + offset) - copy_offset_start;
        /* use copy at end of this section */
    }
    else if (section_start < sizeof(platform_dct_data_t))
    {
        /* B) Some new data is between dct_version and the end of platform_dct_data_t
         * We need to do 2 things here:
         * i)  Copy the whole of the data between dct_header and dct_version
         * ii) Copy the data after dct_version up to the new data
         */

        /* i)  Copy the whole of the data between dct_header and dct_version    */
        copy_offset_start = sizeof(platform_dct_header_t);
        bytes_to_copy = OFFSETOF(platform_dct_data_t, dct_version) - copy_offset_start;
        if ( wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS )
        {
            goto _dct_write_fail;
        }
          /* ii) Copy the data after dct_version up to the new data    */
        copy_offset_start = OFFSETOF(platform_dct_data_t, dct_version) + sizeof(platform_dct_version_t);
        bytes_to_copy = (section_start + offset)- copy_offset_start;
        /* use copy at end of this section */
    }
    else
    {
        /* C) Data that falls beyond platform_dct_data_t ( DCT_APP_SECTION and CT_HK_INFO_SECTION)
         * We need to do 2 things here:
         * i)  Copy the whole of the data between dct_header and dct_version
         * ii) Copy the data after dct_version up to the new data
         */

        /* i)  Copy the whole of the data between dct_header and dct_version    */
        copy_offset_start = sizeof(platform_dct_header_t);
        bytes_to_copy = OFFSETOF(platform_dct_data_t, dct_version) - copy_offset_start;

        if (wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS)
        {
            goto _dct_write_fail;
        }
        /* ii) Copy the data after dct_version up to the new data    */
        copy_offset_start = OFFSETOF(platform_dct_data_t, dct_version) + sizeof(platform_dct_version_t);
        bytes_to_copy = (section_start + offset) - copy_offset_start;
        /* use common copy at end of this section */
    }

    if (bytes_to_copy > 0)
    {
         /* check bytes_to_copy is 4 bytes aligned */
         /* if not then read extra aligned bytes  and write */

         extra_bytes = bytes_to_copy % OCF_BUFFER_SIZE;
         if (extra_bytes)
         {
             /* unaligned size */
             /* read extra bytes to make it align */
             if ( bytes_to_copy > OCF_BUFFER_SIZE )
             {
                bytes_to_copy = bytes_to_copy - extra_bytes;
             }
             else
             {
                 bytes_to_copy = 0; // it will be handle in the next check
             }

            /* read 4 bytes from curr_dct and prepend it to input data */
            if ( wiced_hal_eflash_read((curr_dct + copy_offset_start + bytes_to_copy), extra_word, OCF_BUFFER_SIZE ) != WICED_SUCCESS)
            {
                goto _dct_write_fail;
            }

            if (bytes_to_copy > 0)
            {
                if (wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS)
                {
                    goto _dct_write_fail;
                }
            }
         }
         else
         {
            if (wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS)
            {
                goto _dct_write_fail;
            }
         }
    } //end

     if (extra_bytes)
     {
         copy_offset_start = copy_offset_start + bytes_to_copy;

        /* prepend extra_word to data */
        if ( (int)data_length < (OCF_BUFFER_SIZE-extra_bytes) )
        {
            memcpy (&extra_word[extra_bytes], (uint8_t*)input_data, data_length);
            if ( ocf_buffer_write( new_dct + copy_offset_start, (uint8_t*)extra_word, OCF_BUFFER_SIZE ) != WICED_SUCCESS )
            {
                goto _dct_write_fail;
            }
            data_length = 0;
            //complete writing user data
        }
        else
        {
            memcpy (&extra_word[extra_bytes], (uint8_t*)input_data, (OCF_BUFFER_SIZE-(size_t)extra_bytes));

            if ( ocf_buffer_write( new_dct + copy_offset_start, (uint8_t*)extra_word, OCF_BUFFER_SIZE ) != WICED_SUCCESS )
            {
                goto _dct_write_fail;
            }

            data_length = data_length - (OCF_BUFFER_SIZE-(size_t)extra_bytes);
            input_data = input_data + (OCF_BUFFER_SIZE-(size_t)extra_bytes);
        }

        copy_offset_start = copy_offset_start + OCF_BUFFER_SIZE;
        bytes_to_copy = data_length;

     }
     else
     {
         copy_offset_start = section_start + offset;
         bytes_to_copy = data_length;
     }



    /* 2) the new data (section_start + offset) size: data_length
    *      A) Some new data is between dct_header and dct_version
    *      B) Some new data is between dct_version and the end of platform_dct_data_t
    *      C) Some new data is after platform_dct_data_t
    *
    *  We do this the same for all  new data in any section
    */

    /* use common copy at end of this section */
    if (bytes_to_copy > 0)
    {
        extra_bytes = bytes_to_copy % OCF_BUFFER_SIZE;
        if (extra_bytes)
        {
            /* unaligned size */
            /* read extra bytes to make it align */
            if ( bytes_to_copy > OCF_BUFFER_SIZE )
            {
                bytes_to_copy = bytes_to_copy - extra_bytes;
            }
            else
            {
                bytes_to_copy = 0; // it will be handle in the next check
            }

            /* read 4 bytes from curr_dct and prepend it to input data */
            if ( wiced_hal_eflash_read((curr_dct + copy_offset_start + bytes_to_copy), extra_word, OCF_BUFFER_SIZE ) != WICED_SUCCESS)
            {
                goto _dct_write_fail;
            }

            if (bytes_to_copy > 0)
            {
                /* copy all aligned bytes */
                if ( ocf_buffer_write( new_dct + copy_offset_start, (uint8_t*)input_data, bytes_to_copy ) != WICED_SUCCESS )
                {
                    goto _dct_write_fail;
                }

                copy_offset_start = copy_offset_start + bytes_to_copy;
                input_data = input_data + bytes_to_copy;
            }

            memcpy(extra_word,input_data,extra_bytes);

            /* write rest of the odd bytes */
            if ( ocf_buffer_write( new_dct + copy_offset_start, (uint8_t*)extra_word, OCF_BUFFER_SIZE ) != WICED_SUCCESS )
            {
                goto _dct_write_fail;
            }

            copy_offset_start = copy_offset_start + OCF_BUFFER_SIZE;

        }//unaligned size
        else
        {
            /* write specific section data */
            if ( ocf_buffer_write( new_dct + copy_offset_start, (uint8_t*)input_data, data_length ) != WICED_SUCCESS )
            {
                goto _dct_write_fail;
            }

            copy_offset_start = copy_offset_start + data_length;
        }
    }

    /* 3) Data after the (section_start + offset+ data_length) of the new data to the end of PLATFORM_DCT_COPY1_SIZE
    *      A) Some new data is between dct_header and dct_version
    *      B) Some new data is between dct_version and the end of platform_dct_data_t
    *      C) Some new data is after platform_dct_data_t
    */
    if (section_start < OFFSETOF(platform_dct_data_t, dct_version))
    {
        /*      A) Some new data is between dct_header and dct_version
         *  We need to do 2 things here
         *  i) Copy between the end of the new data and the start of dct_version
         *  ii) Copy from end of dct_version to the end of the DCT section (includes DCT_MISC_SECTION, DCT_APP_SECTION, DCT_HK_INFO_SECTION)
         */

        /*  i) Copy between the end of the new data and the start of dct_version    */
        bytes_to_copy = OFFSETOF(platform_dct_data_t, dct_version) - copy_offset_start;
        if (bytes_to_copy > 0)
        {
            if (wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS)
            {
                goto _dct_write_fail;
            }
        }
        /*  ii) Copy from end of dct_version to the end of the DCT section (includes DCT_MISC_SECTION, DCT_APP_SECTION, DCT_HK_INFO_SECTION)    */
        copy_offset_start = OFFSETOF(platform_dct_data_t, dct_version) + sizeof(platform_dct_version_t);
        bytes_to_copy = PLATFORM_DCT_COPY1_SIZE - copy_offset_start;
        /* use common copy at end of this section */
    }
     else if (section_start < sizeof(platform_dct_data_t))
    {
        /* B) Some new data is between dct_version and the end of platform_dct_data_t
         * We copy from the end of the new data to the end of PLATFORM_DCT_COPY1_SIZE
         */
        bytes_to_copy = PLATFORM_DCT_COPY1_SIZE - copy_offset_start;
        /* use common copy at end of this section */
    }
     else
    {
        /* C) Data that falls beyond platform_dct_data_t ( DCT_APP_SECTION and CT_HK_INFO_SECTION)
         * We copy from the end of the new data to the end of PLATFORM_DCT_COPY1_SIZE
         */
        bytes_to_copy = PLATFORM_DCT_COPY1_SIZE - copy_offset_start;
        /* use common copy at end of this section */
    }
    if (bytes_to_copy > 0)
     {
        if (wiced_dct_copy_ocf( new_dct + copy_offset_start, curr_dct + copy_offset_start, bytes_to_copy ) != WICED_SUCCESS)
        {
            goto _dct_write_fail;
        }
    }

    /* update any header info and finish the new DCT */
    new_dct_header.app_valid = 1;

    result = wiced_dct_finish_new_dct(new_dct, &new_dct_header, &new_dct_version);

_dct_write_fail:
    //WICED_DCT_MUTEX_RELEASE();
    return result;

}

#if (DCT_BOOTLOADER_SDK_VERSION < DCT_BOOTLOADER_SDK_3_1_2)

#else
wiced_result_t wiced_dct_restore_factory_reset( void )
{
    return WICED_UNSUPPORTED;
}

wiced_result_t wiced_dct_get_app_header_location( uint8_t app_id, image_location_t* app_header_location )
{
    if ( app_id >= DCT_MAX_APP_COUNT )
    {
        return WICED_ERROR;
    }
    return wiced_dct_read_with_copy( app_header_location, DCT_INTERNAL_SECTION, DCT_APP_LOCATION_OF( app_id ), sizeof(image_location_t) );
}

wiced_result_t wiced_dct_set_app_header_location( uint8_t app_id, image_location_t* app_header_location )
{
    if ( app_id >= DCT_MAX_APP_COUNT )
    {
        return WICED_ERROR;
    }
    return wiced_dct_write_app_location( app_header_location, app_id );
}
#endif

#if defined(OTA2_SUPPORT)
wiced_result_t wiced_dct_ota2_erase_save_area_and_copy_dct(uint32_t dst_dct)
{
    wiced_result_t          result;
    uint32_t                src_dct;
    uint32_t                platform_dct_copy_size = PLATFORM_DCT_COPY1_SIZE;

    /* Erase the destination DCT. */
    if (wiced_dct_erase_non_current_dct( dst_dct ) != WICED_SUCCESS)
    {
        return WICED_ERROR;
    }

    /* copy the current DCT to the save area */
    src_dct = (uint32_t)wiced_dct_get_current_address( DCT_INTERNAL_SECTION );
    result = wiced_dct_copy_ocf( dst_dct, src_dct, platform_dct_copy_size );

    return result;
}
#endif
