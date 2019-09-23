/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   用V2.3.1版本库建的工程模板
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
#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
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
/******************************************************
 *                      Macros
 ******************************************************/
/** @cond */
#define MAKE_IPV4_ADDRESS(a, b, c, d)          ((((uint32_t) (a)) << 24) | (((uint32_t) (b)) << 16) | (((uint32_t) (c)) << 8) | ((uint32_t) (d)))

/* In release builds all UART printing is suppressed to remove remove printf and malloc dependency which reduces memory usage dramatically */
#ifndef DEBUG
#undef  WPRINT_APP_INFO
#define WPRINT_APP_INFO(args) printf args
#undef  WPRINT_APP_ERROR
#define WPRINT_APP_ERROR(args) printf args
#endif

/** @endcond */

/******************************************************
 *                    Constants
 ******************************************************/

#define AP_SSID              "Embedfire"
#define AP_PASS              "Wildfire2018"
#define AP_SEC               WICED_SECURITY_WPA2_MIXED_PSK

#define COUNTRY              WICED_COUNTRY_AUSTRALIA
#define USE_DHCP             WICED_TRUE
#define IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   31,  128 )  /* Not required if USE_DHCP is WICED_TRUE */
#define GW_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   31,   1 )  /* Not required if USE_DHCP is WICED_TRUE */
#define NETMASK              MAKE_IPV4_ADDRESS( 255, 255, 255,   0 )  /* Not required if USE_DHCP is WICED_TRUE */
/* #define PING_TARGET          MAKE_IPV4_ADDRESS( 192, 168,   1, 2 ) */  /* Uncomment if you want to ping a specific IP instead of the gateway*/

#define PING_RCV_TIMEOUT     (1000)    /** ping receive timeout - in milliseconds */
#define PING_DELAY           (1000)    /** Delay between ping response/timeout and the next ping send - in milliseconds */
#define PING_ID              (0xAFAF)
#define PING_DATA_SIZE       (32)      /** ping additional data size to include in the packet */
#define JOIN_TIMEOUT         (10000)   /** timeout for joining the wireless network in milliseconds  = 10 seconds */
#define APP_THREAD_STACKSIZE (5120)


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
extern void netio_init(void);
static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len );
static err_t ping_recv( int socket_hnd );
static err_t ping_send( int s, ip4_addr_t *addr );
static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );
static void app_main( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint16_t     ping_seq_num;
static struct netif wiced_if;
static TaskHandle_t  startup_thread_handle;

static const wiced_ssid_t ap_ssid =
{
    .length = sizeof(AP_SSID)-1,
    .value  = AP_SSID,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Main Ping app
 *
 * Initialises Wiced, joins a wireless network, then periodically pings the specified IP address forever.
 */

static void app_main( void )
{
    ip4_addr_t ipaddr, netmask, gw;
    ip4_addr_t target;
    int socket_hnd;
    wwd_time_t send_time;
    int recv_timeout = PING_RCV_TIMEOUT;
    wwd_result_t result;

    /* Initialise Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wwd_buffer_init( NULL);
    result = wwd_management_wifi_on( COUNTRY );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }

    /* Attempt to join the Wi-Fi network */
    WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
    while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
    }
    WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

    /* Setup IP configuration */
    if ( USE_DHCP == WICED_TRUE )
    {
        ip4_addr_set_zero( &gw );
        ip4_addr_set_zero( &ipaddr );
        ip4_addr_set_zero( &netmask );
    }
    else
    {
        ipaddr.addr  = htonl( IP_ADDR );
        gw.addr      = htonl( GW_ADDR );
        netmask.addr = htonl( NETMASK );
    }

    if ( NULL == netif_add( &wiced_if, &ipaddr, &netmask, &gw, (void*) WWD_STA_INTERFACE, ethernetif_init, ethernet_input ) )
    {
        WPRINT_APP_ERROR(( "Could not add network interface\n" ));
        return;
    }

    netif_set_up( &wiced_if );

    if ( USE_DHCP == WICED_TRUE )
    {
        struct dhcp netif_dhcp;
        WPRINT_APP_INFO(("Obtaining IP address via DHCP\n"));
        dhcp_set_struct( &wiced_if, &netif_dhcp );
        dhcp_start( &wiced_if );
        while ( netif_dhcp.state != DHCP_STATE_BOUND )
        {
            /* wait */
            sys_msleep( 10 );
        }
    }

    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));

    /* Open a local socket for pinging */
    if ( ( socket_hnd = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP ) ) < 0 )
    {
        WPRINT_APP_ERROR(( "unable to create socket for Ping\n" ));
        return;
    }

    /* Set the receive timeout on local socket so pings will time out. */
    lwip_setsockopt( socket_hnd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof( recv_timeout ) );

