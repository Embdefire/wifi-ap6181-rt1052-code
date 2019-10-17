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
#include "img_data_deal.h"


/** @endcond */


/*****************************************************
 *               静态函数声明
 ******************************************************/

static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );

/******************************************************
 *               变量定义
 ******************************************************/

static TaskHandle_t  startup_thread_handle;
static TaskHandle_t Receive_Task_Handle=NULL;
SemaphoreHandle_t BinarySem_Handle =NULL;
extern void Receive_Task(void* parameter);
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
		
//  /* 创建Receive_Task任务 */
//		xTaskCreate((TaskFunction_t )Receive_Task, /* 任务入口函数 */
//													(const char*    )"Receive_Task",/* 任务名字 */
//													(uint16_t       )1024*5,   /* 任务栈大小 */
//													(void*          )NULL,	/* 任务入口函数参数 */
//													(UBaseType_t    )2,	    /* 任务的优先级 */
//													(TaskHandle_t*  )&Receive_Task_Handle);/* 任务控制块指针 */
													
													
	/*创建一个初始线程 */									
		BaseType_t xReturn = pdPASS;
		xReturn = xTaskCreate((TaskFunction_t )startup_thread,  /* 任务入口函数 */
													(const char*    )"app_thread",/* 任务名字 */
													(uint16_t       )1024*5,  /* 任务栈大小 */
													(void*          )NULL,/* 任务入口函数参数 */
													(UBaseType_t    )1, /* 任务的优先级 */
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
 * @brief Receive_Task
 */
extern char *recv_data;
static void Receive_Task(void* parameter)
{	
	
//  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
//	
//  while (1)
//  {
//    //获取二值信号量 xSemaphore,没获取到则一直等待
//		xReturn = xSemaphoreTake(BinarySem_Handle,/* 二值信号量句柄 */
//                              portMAX_DELAY); /* 等待时间 */

//    if(pdTRUE == xReturn)
//		{
//			PRINTF("收到数据:%s\r\n",recv_data);
//		}
//  }
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

