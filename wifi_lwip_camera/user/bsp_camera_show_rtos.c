#include "bsp_camera_show_rtos.h"

#include "./lcd/bsp_camera_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./camera/bsp_ov2640_config.h"
#include "./lcd/bsp_lcd.h"
#include "./systick/bsp_systick.h"
#include "./key/bsp_key.h"

/* FreeRTOSͷ�ļ� */
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

//		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);
//    
//		/* �ȴ���ȡ����֡����������ʾ */
//		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &activeFrameAddr))
//		{
//		}
		PRINTF("֡���� ok \r\n");
		vTaskDelay(200);

	}
}




