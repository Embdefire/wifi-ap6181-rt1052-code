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

#define MT9V03X_CSI_H 1
#define MT9V03X_CSI_W 1024*1024*2

uint8_t image_csi1[MT9V03X_CSI_H][MT9V03X_CSI_W];
uint8_t (*user_image)[MT9V03X_CSI_W];

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
uint8_t fps = 0;

uint32_t fullCameraBufferAddr;    //采集完成的缓冲区地址 

void USART_SendData( uint16_t Data)
{
  
  /* Transmit Data */
  PRINTF("%x",(Data & (uint16_t)0x01FF));
}


uint8_t jpeg_data_ok=0;

/**
* @brief 发送图片到设备
 *
 * @param img_buf 地址信息
 * @param img_buf_len 长度
 */
void send_jpeg_dev(uint8_t *img_buf,uint32_t img_buf_len)
{
	int img_index=0;
	for(img_index=0;img_index<img_buf_len;img_index++)
	{
		PRINTF("%x ",img_buf[img_index]);	
	}
}

/**
  * @brief  摄像头中断处理函数
  * @param  无
  * @retval 无
  */
extern uint8_t fps;
void CSI_IRQHandler(void)
{
		
    CSI_DriverIRQHandler();
		fps++; //帧率计数
		
		CAMERA_RECEIVER_Stop(&cameraReceiver);//停止采集
		jpeg_data_ok=1;
}

volatile uint32_t JPEG_len=0; //照片长度
int main(void)
{
	uint32_t i,jpgstart,jpglen; 
	uint8_t *p;
	uint8_t headok=0;	
	
	/* 初始化内存保护单元 */
	BOARD_ConfigMPU();
	/* 初始化开发板引脚 */
	BOARD_InitPins();
	/* 初始化开发板时钟 */
	BOARD_BootClockRUN();
	/* 初始化调试串口 */
	BOARD_InitDebugConsole();

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
	SysTick_Init();
	/* 相机初始化*/
	Camera_Init();
//	CAMERA_RECEIVER_Stop(&cameraReceiver);//停止采集
	while(1)
	{	
			if(jpeg_data_ok==0)
			{
							/* 开始采集图像*/
					//memset(*user_image,0,MT9V03X_CSI_W);
					CAMERA_RECEIVER_Start(&cameraReceiver);

					CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, fullCameraBufferAddr);			
					/* 等待获取完整帧缓冲区以显示 */
					while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr))
					{
					}
					CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr);
					if(fullCameraBufferAddr == (uint32_t)image_csi1[0])
					{
						PRINTF("帧 ok \r\n ");
					}
					CAMERA_RECEIVER_Stop(&cameraReceiver);//停止采集
			}
			
			if(jpeg_data_ok==1)
			{
					PRINTF("停止采集 获取图片中 \r\n ");
					p=(uint8_t*)user_image[0];
					jpglen=0;	//设置jpg文件大小为0
					headok=0;	//清除jpg头标记
					
					while(1)//查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小
					{
						if((p[i]==0XFF)&&(p[i+1]==0XD8))//找到FF D8
						{
							jpgstart=i;
							headok=1;	//标记找到jpg头(FF D8)
							PRINTF("find jpeg head \r\n ");
						}
						if((p[i]==0XFF)&&(p[i+1]==0XD9)&&headok)//找到头以后,再找FF D9
						{
							jpglen=i-jpgstart+2;
							
							JPEG_len=jpglen;
							p+=jpgstart;			//偏移到0XFF,0XD8处
							PRINTF("find jpeg end \r\n ");
							
							PRINTF("start send img buf \r\n ");
							send_jpeg_dev(p,JPEG_len);
							//memset(*user_image,0,sizeof(*user_image));
							PRINTF("img buf send over \r\n ");
							//return p;
							break;
						}
						i++;
						if(i>1024*10)//限制一下 避免发生硬件错误
						{
							i=0;
						}
					}	
			}
		
      
	}

}

/****************************END OF FILE**********************/
