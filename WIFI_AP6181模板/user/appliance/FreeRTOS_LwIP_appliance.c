/**
 * @file
 *
 * Home Appliance Control Application
 *
*��Ӧ�ó�����ʾ�����ʹ�ü򵥵���ҳ����
*������ҳ�ϵİ�ťʱ����UART�ṩ��Ϣ�� Ӧ�ó���
*ģ��һ���ǳ��������û����������Ƽ��õ���������ϴ�»�
*�������ɻ���
*
*�ṩ��ʹ��WICEDӦ�ó����ܵĵ�Ч��ʾӦ�ó���
*��<WICED-SDK> / apps / demoĿ¼�С�
 *
*��WWDӦ�ó���ר���������FreeRTOS / LwIP��
*
*Ӧ�ó���ʹ��APģʽ�������ã����ԡ�����ģʽ������
  Ϊ��ʹ�û��ܹ����豸���ӵ���ͥ���磬�豸��ת��Ϊ������ģʽ���Կ�ʼ�������С�
*
*	���ú�Ӧ�ó����������ģʽ������Wi-Fi����
  �㣬DHCP��������������������DNS����Web�������� 
	��ʹWi-Fi�ͻ��ˣ�����ʼǱ����ԣ������ֻ���ƽ����ԣ��������ӡ�
*
*���Ӻ�һ����* any * URL������ҳ�����豸������ҳ
�ͻ��Զ������ڿͻ���Web������С� �ͻ���������
����Wi-Fi���Ӻ󣬴˹��ܿ�ģ�¹����ȵ�Ĳ�����
*
*����ҳ��ʹ�ͻ����ܹ�ʹ��WPS���ţ�
WPS��ť���ֶ�����SSID /����������WICEDģ��
�����ӵ���ͥWi-Fi AP�� ѡ�����÷�����
Ӧ�û�ر�����ģʽ��������ʹ����ѡ����
�÷��������ͥWi-Fi���硣
*
*��ʱ����Ӧ�ó���������ģʽ��
*
*���ӵ���ͥ����󣬸�Ӧ�ó�������һ���豸Web��������
�÷�������ʾ���пɵ�����ť����ҳ�� ���ӵ���WICEDģ����
ͬ�ļ�ͥWi-Fi�����Wi-Fi�ͻ���ʹ��Web��������ӵ���
��Web�������� �豸Ӧ�ó���ǰ��֧��L3���֣�����Bonjour��uPnP����
����б�Ҫ���ͻ���Web�����ָ��WICEDģ���IP��ַ��
����http://192.168.1.100
*
*����/����/��ť������Ϣ����WICEDģ��UART�ϻ�á�
*
 */

#include "wwd_wifi.h"
#include "wwd_management.h"
#include "wwd_network.h"
#include "network/wwd_buffer_interface.h"
#include "platform/wwd_platform_interface.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include <string.h>
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "wwd_wifi.h"
#include "wwd_management.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "platform/wwd_platform_interface.h"
#include "web_server.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include <math.h>
#include "appliance.h"
#include "lwip/prot/dhcp.h"

/******************************************************
 *                      �궨��
 ******************************************************/
/** @cond */
#define MAKE_IPV4_ADDRESS(a, b, c, d)          ((((uint32_t) (a)) << 24) | (((uint32_t) (b)) << 16) | (((uint32_t) (c)) << 8) | ((uint32_t) (d)))
/** @endcond */

/******************************************************
 *                    ����
 ******************************************************/

#define COUNTRY                 WICED_COUNTRY_AUSTRALIA
#define AP_IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,  200 )
#define AP_NETMASK              MAKE_IPV4_ADDRESS( 255, 255, 255,   0 )
#define JOIN_TIMEOUT            (10000)   /** ������������ĳ�ʱʱ�䣨�Ժ���Ϊ��λ��= 10�� */

