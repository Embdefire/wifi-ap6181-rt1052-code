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
 *  准备回显ICMP请求数据包的内容
 *
 *  @param iecho  : 指向icmp_echo_hdr结构的指针，将在其中构造ICMP数据包
 *  @param len    : 传递给iecho参数的数据包缓冲区的字节长度
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

    /* 用一些数据填充额外的数据缓冲区 */
    for ( i = 0; i < PING_DATA_SIZE; i++ )
    {
        ( (char*) iecho )[sizeof(struct icmp_echo_hdr) + i] = i;
    }

    iecho->chksum = inet_chksum( iecho, len );
}



/**
 *  发送一个Ping
 *
 * 使用指定的套接字将ICMP回显请求（Ping）发送到指定的IP地址。
 *
 *  @param socket_hnd :将通过其发送ping请求的本地套接字的句柄
 *  @param addr       :将向其发送ping请求的IP地址
 *
 *  @return 如果成功发送，则返回ERR_OK；如果内存不足，则返回ERR_MEM；否则，返回ERR_VAL
 */
err_t ping_send( int socket_hnd, ip4_addr_t *addr )
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    /* 为数据包分配内存*/
    if ( !( iecho = (struct icmp_echo_hdr*) mem_malloc( ping_size ) ) )
    {
        return ERR_MEM;
    }

    /* 构造ping请求 */
    ping_prepare_echo( iecho, ping_size );

    /*发送ping请求 */
    to.sin_len = sizeof( to );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->addr;

    err = lwip_sendto( socket_hnd, iecho, ping_size, 0, (struct sockaddr*) &to, sizeof( to ) );

    /* 释放 包 */
    mem_free( iecho );

    return ( err ? ERR_OK : ERR_VAL );
}


/**
 *  收到Ping回复
 *  等待使用指定的套接字接收ICMP回显答复（Ping答复）。 将序列号和ID号与上
 *  次发送的ping进行比较，如果匹配，则返回ERR_OK，表示有效的ping响应。
 *  @param socket_hnd : 本地套接字的句柄，通过该句柄将收到ping答复
 *
 *  @return  如果收到有效答复，则为ERR_OK，否则为ERR_TIMEOUT
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
                return ERR_OK; /*收到回声回复-成功返回 */
            }
        }
    }

    return ERR_TIMEOUT; /* 超时前未收到有效的回声回复 */
}

/*
配置wifi lwip信息
*/
void Config_WIFI_LwIP_Info()
{
    int recv_timeout = PING_RCV_TIMEOUT;
    wwd_result_t result;

    /* 初始化 Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wwd_buffer_init( NULL);
    result = wwd_management_wifi_on( COUNTRY );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }
    /*尝试加入Wi-Fi网络 */
    WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
    while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
    }
    WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

    /* 设置IP配置*/
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
            /* 等待 */
            sys_msleep( 10 );
        }
    }
		
    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));
		
//		    /*打开本地套接字进行ping */
//    if ( ( socket_hnd = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP ) ) < 0 )
//    {
//        WPRINT_APP_ERROR(( "unable to create socket for Ping\n" ));
//        return;
//    }
//    /* 在本地套接字上设置接收超时，以便ping会超时. */
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
 * 初始化Wiced，加入无线网络，然后一直ping指定的IP地址。
 */
void app_main( void )
{
#define PING_SW 0
		/*配置wifi lwip信息*/
		Config_WIFI_LwIP_Info();
		client_init();
	
    while ( 1 )
    {
        err_t result;

#if PING_SW
        /* 发送ping*/
        if ( ping_send( socket_hnd, &target ) != ERR_OK )
        {
            WPRINT_APP_ERROR(( "Unable to send Ping\n" ));
            return;
        }

        /* 记录时间ping已发送 */
        send_time = host_rtos_get_time( );

        /* 等待ping回复 */
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
        /*等待直到下一次ping的时间 */
        sys_msleep( PING_DELAY );
    }

    /* 关机代码-由于无限循环而未使用 */
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
