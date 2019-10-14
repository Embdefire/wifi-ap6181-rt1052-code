/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   i.MX RT + FreeRTOS + LWIP + AP6181（wifi）
  ******************************************************************
  * @attention
  *
  * 实验平台:野火  i.MXRT1052开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
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
 *               静态函数声明
 ******************************************************/

static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );

/******************************************************
 *               变量定义
 ******************************************************/

static TaskHandle_t  startup_thread_handle;

/******************************************************
 *               功能定义
 ******************************************************/

static void BOARD_USDHCClockConfiguration(void)
{
    /*将系统pll PFD2分数分频器配置为18*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* 配置USDHC时钟源和分频器*/
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 1U);
}

/**
 * 主函数
 */

int main( void )
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    /* 初始化内存管理单元 */
    BOARD_ConfigMPU();
    /* 初始化开发板引脚 */
    BOARD_InitPins();
    /* 初始化开发板时钟 */
    BOARD_BootClockRUN();
    /* 初始化SDIO时钟 */
    BOARD_USDHCClockConfiguration();
    /* 初始化调试串口 */
    BOARD_InitDebugConsole();
    /* 初始化LED */
    LED_GPIO_Config();
    /* 设置SDIO中断优先级 */
    NVIC_SetPriority(USDHC2_IRQn, 5U);

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
    /* 初始化PHY RESET引脚 */
    GPIO_PinInit(GPIO1, 9, &gpio_config);
    /* 初始化PHY INT引脚 */
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* 在复位之前ENET_INT输出高电平 */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    GPIO_WritePinOutput(GPIO1, 9, 1);
    /*创建一个初始线程 */									
		BaseType_t xReturn = pdPASS;
		xReturn = xTaskCreate((TaskFunction_t )startup_thread,  /* 任务入口函数 */
													(const char*    )"app_thread",/* 任务名字 */
													(uint16_t       )APP_THREAD_STACKSIZE/sizeof( portSTACK_TYPE ),  /* 任务栈大小 */
													(void*          )NULL,/* 任务入口函数参数 */
													(UBaseType_t    )DEFAULT_THREAD_PRIO, /* 任务的优先级 */
													(TaskHandle_t*  )&startup_thread_handle);/* 任务控制块指针 */ 
		 /* 启动任务调度 */           
		if(pdPASS == xReturn)
			vTaskStartScheduler();   /* 启动任务，开启调度 */
		else
			return -1;  
    /* 除非vTaskStartScheduler中出现错误，否则永远不要到这里 */
    WPRINT_APP_ERROR(("Main() function returned - error" ));
    return 0;
}



/**
 *  初始线程功能-启动LwIP并调用app_main
 * 该函数使用tcpip_init函数启动LwIP，然后等待信号量，
 * 直到LwIP通过调用回调@ref tcpip_init_done指示其已
 * 启动。完成后，将调用应用程序的@ref app_main函数。
 * @param arg :  未使用-符合线程功能原型所需
 */

static void startup_thread( void *arg )
{
    SemaphoreHandle_t lwip_done_sema;
    UNUSED_PARAMETER( arg);

    WPRINT_APP_INFO( ( "\nPlatform " PLATFORM " initialised\n" ) );
    WPRINT_APP_INFO( ( "Started FreeRTOS" FreeRTOS_VERSION "\n" ) );
    WPRINT_APP_INFO( ( "Starting LwIP " LwIP_VERSION "\n" ) );

    /* 创建信号量以在LwIP完成初始化时发出信号 */
    lwip_done_sema = xSemaphoreCreateCounting( 1, 0 );
    if ( lwip_done_sema == NULL )	
    {
        /*无法创建信号量 */
        WPRINT_APP_ERROR(("Could not create LwIP init semaphore"));
        return;
    }

    /*初始化LwIP，提供回调函数和回调信号量 */
    tcpip_init( tcpip_init_done, (void*) &lwip_done_sema );
    xSemaphoreTake( lwip_done_sema, portMAX_DELAY );
    vQueueDelete( lwip_done_sema );

    /* 运行主应用程序功能 */
    app_main( );

    /* 清理此启动线程*/
    vTaskDelete( startup_thread_handle );
}



/**
 *  LwIP初始化完成回调
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

#endif 