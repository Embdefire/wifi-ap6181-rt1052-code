#include "bsp_camera_show_rtos.h"

#include "./lcd/bsp_camera_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./camera/bsp_ov2640_config.h"
#include "./lcd/bsp_lcd.h"
#include "./systick/bsp_systick.h"
#include "./key/bsp_key.h"


#include "fsl_common.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef unsigned char       uint8;




//图像缓冲区  如果用户需要访问图像数据 最好通过user_image来访问数据，最好不要直接访问缓冲区
//ALIGN(64) uint8 image_csi1[CSI_H][CSI_W];
//ALIGN(64) uint8 image_csi2[CSI_H][CSI_W];
uint8_t image_csi1[CSI_H][CSI_W];
uint8_t image_csi2[CSI_H][CSI_W];

//用户访问图像数据直接访问这个指针变量就可以
//访问方式非常简单，可以直接使用下标的方式访问
//例如访问第10行 50列的点，user_image[10][50]就可以了
uint8_t (*user_image)[CSI_W];



void HardFault_Handler()
{

	while (1)
  {
  }
}

//static char img_data[APP_IMG_HEIGHT][APP_IMG_WIDTH];
//uint32_t img_data[APP_IMG_HEIGHT];
/**
 * @brief 摄像头采集
 *
 * @return 无
 *   @retval 无
 */
void  Get_Camera_Data_Task(void)
{
	//Camera_Init();
	while (1)
	{
		#if 0

		//CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);
		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, *img_data);
    
		/* 等待获取完整帧缓冲区以显示 */
		 //CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr);
		
//		CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, img_data);
		

//		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr))
//		{ 
//		}
		
		
		PRINTF("帧缓冲 ok \r\n");
		vTaskDelay(200);
		
		#endif

	}
}




