
/** @file
 *
 * TCP客户端应用程序
 *
 * 此应用程序片段演示了如何连接到Wi-Fi
 * 网络并与TCP服务器通信
 *
 *功能展示
 *  - Wi-Fi 客户端模式
 *  - DHCP 客户端
 *  - TCP发送和接收
 *
 * 申请说明
 *   1.修改wifi_config_dct.h标头文件中的
				CLIENT_AP_SSID / CLIENT_AP_PASSPHRASE Wi-Fi
				凭据以匹配您的Wi-Fi接入点

 *   2.确保您的计算机连接到相同的Wi-Fi接入点。
 *   3. 确定Wi-Fi接口的计算机IP地址。
 *   4. 在下面的代码中更改#define TCP_SERVER_IP_ADDRESS以匹配计算机的IP地址
 *   5. 将PC终端连接到WICED评估板的串行端口，然后按照WICED中所述构建和下载应用程序
        快速入门指南
 *   6. 确保您的计算机上安装了Python 2.7.x（* NOT * 3.x）
 *   7. 打开一个命令 shell
 *   8. 从tcp_client目录运行python TCP echo服务器，如下所示
			c：\ <WICED-SDK> \ Apps \ snip \ tcp_client> 
			c：\ path \ to \ Python27 \ python.exe tcp_echo_server.py
      -确保您的防火墙在端口50007上允许TCP for Python 
 *
			每隔TCP_CLIENT_INTERVAL秒，应用程序就会与远程TCP服务器
			建立连接，发送一条消息“ WICED问候”，并作为响应接收该消
			息的回显。 响应打印在串行控制台上。
 *
			可以通过wifi_config_dct.h中的#define WICED_NETWORK_INTERFACE
			更改要使用的网络。在使用AP或STA模式的情况下，
			请相应地更改AP_SSID和AP_PASSPHRASE。
 */

#include "wiced.h"
#include "wiced_utilities.h"
/* include platform first so any defines therein can take precedence */
#include "wiced_platform.h"
#include "wiced_framework.h"
#include "wiced_rtos.h"
#include "wiced_tcpip.h"
#include "wiced_time.h"
#include "wiced_wifi.h"
#include "wiced_defaults.h"
#include "wiced_network.h"
#include "wiced_management.h"
#include "wiced_easy_setup.h"
#include "wiced_usb.h"
#include "wiced_xip.h"
#include "wwd_debug.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define TCP_PACKET_MAX_DATA_LENGTH        30
#define TCP_CLIENT_INTERVAL               2
#define TCP_SERVER_PORT                   50007
#define TCP_CLIENT_CONNECT_TIMEOUT        500
#define TCP_CLIENT_RECEIVE_TIMEOUT        300
#define TCP_CONNECTION_NUMBER_OF_RETRIES  3

/* 更改服务器IP地址以匹配TCP回显服务器地址 */
#define TCP_SERVER_IP_ADDRESS MAKE_IPV4_ADDRESS(192,168,1,1)


/******************************************************
 *                    Constants
 ******************************************************/

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

static wiced_result_t tcp_client();

/******************************************************
 *               变量定义
 ******************************************************/

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
};

static wiced_tcp_socket_t  tcp_client_socket;
static wiced_timed_event_t tcp_client_event;

/******************************************************
 *               功能定义
 ******************************************************/

void application_start(void)
{
    wiced_interface_t interface;
    wiced_result_t result;

    /* 初始化设备和WICED框架*/
    wiced_init( );

    /*调出网络接口*/
    result = wiced_network_up_default( &interface, &device_init_ip_settings );

    if ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("Bringing up network interface failed !\n") );
    }

    /* 创建一个TCP套接字 */
    if ( wiced_tcp_create_socket( &tcp_client_socket, interface ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP socket creation failed\n") );
    }

    /* 绑定到套接字 */
    wiced_tcp_bind( &tcp_client_socket, TCP_SERVER_PORT );

    /* 注册发送TCP数据包的功能 */
    wiced_rtos_register_timed_event( &tcp_client_event, WICED_NETWORKING_WORKER_THREAD, &tcp_client, TCP_CLIENT_INTERVAL * SECONDS, 0 );

    WPRINT_APP_INFO(("Connecting to the remote TCP server every %d seconds ...\n", TCP_CLIENT_INTERVAL));

}


wiced_result_t tcp_client( void* arg )
{
    wiced_result_t           result;
    wiced_packet_t*          packet;
    wiced_packet_t*          rx_packet;
    char*                    tx_data;
    char*                    rx_data;
    uint16_t                 rx_data_length;
    uint16_t                 available_data_length;
    const wiced_ip_address_t INITIALISER_IPV4_ADDRESS( server_ip_address, TCP_SERVER_IP_ADDRESS );
    int                      connection_retries;
    UNUSED_PARAMETER( arg );

    /* 连接到远程TCP服务器，尝试几次 */
    connection_retries = 0;
    do
    {
        result = wiced_tcp_connect( &tcp_client_socket, &server_ip_address, TCP_SERVER_PORT, TCP_CLIENT_CONNECT_TIMEOUT );
        connection_retries++;
    }
    while( ( result != WICED_SUCCESS ) && ( connection_retries < TCP_CONNECTION_NUMBER_OF_RETRIES ) );
    if( result != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("Unable to connect to the server! Halt.\n"));
    }

    /* 创建TCP数据包。 自动分配tx_data的内存 */
    if (wiced_packet_create_tcp(&tcp_client_socket, TCP_PACKET_MAX_DATA_LENGTH, &packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet creation failed\n"));
        return WICED_ERROR;
    }

    /*将消息写入tx_data"  */
    sprintf(tx_data, "%s", "Hello from WICED\n");

    /*设置数据部分的结尾 */
    wiced_packet_set_data_end(packet, (uint8_t*)tx_data + strlen(tx_data));

    /* 发送TCP数据包 */
    if (wiced_tcp_send_packet(&tcp_client_socket, packet) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet send failed\n"));

        /* 删除数据包，因为发送失败 */
        wiced_packet_delete(packet);

        /* 关闭连接*/
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /*接收来自服务器的响应并将其输出到串行控制台 */
    result = wiced_tcp_receive(&tcp_client_socket, &rx_packet, TCP_CLIENT_RECEIVE_TIMEOUT);
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("TCP packet reception failed\n"));

        /* 删除数据包，因为接收失败 */
        wiced_packet_delete(rx_packet);

        /*  关闭连接 */
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /* 获取接收到的数据包的内容 */
    wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

    if (rx_data_length != available_data_length)
    {
        WPRINT_APP_INFO(("Fragmented packets not supported\n"));

        /* 删除数据包，因为接收失败*/
        wiced_packet_delete(rx_packet);

        /* 关闭连接 */
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /* 空终止接收到的字符串 */
    rx_data[rx_data_length] = '\x0';
    WPRINT_APP_INFO(("%s", rx_data));

    /* 删除数据包并终止连接 */
    wiced_packet_delete(rx_packet);
    wiced_tcp_disconnect(&tcp_client_socket);

    return WICED_SUCCESS;

}


