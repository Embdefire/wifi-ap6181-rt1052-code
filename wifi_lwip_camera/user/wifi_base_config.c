#include "wifi_base_config.h"

extern void netio_init(void);
ip4_addr_t ipaddr, netmask, gw;

/*
����wifi lwip��Ϣ
*/
void Config_WIFI_LwIP_Info()
{
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
		
}
/**
 * Main Ping app
 *
 * ��ʼ��Wiced�������������磬Ȼ��һֱpingָ����IP��ַ��
 */
void app_main( void )
{
		/*����wifi lwip��Ϣ*/
		Config_WIFI_LwIP_Info();
    while (1)
    {
    }

}
