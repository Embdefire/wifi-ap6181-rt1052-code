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

/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef unsigned char       uint8;




//ͼ�񻺳���  ����û���Ҫ����ͼ������ ���ͨ��user_image���������ݣ���ò�Ҫֱ�ӷ��ʻ�����
//ALIGN(64) uint8 image_csi1[CSI_H][CSI_W];
//ALIGN(64) uint8 image_csi2[CSI_H][CSI_W];
uint8_t image_csi1[CSI_H][CSI_W];
uint8_t image_csi2[CSI_H][CSI_W];

//�û�����ͼ������ֱ�ӷ������ָ������Ϳ���
//���ʷ�ʽ�ǳ��򵥣�����ֱ��ʹ���±�ķ�ʽ����
//������ʵ�10�� 50�еĵ㣬user_image[10][50]�Ϳ�����
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
 * @brief ����ͷ�ɼ�
 *
 * @return ��
 *   @retval ��
 */
void  Get_Camera_Data_Task(void)
{
	//Camera_Init();
	while (1)
	{
		#if 0

		//CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);
		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, *img_data);
    
		/* �ȴ���ȡ����֡����������ʾ */
		 //CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr);
		
//		CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, img_data);
		

//		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr))
//		{ 
//		}
		
		
		PRINTF("֡���� ok \r\n");
		vTaskDelay(200);
		
		#endif

	}
}




