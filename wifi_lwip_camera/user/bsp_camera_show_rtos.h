#ifndef __BSP_CAMERA_SHOW_RTOS_H__
#define __BSP_CAMERA_SHOW_RTOS_H__
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

void Show_img(void);
#define CSI_W               188          
#define CSI_H               120 

//extern uint8_t image_csi1[CSI_H][CSI_W];
//extern uint8_t image_csi2[CSI_H][CSI_W];

//extern uint8_t (*user_image)[CSI_W];

//#define USE_LCD


#endif 
