#include "ping.h"

ip4_addr_t target;
int socket_hnd;
wwd_time_t send_time;
static uint16_t     ping_seq_num;
extern void netio_init(void);
ip4_addr_t ipaddr, netmask, gw;

#include "ping.h"
#include "lwip/timeouts.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
//#include "ethernetif.h"
#include "sys_arch.h"
#include "lwip/timeouts.h"

#include "netif/ethernet.h"
/**
 *  ׼������ICMP�������ݰ�������
 *
 *  @param iecho  : ָ��icmp_echo_hdr�ṹ��ָ�룬�������й���ICMP���ݰ�
 *  @param len    : ���ݸ�iecho���������ݰ����������ֽڳ���
 *
 */

void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len )
{
    int i;

    ICMPH_TYPE_SET( iecho, ICMP_ECHO );
    ICMPH_CODE_SET( iecho, 0 );
    iecho->chksum = 0;
    iecho->id = PING_ID;
    iecho->seqno = htons( ++ping_seq_num );

    /* ��һЩ��������������ݻ����� */
    for ( i = 0; i < PING_DATA_SIZE; i++ )
    {
        ( (char*) iecho )[sizeof(struct icmp_echo_hdr) + i] = i;
    }

    iecho->chksum = inet_chksum( iecho, len );
}



/**
 *  ����һ��Ping
 *
 * ʹ��ָ�����׽��ֽ�ICMP��������Ping�����͵�ָ����IP��ַ��
 *
 *  @param socket_hnd :��ͨ���䷢��ping����ı����׽��ֵľ��
 *  @param addr       :�����䷢��ping�����IP��ַ
 *
 *  @return ����ɹ����ͣ��򷵻�ERR_OK������ڴ治�㣬�򷵻�ERR_MEM�����򣬷���ERR_VAL
 */
err_t ping_send( int socket_hnd, ip4_addr_t *addr )
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    /* Ϊ���ݰ������ڴ�*/
    if ( !( iecho = (struct icmp_echo_hdr*) mem_malloc( ping_size ) ) )
    {
        return ERR_MEM;
    }

    /* ����ping���� */
    ping_prepare_echo( iecho, ping_size );

    /*����ping���� */
    to.sin_len = sizeof( to );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->addr;

    err = lwip_sendto( socket_hnd, iecho, ping_size, 0, (struct sockaddr*) &to, sizeof( to ) );

    /* �ͷ� �� */
    mem_free( iecho );

    return ( err ? ERR_OK : ERR_VAL );
}


/**
 *  �յ�Ping�ظ�
 *  �ȴ�ʹ��ָ�����׽��ֽ���ICMP���Դ𸴣�Ping�𸴣��� �����кź�ID������
 *  �η��͵�ping���бȽϣ����ƥ�䣬�򷵻�ERR_OK����ʾ��Ч��ping��Ӧ��
 *  @param socket_hnd : �����׽��ֵľ����ͨ���þ�����յ�ping��
 *
 *  @return  ����յ���Ч�𸴣���ΪERR_OK������ΪERR_TIMEOUT
 */
err_t ping_recv( int socket_hnd )
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
                return ERR_OK; /*�յ������ظ�-�ɹ����� */
            }
        }
    }

    return ERR_TIMEOUT; /* ��ʱǰδ�յ���Ч�Ļ����ظ� */
}

/*
����wifi lwip��Ϣ
*/
void Config_WIFI_LwIP_Info()
{
    int recv_timeout = PING_RCV_TIMEOUT;
    wwd_result_t result;

    /* ��ʼ�� Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wwd_buffer_init( NULL);
    result = wwd_management_wifi_on( COUNTRY );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }
    /*���Լ���Wi-Fi���� */
    WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
    while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
    }
    WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

    /* ����IP����*/
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
            /* �ȴ� */
            sys_msleep( 10 );
        }
    }
		
    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));
		
//		    /*�򿪱����׽��ֽ���ping */
//    if ( ( socket_hnd = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP ) ) < 0 )
//    {
//        WPRINT_APP_ERROR(( "unable to create socket for Ping\n" ));
//        return;
//    }
//    /* �ڱ����׽��������ý��ճ�ʱ���Ա�ping�ᳬʱ. */
//    lwip_setsockopt( socket_hnd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof( recv_timeout ) );
//#ifdef PING_TARGET
//        target.addr = htonl( PING_TARGET );
//#else /* ifdef PING_TARGET */
//        target.addr = netif_ip4_gw(&wiced_if)->addr;
//#endif /* ifdef PING_TARGET */

//   WPRINT_APP_INFO(("Pinging: %s\n", ip4addr_ntoa( &target )));
//		netio_init();
		

}
/**
 * Main Ping app
 *
 * ��ʼ��Wiced�������������磬Ȼ��һֱpingָ����IP��ַ��
 */
void app_main( void )
{
#define PING_SW 0
		/*����wifi lwip��Ϣ*/
		Config_WIFI_LwIP_Info();
		client_init();
	
    while ( 1 )
    {
        err_t result;

#if PING_SW
        /* ����ping*/
        if ( ping_send( socket_hnd, &target ) != ERR_OK )
        {
            WPRINT_APP_ERROR(( "Unable to send Ping\n" ));
            return;
        }

        /* ��¼ʱ��ping�ѷ��� */
        send_time = host_rtos_get_time( );

        /* �ȴ�ping�ظ� */
        result = ping_recv( socket_hnd );
        if ( ERR_OK == result )
        {
            WPRINT_APP_INFO(("Pinginging Reply %dms\n", (int)( host_rtos_get_time( ) - send_time ) ));
        }
        else
        {
            WPRINT_APP_INFO(("Ping timeout\n"));
        }
#endif 
        /*�ȴ�ֱ����һ��ping��ʱ�� */
        sys_msleep( PING_DELAY );
    }

    /* �ػ�����-��������ѭ����δʹ�� */
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
