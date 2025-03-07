#
# Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 # Cypress Semiconductor Corporation. All Rights Reserved.
 # This software, including source code, documentation and related
 # materials ("Software"), is owned by Cypress Semiconductor Corporation
 # or one of its subsidiaries ("Cypress") and is protected by and subject to
 # worldwide patent protection (United States and foreign),
 # United States copyright laws and international treaty provisions.
 # Therefore, you may use this Software only as provided in the license
 # agreement accompanying the software package from which you
 # obtained this Software ("EULA").
 # If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 # non-transferable license to copy, modify, and compile the Software
 # source code solely for use in connection with Cypress's
 # integrated circuit products. Any reproduction, modification, translation,
 # compilation, or representation of this Software except as specified
 # above is prohibited without the express written permission of Cypress.
 #
 # Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 # reserves the right to make changes to the Software without notice. Cypress
 # does not assume any liability arising out of the application or use of the
 # Software or any product or circuit described in the Software. Cypress does
 # not authorize its products for use in any products where a malfunction or
 # failure of the Cypress product may reasonably be expected to result in
 # significant property damage, injury or death ("High Risk Product"). By
 # including Cypress's product in a High Risk Product, the manufacturer
 # of such system or application assumes all risk of such use and in doing
 # so agrees to indemnify Cypress against all liability.
#
WLAN_CHIP                := 43909
WLAN_CHIP_FAMILY         := 4390x
HOST_MCU_FAMILY          := BCM4390x
HOST_MCU_VARIANT         := BCM43909
HOST_MCU_PART_NUMBER     := BCM43909KRFBG

BT_CHIP                  := 20706
BT_CHIP_REVISION         := A1
PLATFORM_NAME            := BCM943909WCD1_3
WLAN_CHIP_REVISION       := B0

DEFAULT_BOARD_REVISION   := P320
BOARDS_WITH_B0_CHIP      := P308
BOARDS_WITH_B1_CHIP      := P319
BOARDS_WITH_B2_CHIP      := P320
SUPPORTED_BOARD_REVISION := $(BOARDS_WITH_B0_CHIP) $(BOARDS_WITH_B1_CHIP) $(BOARDS_WITH_B2_CHIP)
BOARD_REVISION_DIR       := board_revision

#if no board revision specified in build string
ifeq ($(BOARD_REVISION),)
BOARD_REVISION           := $(DEFAULT_BOARD_REVISION)
endif

ifneq ($(filter $(BOARD_REVISION), $(BOARDS_WITH_B0_CHIP)),)
APPS_CHIP_REVISION       := B0
else ifneq ($(filter $(BOARD_REVISION), $(BOARDS_WITH_B1_CHIP) $(BOARDS_WITH_B2_CHIP) ),)
APPS_CHIP_REVISION       := B1
endif

NAME                     := Platform_$(PLATFORM_NAME)_$(BOARD_REVISION)_$(APPS_CHIP_REVISION)
PLATFORM_SUPPORTS_BUTTONS := 1

GLOBAL_DEFINES           += SFLASH_SUPPORT_MACRONIX_PARTS
GLOBAL_DEFINES           += WICED_DCT_INCLUDE_BT_CONFIG

BOARD_SPECIFIC_OPENOCD_SCRIPT := $(SOURCE_ROOT)WICED/platform/MCU/BCM4390x/peripherals/board_init_ddr_nanya_nt5cb64m16dp_cf.cfg

PLATFORM_SOURCES         := $(SOURCE_ROOT)platforms/$(PLATFORM_DIRECTORY)/

$(NAME)_SOURCES          := platform.c

GLOBAL_INCLUDES          := . \
                            ./$(BOARD_REVISION_DIR)/$(BOARD_REVISION)

ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))
$(NAME)_SOURCES          += platform_audio.c
$(NAME)_COMPONENTS       += drivers/audio/AK4954 \
                            drivers/audio/WM8533 \
                            drivers/audio/spdif
endif  # (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))

# If you want to use below drirver, please add "INCLUDE_RTL_CODEC  := TRUE" string
# in application mk file.
ifeq ($(INCLUDE_RTL_DAC), TRUE)
GLOBAL_DEFINES           += INCLUDE_RTL_DAC
$(NAME)_COMPONENTS       += drivers/audio/RT5628
endif