#ifdef PING_TARGET
        target.addr = htonl( PING_TARGET );
#else /* ifdef PING_TARGET */
        target.addr = netif_ip4_gw(&wiced_if)->addr;
#endif /* ifdef PING_TARGET */

//    WPRINT_APP_INFO(("Pinging: %s\n", ip4addr_ntoa( &target )));
		netio_init();
		
    /* Loop forever */
    while ( 1 )
    {
//        err_t result;

//        /* Send a ping */
//        if ( ping_send( socket_hnd, &target ) != ERR_OK )
//        {
//            WPRINT_APP_ERROR(( "Unable to send Ping\n" ));
//            return;
//        }

//        /* Record time ping was sent */
//        send_time = host_rtos_get_time( );

//        /* Wait for ping reply */
//        result = ping_recv( socket_hnd );
//        if ( ERR_OK == result )
//        {
//            WPRINT_APP_INFO(("Ping Reply %dms\n", (int)( host_rtos_get_time( ) - send_time ) ));
//        }
//        else
//        {
//            WPRINT_APP_INFO(("Ping timeout\n"));
//        }

        /* Sleep until time for next ping */
        sys_msleep( PING_DELAY );
    }

    /* Shutdown code - not used due to infinite loop */
#if 0
    WPRINT_APP_INFO(("Closing down\n"));
    lwip_close( socket_hnd );
    if ( USE_DHCP == WICED_TRUE )
    {
        dhcp_stop( &wiced_if );
    }
    netif_set_down( &wiced_if );
    netif_remove( &wiced_if );
    wwd_wifi_leave( );
    if ( WWD_SUCCESS != wwd_management_deinit( ) )
    {
        WPRINT_APP_ERROR(("WICED de-initialization failed\n"));
    }
#endif /* if 0 */
}



/**
 *  Prepare the contents of an echo ICMP request packet
 *
 *  @param iecho  : Pointer to an icmp_echo_hdr structure in which the ICMP packet will be constructed
 *  @param len    : The length in bytes of the packet buffer passed to the iecho parameter
 *
 */

static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len )
{
    int i;

    ICMPH_TYPE_SET( iecho, ICMP_ECHO );
    ICMPH_CODE_SET( iecho, 0 );
    iecho->chksum = 0;
    iecho->id = PING_ID;
    iecho->seqno = htons( ++ping_seq_num );

    /* fill the additional data buffer with some data */
    for ( i = 0; i < PING_DATA_SIZE; i++ )
    {
        ( (char*) iecho )[sizeof(struct icmp_echo_hdr) + i] = i;
    }

    iecho->chksum = inet_chksum( iecho, len );
}



/**
 *  Send a Ping
 *
 *  Sends a ICMP echo request (Ping) to the specified IP address, using the specified socket.
 *
 *  @param socket_hnd : The handle for the local socket through which the ping request will be sent
 *  @param addr       : The IP address to which the ping request will be sent
 *
 *  @return  ERR_OK if successfully sent, ERR_MEM if out of memory or ERR_VAL otherwise
 */

static err_t ping_send( int socket_hnd, ip4_addr_t *addr )
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    /* Allocate memory for packet */
    if ( !( iecho = (struct icmp_echo_hdr*) mem_malloc( ping_size ) ) )
    {
        return ERR_MEM;
    }

    /* Construct ping request */
    ping_prepare_echo( iecho, ping_size );

    /* Send the ping request */
    to.sin_len = sizeof( to );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->addr;

    err = lwip_sendto( socket_hnd, iecho, ping_size, 0, (struct sockaddr*) &to, sizeof( to ) );

    /* free packet */
    mem_free( iecho );

    return ( err ? ERR_OK : ERR_VAL );
}


/**
 *  Receive a Ping reply
 *
 *  Waits for a ICMP echo reply (Ping reply) to be received using the specified socket. Compares the
 *  sequence number, and ID number to the last ping sent, and if they match, returns ERR_OK, indicating
 *  a valid ping response.
 *
 *  @param socket_hnd : The handle for the local socket through which the ping reply will be received
 *
 *  @return  ERR_OK if valid reply received, ERR_TIMEOUT otherwise
 */

