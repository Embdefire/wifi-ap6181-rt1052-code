#include "wifi_base_config.h"
#include "appliance.h"

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "debug.h"
#include "fsl_iomuxc.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "./led/bsp_led.h"
#include "tcpecho.h"


/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */

static TaskHandle_t Receive_Task_Handle=NULL;

/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */
 
SemaphoreHandle_t BinarySem_Handle =NULL;
/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */
extern char *recv_data;

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */
static void Receive_Task(void* parameter);

extern void TCPIP_Init(void);


/**
 * @brief app_main
 *
 */
void app_main( void )
{
		/*����wifi lwip��Ϣ*/
		Config_WIFI_LwIP_Info();
	
	  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
		
		/* ������Ӳ����ʼ�� */
		LED_GPIO_Config();

		/* ����AppTaskCreate���� */
		xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
													(const char*    )"AppTaskCreate",/* �������� */
													(uint16_t       )512,  /* ����ջ��С */
													(void*          )NULL,/* ������ں������� */
													(UBaseType_t    )1, /* ��������ȼ� */
													(TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
		/* ����������� */           
		if(pdPASS == xReturn)
			vTaskStartScheduler();   /* �������񣬿������� */
	
  /* ��������ִ�е����� */    
    while (1)
    {
    }

}



/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{

	/* ���� BinarySem */
	BinarySem_Handle = xSemaphoreCreateBinary();	 
  
  tcpecho_init();

  taskENTER_CRITICAL();           //�����ٽ���
 

  /* ����Receive_Task���� */
		xTaskCreate((TaskFunction_t )Receive_Task, /* ������ں��� */
													(const char*    )"Receive_Task",/* �������� */
													(uint16_t       )512,   /* ����ջ��С */
													(void*          )NULL,	/* ������ں������� */
													(UBaseType_t    )2,	    /* ��������ȼ� */
													(TaskHandle_t*  )&Receive_Task_Handle);/* ������ƿ�ָ�� */
  
													
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}




/**
 * @brief Receive_Task
 */

static void Receive_Task(void* parameter)
{	

  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
  while (1)
  {
    //��ȡ��ֵ�ź��� xSemaphore,û��ȡ����һֱ�ȴ�
		xReturn = xSemaphoreTake(BinarySem_Handle,/* ��ֵ�ź������ */
                              portMAX_DELAY); /* �ȴ�ʱ�� */

    if(pdTRUE == xReturn)
		{
			PRINTF("�յ�����:%s\r\n",recv_data);
		}
  }
}

/********************************END OF FILE****************************/




