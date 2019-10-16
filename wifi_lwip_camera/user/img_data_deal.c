#include "img_data_deal.h"


/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

uint8_t image_csi1[CSI_H][ CSI_W];
uint8_t (*user_image)[CSI_W];

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
uint32_t fullCameraBufferAddr;    //�ɼ���ɵĻ�������ַ 
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
volatile uint32_t JPEG_len=0; //��Ƭ����

void CSI_IRQHandler(void)
{
		
    CSI_DriverIRQHandler();		
		CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
		jpeg_data_ok=1;
}


void img_sed_uart()
{
	uint32_t i,jpgstart,jpglen; 
	uint8_t *p;
	uint8_t headok=0;	
	if(jpeg_data_ok==0)
			{
							/* ��ʼ�ɼ�ͼ��*/
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



//int main(void)
//{

//	
//	/* ��ʼ���ڴ汣����Ԫ */
//	BOARD_ConfigMPU();
//	/* ��ʼ������������ */
//	BOARD_InitPins();
//	/* ��ʼ��������ʱ�� */
//	BOARD_BootClockRUN();
//	/* ��ʼ�����Դ��� */
//	BOARD_InitDebugConsole();


//	/* ��ӡϵͳʱ�� */
//	PRINTF("\r\n");
//	PRINTF("*****��ӭʹ�� Ұ��i.MX RT1052 ������*****\r\n");
//	PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
//	PRINTF("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
//	PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
//	PRINTF("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
//	PRINTF("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
//	PRINTF("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
//	PRINTF("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
//	PRINTF("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
//	PRINTF("CSI RGB565 example start...\r\n");

//	/* �����ʼ��*/
//	Camera_Init();

//	while(1)
//	{	
//		img_sed_uart(); 
//	}

//}