static err_t ping_recv( int socket_hnd )
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    while ( ( len = lwip_recvfrom( socket_hnd, buf, sizeof( buf ), 0, (struct sockaddr*) &from, (socklen_t*) &fromlen ) ) > 0 )
    {
        if ( len >= (int) ( sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) ) )
        {
            iphdr = (struct ip_hdr *) buf;
            iecho = (struct icmp_echo_hdr *) ( buf + ( IPH_HL( iphdr ) * 4 ) );

            if ( ( iecho->id == PING_ID ) &&
                 ( iecho->seqno == htons( ping_seq_num ) ) &&
                 ( ICMPH_TYPE( iecho ) == ICMP_ER ) )
            {
                return ERR_OK; /* Echo reply received - return success */
            }
        }
    }

    return ERR_TIMEOUT; /* No valid echo reply received before timeout */
}

static void BOARD_USDHCClockConfiguration(void)
{
    /*configure system pll PFD2 fractional divider to 18*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 1U);
}



/**
 *  Main function - creates an initial thread then starts FreeRTOS
 *  Called from the crt0 _start function
 *
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
		/* 初始化以太网模块时钟 */
		//BOARD_InitModuleClock();
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
    //delay();
    GPIO_WritePinOutput(GPIO1, 9, 1);
	/* Create an initial thread */
    xTaskCreate(startup_thread, "app_thread", APP_THREAD_STACKSIZE/sizeof( portSTACK_TYPE ), NULL, DEFAULT_THREAD_PRIO, &startup_thread_handle);

    /* Start the FreeRTOS scheduler - this call should never return */
    vTaskStartScheduler( );

    /* Should never get here, unless there is an error in vTaskStartScheduler */
    WPRINT_APP_ERROR(("Main() function returned - error" ));
    return 0;
}



/**
 *  Initial thread function - Starts LwIP and calls app_main
 *
 *  This function starts up LwIP using the tcpip_init function, then waits on a semaphore
 *  until LwIP indicates that it has started by calling the callback @ref tcpip_init_done.
 *  Once that has been done, the @ref app_main function of the app is called.
 *
 * @param arg : Unused - required for conformance to thread function prototype
 */

static void startup_thread( void *arg )
{
    SemaphoreHandle_t lwip_done_sema;
    UNUSED_PARAMETER( arg);

    WPRINT_APP_INFO( ( "\nPlatform " PLATFORM " initialised\n" ) );
    WPRINT_APP_INFO( ( "Started FreeRTOS" FreeRTOS_VERSION "\n" ) );
    WPRINT_APP_INFO( ( "Starting LwIP " LwIP_VERSION "\n" ) );

    /* Create a semaphore to signal when LwIP has finished initialising */
    lwip_done_sema = xSemaphoreCreateCounting( 1, 0 );
    if ( lwip_done_sema == NULL )
    {
        /* could not create semaphore */
        WPRINT_APP_ERROR(("Could not create LwIP init semaphore"));
        return;
    }

    /* Initialise LwIP, providing the callback function and callback semaphore */
    tcpip_init( tcpip_init_done, (void*) &lwip_done_sema );
    xSemaphoreTake( lwip_done_sema, portMAX_DELAY );
    vQueueDelete( lwip_done_sema );

    /* Run the main application function */
    app_main( );

    /* Clean up this startup thread */
    vTaskDelete( startup_thread_handle );
}



/**
 *  LwIP init complete callback
 *
 *  This function is called by LwIP when initialisation has finished.
 *  A semaphore is posted to allow the startup thread to resume, and to run the app_main function
 *
 * @param arg : the handle for the semaphore to post (cast to a void pointer)
 */

static void tcpip_init_done( void * arg )
{
    SemaphoreHandle_t * lwip_done_sema = (SemaphoreHandle_t *) arg;
    xSemaphoreGive( *lwip_done_sema );
}


//#include "lwip/opt.h"

//#if LWIP_IPV4 && LWIP_RAW && LWIP_SOCKET

//#include "ping/ping.h"
//#include "lwip/tcpip.h"
//#include "netif/ethernet.h"
//#include "ethernetif.h"

//#include "board.h"

//#include "pin_mux.h"
//#include "clock_config.h"
//#include "fsl_gpio.h"
//#include "fsl_iomuxc.h"
///*******************************************************************************
// * Definitions
// ******************************************************************************/
///* IP address configuration. */
//#define configIP_ADDR0 192
//#define configIP_ADDR1 168
//#define configIP_ADDR2 31
//#define configIP_ADDR3 172

