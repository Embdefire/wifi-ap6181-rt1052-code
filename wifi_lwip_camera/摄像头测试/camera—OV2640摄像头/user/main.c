/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   cammera��ov2640����ͷ
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
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

//��ʾ֡�����ݣ�Ĭ�ϲ���ʾ����Ҫ��ʾʱ�����������Ϊ1����
#define FRAME_RATE_DISPLAY 0
/*���������*/
uint32_t Task_Delay[NumOfTask];
extern void LCD_Test(void);
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
uint8_t fps = 0;

int main(void)
{
	/* ��ʼ���ڴ汣����Ԫ */
	BOARD_ConfigMPU();
	/* ��ʼ������������ */
	BOARD_InitPins();
	/* ��ʼ��������ʱ�� */
	BOARD_BootClockRUN();
	/* ��ʼ�����Դ��� */
	BOARD_InitDebugConsole();
	/* ��ʼ��Һ���ӿ�*/
	BOARD_InitLcd();
	/* ������ʼ�� */
	Key_GPIO_Config();
	/* ��ӡϵͳʱ�� */
	PRINTF("\r\n");
	PRINTF("*****��ӭʹ�� Ұ��i.MX RT1052 ������*****\r\n");
	PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
	PRINTF("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
	PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
	PRINTF("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
	PRINTF("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
	PRINTF("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
	PRINTF("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
	PRINTF("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
	PRINTF("CSI RGB565 example start...\r\n");
	/*  ��ȷ��ʱ */
	//SysTick_Init();
	/* �����ʼ��*/
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
					PRINTF("image_csi1�ɼ����...\r\n");
			}
			else if(activeFrameAddr == (uint32_t)image_csi2[0])
			{		
				PRINTF("image_csi2�ɼ����...\r\n");
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