#define SAMPLES_PER_SEND        (26)
#define TIME_MS_BETWEEN_SAMPLES (1000)
#define DESTINATION_UDP_PORT    (50007)
#define AP_SSID_START           "WICED_Appliance_"
#define AP_PASS                 "12345678"
#define AP_SEC                  WICED_SECURITY_WPA2_AES_PSK  /* WICED_SECURITY_OPEN */
#define AP_CHANNEL              (1)
#define APP_THREAD_STACKSIZE    (5120)
#define DHCP_THREAD_STACKSIZE   (800)
#define WEB_SERVER_STACK_SIZE   (1024)

/******************************************************
 *                   ö��
 ******************************************************/

/******************************************************
 *                 ���Ͷ���
 ******************************************************/

/******************************************************
 *                    �ṹ
 ******************************************************/

/******************************************************
 *              ��̬��������
 ******************************************************/

/******************************************************
 *              ��������
 ******************************************************/
static struct netif wiced_if;
static xTaskHandle startup_thread_handle;

/* �������������ñ��� */
appliance_config_t       appliance_config   = { .config_type = CONFIG_NONE };

/******************************************************
 *             ��̬����ԭ��
 ******************************************************/

//static void startup_thread  ( void *arg );
//static void tcpip_init_done ( void * arg );
static void run_ap_webserver( void );
//static void run_sta_web_server( void );

/******************************************************
 *              ���ܶ���
 ******************************************************/

/**
 * Main appliance app thread
 *
 * Obtains network information and credentials via AP mode web server
 *
 */

void app_main( void )
{
    /* ����Web�������Ի�ȡ���� */

    run_ap_webserver( );


   // run_sta_web_server( );

}

/**
 *  �������磬���ʹ��������ݣ�Ȼ��ֹͣ����
 *
 *�˹��ܿɳ�ʼ��802.11�豸�������������õ����磬
 *����LwIP����ӿڣ�ͨ��DHCP��ȡIP����UDP�㲥����ʽ���͸��������ݣ�
 *�ر�LwIP����ӿڣ��뿪���磬���ر�802.11�豸
 *
 * @param arg : data-����Ҫ���͵����ݵ��ַ�����
 * @param arg : sendlen-�ַ�����ĳ��ȣ����ֽ�Ϊ��λ��
 */

static void run_sta_web_server( void )
{
    ip4_addr_t ipaddr, netmask, gw;
    wwd_result_t result;

    /* ����WICED������802.11�豸�� */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wwd_buffer_init( NULL );
    result = wwd_management_wifi_on( COUNTRY );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }

#ifdef APPLIANCE_ENABLE_WPS
    if ( appliance_config.config_type == CONFIG_WPS_PBC )
    {
        do_wps( WICED_WPS_PBC_MODE, "" );
    }
    else if ( appliance_config.config_type == CONFIG_WPS_PIN )
    {
        do_wps( WICED_WPS_PIN_MODE, appliance_config.vals.wps_pin.pin );
    }
    else
#endif /* ifdef APPLIANCE_ENABLE_WPS */
    {
        if ( appliance_config.config_type == CONFIG_SCANJOIN )
        {
            /* ������������ */
            while ( WWD_SUCCESS != wwd_wifi_join_specific( &appliance_config.vals.scanjoin.scanresult, (uint8_t*) appliance_config.vals.scanjoin.passphrase, appliance_config.vals.scanjoin.passphrase_len, NULL, WWD_STA_INTERFACE ) )
            {
                WPRINT_APP_INFO(("Failed to join .. retrying\n"));
            }
            WPRINT_APP_INFO( ( "Successfully joined Network\n" ) );
        }
    }

    /* ��������ӿ� */
    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

    if ( NULL == netif_add( &wiced_if, &ipaddr, &netmask, &gw, (void*) WWD_STA_INTERFACE, ethernetif_init, ethernet_input ) )
    {
        WPRINT_APP_ERROR( ( "Failed to start network interface\n" ) );
        return;
    }

    netif_set_default( &wiced_if );
    netif_set_up( &wiced_if );

    /* ��DHCPЭ��*/
    WPRINT_APP_INFO(("Obtaining IP address via DHCP\n"));
    struct dhcp netif_dhcp;
    dhcp_set_struct( &wiced_if, &netif_dhcp );
    dhcp_start( &wiced_if );
    while ( netif_dhcp.state != DHCP_STATE_BOUND )
    {
        /* �ȴ� */
        sys_msleep( 10 );
    }

    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));

