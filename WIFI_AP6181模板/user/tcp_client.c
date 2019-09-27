
/** @file
 *
 * TCP�ͻ���Ӧ�ó���
 *
 * ��Ӧ�ó���Ƭ����ʾ��������ӵ�Wi-Fi
 * ���粢��TCP������ͨ��
 *
 *����չʾ
 *  - Wi-Fi �ͻ���ģʽ
 *  - DHCP �ͻ���
 *  - TCP���ͺͽ���
 *
 * ����˵��
 *   1.�޸�wifi_config_dct.h��ͷ�ļ��е�
				CLIENT_AP_SSID / CLIENT_AP_PASSPHRASE Wi-Fi
				ƾ����ƥ������Wi-Fi�����

 *   2.ȷ�����ļ�������ӵ���ͬ��Wi-Fi����㡣
 *   3. ȷ��Wi-Fi�ӿڵļ����IP��ַ��
 *   4. ������Ĵ����и���#define TCP_SERVER_IP_ADDRESS��ƥ��������IP��ַ
 *   5. ��PC�ն����ӵ�WICED������Ĵ��ж˿ڣ�Ȼ����WICED����������������Ӧ�ó���
        ��������ָ��
 *   6. ȷ�����ļ�����ϰ�װ��Python 2.7.x��* NOT * 3.x��
 *   7. ��һ������ shell
 *   8. ��tcp_clientĿ¼����python TCP echo��������������ʾ
			c��\ <WICED-SDK> \ Apps \ snip \ tcp_client> 
			c��\ path \ to \ Python27 \ python.exe tcp_echo_server.py
      -ȷ�����ķ���ǽ�ڶ˿�50007������TCP for Python 
 *
			ÿ��TCP_CLIENT_INTERVAL�룬Ӧ�ó���ͻ���Զ��TCP������
			�������ӣ�����һ����Ϣ�� WICED�ʺ򡱣�����Ϊ��Ӧ���ո���
			Ϣ�Ļ��ԡ� ��Ӧ��ӡ�ڴ��п���̨�ϡ�
 *
			����ͨ��wifi_config_dct.h�е�#define WICED_NETWORK_INTERFACE
			����Ҫʹ�õ����硣��ʹ��AP��STAģʽ������£�
			����Ӧ�ظ���AP_SSID��AP_PASSPHRASE��
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

/* ���ķ�����IP��ַ��ƥ��TCP���Է�������ַ */
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
 *               ��������
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
 *               ���ܶ���
 ******************************************************/

void application_start(void)
{
    wiced_interface_t interface;
    wiced_result_t result;

    /* ��ʼ���豸��WICED���*/
    wiced_init( );

    /*��������ӿ�*/
    result = wiced_network_up_default( &interface, &device_init_ip_settings );

    if ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("Bringing up network interface failed !\n") );
    }

    /* ����һ��TCP�׽��� */
    if ( wiced_tcp_create_socket( &tcp_client_socket, interface ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP socket creation failed\n") );
    }

    /* �󶨵��׽��� */
    wiced_tcp_bind( &tcp_client_socket, TCP_SERVER_PORT );

    /* ע�ᷢ��TCP���ݰ��Ĺ��� */
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

    /* ���ӵ�Զ��TCP�����������Լ��� */
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

    /* ����TCP���ݰ��� �Զ�����tx_data���ڴ� */
    if (wiced_packet_create_tcp(&tcp_client_socket, TCP_PACKET_MAX_DATA_LENGTH, &packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet creation failed\n"));
        return WICED_ERROR;
    }

    /*����Ϣд��tx_data"  */
    sprintf(tx_data, "%s", "Hello from WICED\n");

    /*�������ݲ��ֵĽ�β */
    wiced_packet_set_data_end(packet, (uint8_t*)tx_data + strlen(tx_data));

    /* ����TCP���ݰ� */
    if (wiced_tcp_send_packet(&tcp_client_socket, packet) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet send failed\n"));

        /* ɾ�����ݰ�����Ϊ����ʧ�� */
        wiced_packet_delete(packet);

        /* �ر�����*/
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /*�������Է���������Ӧ��������������п���̨ */
    result = wiced_tcp_receive(&tcp_client_socket, &rx_packet, TCP_CLIENT_RECEIVE_TIMEOUT);
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("TCP packet reception failed\n"));

        /* ɾ�����ݰ�����Ϊ����ʧ�� */
        wiced_packet_delete(rx_packet);

        /*  �ر����� */
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /* ��ȡ���յ������ݰ������� */
    wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

    if (rx_data_length != available_data_length)
    {
        WPRINT_APP_INFO(("Fragmented packets not supported\n"));

        /* ɾ�����ݰ�����Ϊ����ʧ��*/
        wiced_packet_delete(rx_packet);

        /* �ر����� */
        wiced_tcp_disconnect(&tcp_client_socket);
        return WICED_ERROR;
    }

    /* ����ֹ���յ����ַ��� */
    rx_data[rx_data_length] = '\x0';
    WPRINT_APP_INFO(("%s", rx_data));

    /* ɾ�����ݰ�����ֹ���� */
    wiced_packet_delete(rx_packet);
    wiced_tcp_disconnect(&tcp_client_socket);

    return WICED_SUCCESS;

}


