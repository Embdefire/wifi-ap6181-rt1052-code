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

#define MT9V03X_CSI_H 1
#define MT9V03X_CSI_W 1024*1024*2

uint8_t image_csi1[MT9V03X_CSI_H][MT9V03X_CSI_W];
uint8_t (*user_image)[MT9V03X_CSI_W];

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
uint8_t fps = 0;

uint32_t fullCameraBufferAddr;    //�ɼ���ɵĻ�������ַ 

void USART_SendData( uint16_t Data)
{
  
  /* Transmit Data */
  PRINTF("%x",(Data & (uint16_t)0x01FF));
}


uint8_t jpeg_data_ok=0;

/**
* @brief ����ͼƬ���豸
 *
 * @param img_buf ��ַ��Ϣ
 * @param img_buf_len ����
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
  * @brief  ����ͷ�жϴ�����
  * @param  ��
  * @retval ��
  */
extern uint8_t fps;
void CSI_IRQHandler(void)
{
		
    CSI_DriverIRQHandler();
		fps++; //֡�ʼ���
		
		CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
		jpeg_data_ok=1;
}

volatile uint32_t JPEG_len=0; //��Ƭ����
int main(void)
{
	uint32_t i,jpgstart,jpglen; 
	uint8_t *p;
	uint8_t headok=0;	
	
	/* ��ʼ���ڴ汣����Ԫ */
	BOARD_ConfigMPU();
	/* ��ʼ������������ */
	BOARD_InitPins();
	/* ��ʼ��������ʱ�� */
	BOARD_BootClockRUN();
	/* ��ʼ�����Դ��� */
	BOARD_InitDebugConsole();

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
	SysTick_Init();
	/* �����ʼ��*/
	Camera_Init();
//	CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
	while(1)
	{	
			if(jpeg_data_ok==0)
			{
							/* ��ʼ�ɼ�ͼ��*/
					//memset(*user_image,0,MT9V03X_CSI_W);
					CAMERA_RECEIVER_Start(&cameraReceiver);

					CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, fullCameraBufferAddr);			
					/* �ȴ���ȡ����֡����������ʾ */
					while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr))
					{
					}
					CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr);
					if(fullCameraBufferAddr == (uint32_t)image_csi1[0])
					{
						PRINTF("֡ ok \r\n ");
					}
					CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
			}
			
			if(jpeg_data_ok==1)
			{
					PRINTF("ֹͣ�ɼ� ��ȡͼƬ�� \r\n ");
					p=(uint8_t*)user_image[0];
					jpglen=0;	//����jpg�ļ���СΪ0
					headok=0;	//���jpgͷ���
					
					while(1)//����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С
					{
						if((p[i]==0XFF)&&(p[i+1]==0XD8))//�ҵ�FF D8
						{
							jpgstart=i;
							headok=1;	//����ҵ�jpgͷ(FF D8)
							PRINTF("find jpeg head \r\n ");
						}
						if((p[i]==0XFF)&&(p[i+1]==0XD9)&&headok)//�ҵ�ͷ�Ժ�,����FF D9
						{
							jpglen=i-jpgstart+2;
							
							JPEG_len=jpglen;
							p+=jpgstart;			//ƫ�Ƶ�0XFF,0XD8��
							PRINTF("find jpeg end \r\n ");
							
							PRINTF("start send img buf \r\n ");
							send_jpeg_dev(p,JPEG_len);
							//memset(*user_image,0,sizeof(*user_image));
							PRINTF("img buf send over \r\n ");
							//return p;
							break;
						}
						i++;
						if(i>1024*10)//����һ�� ���ⷢ��Ӳ������
						{
							i=0;
						}
					}	
			}
		
      
	}

}

/****************************END OF FILE**********************/