//    run_webserver( netif_ip4_addr(&wiced_if)->addr, config_STA_url_list );

    WPRINT_APP_INFO( ( "Shutting down WICED\n" ) );

    /* �ٴιر�һ��*/
    dhcp_stop( &wiced_if );
    netif_set_down( &wiced_if );
    netif_remove( &wiced_if );
    wwd_wifi_leave( WWD_STA_INTERFACE );
    wwd_wifi_set_down();
    if ( WWD_SUCCESS != wwd_management_wifi_off( ) )
    {
        WPRINT_APP_ERROR(("WICED de-initialization failed\n"));
    }
}

/**
 * �������ʵ㣬Web��������DHCP������
 *
 *�˹��ܽ���ʼ��802.11�豸������802.11����㣬Ȼ������Web������
 * ��DHCP��������������������ͻ��˽������ӡ�
 */
static void run_ap_webserver( void )
{
    wiced_ssid_t ap_ssid;
    wiced_mac_t  my_mac;
    ip4_addr_t ap_ipaddr;
    ip4_addr_t ap_netmask;
    wwd_result_t result;

    /* ��ʼ��Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wwd_buffer_init( NULL );
    result = wwd_management_wifi_on( COUNTRY );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }

    /* ����SSID*/
    wwd_wifi_get_mac_address( &my_mac, WWD_STA_INTERFACE );
    sprintf( (char*) ap_ssid.value, AP_SSID_START "%02X%02X%02X%02X%02X%02X", my_mac.octet[0], my_mac.octet[1], my_mac.octet[2], my_mac.octet[3], my_mac.octet[4], my_mac.octet[5] );
    ap_ssid.length = strlen( (char*)ap_ssid.value );

    WPRINT_APP_INFO(("Starting Access Point: SSID: %s\n", (char*)ap_ssid.value ));

    /*��������� */
    wwd_wifi_start_ap( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, AP_CHANNEL );

    /* ��������ӿ�*/
    ip4_addr_set_u32(&ap_ipaddr, htonl( AP_IP_ADDR ));
    ip4_addr_set_u32(&ap_netmask, htonl( AP_NETMASK ));

    if ( NULL == netif_add( &wiced_if, &ap_ipaddr, &ap_netmask, &ap_ipaddr, (void*) WWD_AP_INTERFACE, ethernetif_init, ethernet_input ) )
    {
        WPRINT_APP_ERROR( ( "Failed to start network interface\n" ) );
        return;
    }
    netif_set_default( &wiced_if );
    netif_set_up( &wiced_if );

    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));


    /*����DNS��DHCP������ */
    //start_dns_server( ap_ipaddr.addr );
    start_dhcp_server( ap_ipaddr.addr );
		
		while(1)
		{
		
		}
		
//    run_webserver( ap_ipaddr.addr, config_AP_url_list );

//    quit_dhcp_server( );
//    quit_dns_server( );

    /* �ص�һ�� */
    netif_set_down( &wiced_if );
    netif_remove( &wiced_if );

    if ( WWD_SUCCESS != wwd_wifi_stop_ap( ) )
    {
        WPRINT_APP_ERROR(("Failed to stop WICED access point\n"));
    }
/*
    if ( WICED_SUCCESS != wwd_management_wifi_off( ) )
    {
        WINFO_APP_ERROR(("WICED de-initialization failed\n"));
    }
*/
}