///* Netmask configuration. */
//#define configNET_MASK0 255
//#define configNET_MASK1 255
//#define configNET_MASK2 255
//#define configNET_MASK3 0

///* Gateway address configuration. */
//#define configGW_ADDR0 192
//#define configGW_ADDR1 168
//#define configGW_ADDR2 31
//#define configGW_ADDR3 1

///* MAC address configuration. */
//#define configMAC_ADDR                     \
//    {                                      \
//        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
//    }

///* Address of PHY interface. */
//#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

///* System clock name. */
//#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk


///*! @brief Stack size of the temporary lwIP initialization thread. */
//#define INIT_THREAD_STACKSIZE 1024

///*! @brief Priority of the temporary lwIP initialization thread. */
//#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

///*******************************************************************************
//* Prototypes
//******************************************************************************/

///*******************************************************************************
//* Variables
//******************************************************************************/

///*******************************************************************************
// * Code
// ******************************************************************************/
//void BOARD_InitModuleClock(void)
//{
//    const clock_enet_pll_config_t config = {true, false, 1};
//    CLOCK_InitEnetPll(&config);
//}

//void delay(void)
//{
//    volatile uint32_t i = 0;
//    for (i = 0; i < 1000000; ++i)
//    {
//        __asm("NOP"); /* delay */
//    }
//}



///*!
// * @brief Initializes lwIP stack.
// */
//static void stack_init(void *arg)
//{
//    static struct netif fsl_netif0;
//    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
//    ethernetif_config_t fsl_enet_config0 = {
//        .phyAddress = EXAMPLE_PHY_ADDRESS,
//        .clockName = EXAMPLE_CLOCK_NAME,
//        .macAddress = configMAC_ADDR,
//    };

//    LWIP_UNUSED_ARG(arg);

//    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
//    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
//    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

//    tcpip_init(NULL, NULL);

//    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw,
//              &fsl_enet_config0, ethernetif0_init, tcpip_input);
//    netif_set_default(&fsl_netif0);
//    netif_set_up(&fsl_netif0);

//    PRINTF("\r\n************************************************\r\n");
//    PRINTF(" PING example\r\n");
//    PRINTF("************************************************\r\n");
//    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_ipaddr)[0], ((u8_t *)&fsl_netif0_ipaddr)[1],
//           ((u8_t *)&fsl_netif0_ipaddr)[2], ((u8_t *)&fsl_netif0_ipaddr)[3]);
//    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_netmask)[0], ((u8_t *)&fsl_netif0_netmask)[1],
//           ((u8_t *)&fsl_netif0_netmask)[2], ((u8_t *)&fsl_netif0_netmask)[3]);
//    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
//           ((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]);
//    PRINTF("************************************************\r\n");

//    ping_init(&fsl_netif0_gw);

//    vTaskDelete(NULL);
//}
///*******************************************************************
// * Code
// *******************************************************************/

///**
//  * @brief  主函数
//  * @param  无
//  * @retval 无
//  */
//int main(void)
//{
//	  gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

//    /* 初始化内存保护单元 */
//    BOARD_ConfigMPU();
//    /* 初始化开发板引脚 */
//    BOARD_InitPins();
//    /* 初始化开发板时钟 */
//    BOARD_BootClockRUN();
//    /* 初始化调试串口 */
//    BOARD_InitDebugConsole();
//		BOARD_InitModuleClock();
//		
//    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
//		/* 初始化PHY RESET引脚 */
//    GPIO_PinInit(GPIO1, 9, &gpio_config);
//		/* 初始化PHY INT引脚 */
//    GPIO_PinInit(GPIO1, 10, &gpio_config);
//    /* 在复位之前ENET_INT输出高电平 */
//    GPIO_WritePinOutput(GPIO1, 10, 1);
//    GPIO_WritePinOutput(GPIO1, 9, 0);
//    delay();
//    GPIO_WritePinOutput(GPIO1, 9, 1);
//		
//    /* 进程中初始化lwIP */
//    if(sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
//        LWIP_ASSERT("main(): Task creation failed.", 0);

//    vTaskStartScheduler();

//    /* Will not get here unless a task calls vTaskEndScheduler ()*/
//    return 0;
//}
//#endif  
/****************************END OF FILE**********************/
