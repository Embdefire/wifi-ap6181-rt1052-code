/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   i.MX RT + FreeRTOS + LWIP + AP6181��wifi��
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************
  */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "wwd_network.h"
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_buffer_interface.h"

#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "./led/bsp_led.h" 
#include "wifi_base_config.h"
#include "img_data_deal.h"


/** @endcond */


/*****************************************************
 *               ��̬��������
 ******************************************************/

static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );

/******************************************************
 *               ��������
 ******************************************************/

static TaskHandle_t  startup_thread_handle;
static TaskHandle_t Receive_Task_Handle=NULL;
SemaphoreHandle_t BinarySem_Handle =NULL;
extern void Receive_Task(void* parameter);
/******************************************************
 *               ���ܶ���
 ******************************************************/

static void BOARD_USDHCClockConfiguration(void)
{
    /*��ϵͳpll PFD2������Ƶ������Ϊ18*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* ����USDHCʱ��Դ�ͷ�Ƶ��*/
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 1U);
}

/**
 * ������
 */

int main( void )
{

    /* ��ʼ���ڴ����Ԫ */
    BOARD_ConfigMPU();
    /* ��ʼ������������ */
    BOARD_InitPins();
    /* ��ʼ��������ʱ�� */
    BOARD_BootClockRUN();
    /* ��ʼ��SDIOʱ�� */
    BOARD_USDHCClockConfiguration();
    /* ��ʼ�����Դ��� */
    BOARD_InitDebugConsole();
    /* ��ʼ��LED */
    LED_GPIO_Config();
    /* ����SDIO�ж����ȼ� */
    NVIC_SetPriority(USDHC2_IRQn, 5U);

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
		
//  /* ����Receive_Task���� */
//		xTaskCreate((TaskFunction_t )Receive_Task, /* ������ں��� */
//													(const char*    )"Receive_Task",/* �������� */
//													(uint16_t       )1024*5,   /* ����ջ��С */
//													(void*          )NULL,	/* ������ں������� */
//													(UBaseType_t    )2,	    /* ��������ȼ� */
//													(TaskHandle_t*  )&Receive_Task_Handle);/* ������ƿ�ָ�� */
													
													
	/*����һ����ʼ�߳� */									
		BaseType_t xReturn = pdPASS;
		xReturn = xTaskCreate((TaskFunction_t )startup_thread,  /* ������ں��� */
													(const char*    )"app_thread",/* �������� */
													(uint16_t       )1024*5,  /* ����ջ��С */
													(void*          )NULL,/* ������ں������� */
													(UBaseType_t    )1, /* ��������ȼ� */
													(TaskHandle_t*  )&startup_thread_handle);/* ������ƿ�ָ�� */ 
													
													
		 /* ����������� */           
		if(pdPASS == xReturn)
			vTaskStartScheduler();   /* �������񣬿������� */
		else
			return -1;  
    /* ����vTaskStartScheduler�г��ִ��󣬷�����Զ��Ҫ������ */
    WPRINT_APP_ERROR(("Main() function returned - error" ));
    return 0;
}



/**
 *  ��ʼ�̹߳���-����LwIP������app_main
 * �ú���ʹ��tcpip_init��������LwIP��Ȼ��ȴ��ź�����
 * ֱ��LwIPͨ�����ûص�@ref tcpip_init_doneָʾ����
 * ��������ɺ󣬽�����Ӧ�ó����@ref app_main������
 * @param arg :  δʹ��-�����̹߳���ԭ������
 */

static void startup_thread( void *arg )
{
    SemaphoreHandle_t lwip_done_sema;
    UNUSED_PARAMETER( arg);

    WPRINT_APP_INFO( ( "\nPlatform " PLATFORM " initialised\n" ) );
    WPRINT_APP_INFO( ( "Started FreeRTOS" FreeRTOS_VERSION "\n" ) );
    WPRINT_APP_INFO( ( "Starting LwIP " LwIP_VERSION "\n" ) );

    /* �����ź�������LwIP��ɳ�ʼ��ʱ�����ź� */
    lwip_done_sema = xSemaphoreCreateCounting( 1, 0 );
    if ( lwip_done_sema == NULL )	
    {
        /*�޷������ź��� */
        WPRINT_APP_ERROR(("Could not create LwIP init semaphore"));
        return;
    }

    /*��ʼ��LwIP���ṩ�ص������ͻص��ź��� */
    tcpip_init( tcpip_init_done, (void*) &lwip_done_sema );
    xSemaphoreTake( lwip_done_sema, portMAX_DELAY );
    vQueueDelete( lwip_done_sema );

    /* ������Ӧ�ó����� */
    app_main( );

    /* ����������߳�*/
    vTaskDelete( startup_thread_handle );
}
/**
 * @brief Receive_Task
 */
extern char *recv_data;
static void Receive_Task(void* parameter)
{	
	
//  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
//	
//  while (1)
//  {
//    //��ȡ��ֵ�ź��� xSemaphore,û��ȡ����һֱ�ȴ�
//		xReturn = xSemaphoreTake(BinarySem_Handle,/* ��ֵ�ź������ */
//                              portMAX_DELAY); /* �ȴ�ʱ�� */

//    if(pdTRUE == xReturn)
//		{
//			PRINTF("�յ�����:%s\r\n",recv_data);
//		}
//  }
}



/**
 *  LwIP��ʼ����ɻص�
 * @param arg : the handle for the semaphore to post (cast to a void pointer)
 */

static void tcpip_init_done( void * arg )
{
    SemaphoreHandle_t * lwip_done_sema = (SemaphoreHandle_t *) arg;
    xSemaphoreGive( *lwip_done_sema );
}

