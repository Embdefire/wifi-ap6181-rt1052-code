#include "bsp_camera_show_rtos.h"

#include "./lcd/bsp_camera_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./camera/bsp_ov2640_config.h"
#include "./lcd/bsp_lcd.h"
#include "./systick/bsp_systick.h"
#include "./key/bsp_key.h"

/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void HardFault_Handler()
{

	while (1)
  {
  }
}

/**
 * @brief 摄像头采集
 *
 * @return 无
 *   @retval 无
 */
int Camera_collection(void)
//int main(void)
{
//	//taskENTER_CRITICAL();           //进入临界区
//	Camera_Init();
//	//taskEXIT_CRITICAL();            //退出临界区
	
		/* 初始化内存保护单元 */
//	BOARD_ConfigMPU();
//	/* 初始化开发板引脚 */
//	BOARD_InitPins();
//	/* 初始化开发板时钟 */
//	BOARD_BootClockRUN();
//	/* 初始化调试串口 */
//	BOARD_InitDebugConsole();
//	/* 初始化液晶接口*/
//	BOARD_InitLcd();
//	/* 按键初始化 */
//	Key_GPIO_Config();
	Camera_Init();
	while (1)
	{
		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);
    
		/* 等待获取完整帧缓冲区以显示 */
		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr))
		{
		}
		PRINTF("帧缓冲 ok \r\n");

	}
}


