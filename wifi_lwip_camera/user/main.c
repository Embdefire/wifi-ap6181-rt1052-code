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

/** @endcond */

#define MT9V03X_CSI_H 1
#define MT9V03X_CSI_W 1024*1024*2

uint8_t image_csi1[MT9V03X_CSI_H][MT9V03X_CSI_W];
uint8_t (*user_image)[MT9V03X_CSI_W];



/*****************************************************
 *               ��̬��������
 ******************************************************/

static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );

/******************************************************
 *               ��������
 ******************************************************/

static TaskHandle_t  startup_thread_handle;

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
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

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
    /* ��ʼ��PHY RESET���� */
    GPIO_PinInit(GPIO1, 9, &gpio_config);
    /* ��ʼ��PHY INT���� */
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* �ڸ�λ֮ǰENET_INT����ߵ�ƽ */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    GPIO_WritePinOutput(GPIO1, 9, 1);
    /*����һ����ʼ�߳� */									
		BaseType_t xReturn = pdPASS;
		xReturn = xTaskCreate((TaskFunction_t )startup_thread,  /* ������ں��� */
													(const char*    )"app_thread",/* �������� */
													(uint16_t       )APP_THREAD_STACKSIZE/sizeof( portSTACK_TYPE ),  /* ����ջ��С */
													(void*          )NULL,/* ������ں������� */
													(UBaseType_t    )DEFAULT_THREAD_PRIO, /* ��������ȼ� */
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
 *  LwIP��ʼ����ɻص�
 * @param arg : the handle for the semaphore to post (cast to a void pointer)
 */

static void tcpip_init_done( void * arg )
{
    SemaphoreHandle_t * lwip_done_sema = (SemaphoreHandle_t *) arg;
    xSemaphoreGive( *lwip_done_sema );
}



#if 0

#include <stdio.h>
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
//#include "./lcd/bsp_camera_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./camera/bsp_ov2640_config.h"
//#include "./lcd/bsp_lcd.h"
#include "./systick/bsp_systick.h"
#include "./key/bsp_key.h"

#define MT9V03X_CSI_H 1
//#define MT9V03X_CSI_W 1024*1024*2
#define MT9V03X_CSI_W 1024

uint8_t image_csi1[MT9V03X_CSI_H][MT9V03X_CSI_W];
uint8_t (*user_image)[MT9V03X_CSI_W];

//char jpeg_data[1024*1024*10];


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

#endif 