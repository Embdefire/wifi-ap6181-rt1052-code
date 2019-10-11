/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   cammera—ov2640摄像头
  ******************************************************************
  * @attention
  *
  * 实验平台:野火  i.MXRT1052开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************
  */
#include <stdio.h>
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "./lcd/bsp_camera_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./camera/bsp_ov2640_config.h"
#include "./lcd/bsp_lcd.h"
#include "./systick/bsp_systick.h"
#include "./key/bsp_key.h"

/*******************************************************************
 * Code
 *******************************************************************/

//显示帧率数据，默认不显示，需要显示时把这个宏设置为1即可
#define FRAME_RATE_DISPLAY 0
/*简单任务管理*/
uint32_t Task_Delay[NumOfTask];
extern void LCD_Test(void);
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
uint8_t fps = 0;

int main(void)
{
	/* 初始化内存保护单元 */
	BOARD_ConfigMPU();
	/* 初始化开发板引脚 */
	BOARD_InitPins();
	/* 初始化开发板时钟 */
	BOARD_BootClockRUN();
	/* 初始化调试串口 */
	BOARD_InitDebugConsole();
	/* 初始化液晶接口*/
	BOARD_InitLcd();
	/* 按键初始化 */
	Key_GPIO_Config();
	/* 打印系统时钟 */
	PRINTF("\r\n");
	PRINTF("*****欢迎使用 野火i.MX RT1052 开发板*****\r\n");
	PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
	PRINTF("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
	PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
	PRINTF("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
	PRINTF("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
	PRINTF("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
	PRINTF("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
	PRINTF("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
	PRINTF("CSI RGB565 example start...\r\n");
	/*  精确延时 */
	//SysTick_Init();
	/* 相机初始化*/
	Camera_Init();
	int a=20;
	while (1)
	{
//		if (kStatus_Success == CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr))
//    {
		
			CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr);
			CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(image_csi1[0]));  
			if(activeFrameAddr == (uint32_t)image_csi1[0])
			{
					PRINTF("image_csi1采集完成...\r\n");
			}
			else if(activeFrameAddr == (uint32_t)image_csi2[0])
			{		
				PRINTF("image_csi2采集完成...\r\n");
			}


			//			memset(image_csi1[10+a],0x55,sizeof(image_csi1[10+a]));		
			//			memset(image_csi1[11+a],0x55,sizeof(image_csi1[11+a]));		
			//			memset(image_csi1[12+a],0x55,sizeof(image_csi1[12+a]));		
			//			memset(image_csi1[13+a],0x55,sizeof(image_csi1[13+a]));		
			//			memset(image_csi1[14+a],0x55,sizeof(image_csi1[14+a]));		
			//			memset(image_csi1[15+a],0x55,sizeof(image_csi1[15+a]));		
			//			memset(image_csi1[16+a],0x55,sizeof(image_csi1[16+a]));		
			ELCDIF_SetNextBufferAddr(APP_ELCDIF, (uint32_t)(image_csi1[0]));
		
		a++;
		if(a>400)
		{
			a=0;
		}
//    }

	}
}

/****************************END OF FILE**********************/
