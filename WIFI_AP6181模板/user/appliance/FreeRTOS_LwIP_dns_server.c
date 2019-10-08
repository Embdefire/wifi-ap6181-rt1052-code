/**
 * @file
 * 一个DNS服务器，它使用服务器的本地地址响应所有DNS查询。
 */


#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include <string.h>
#include <stdint.h>
#include "appliance.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"

/******************************************************
 *                      宏定义
 ******************************************************/

/******************************************************
 *                    常亮
 ******************************************************/
#define DNS_STACK_SIZE                     (600)

#define DNS_FLAG_QR_POS                     (15)
#define DNS_FLAG_QR_SIZE                  (0x01)
#define DNS_FLAG_QR_QUERY                    (0)
#define DNS_FLAG_QR_RESPONSE                 (1)


/* DNS opcodes */
#define DNS_FLAG_OPCODE_POS                 (11)
#define DNS_FLAG_OPCODE_OPCODE_SIZE       (0x0f)
#define DNS_FLAG_OPCODE_SQUERY               (0) /* RFC 1035 */
#define DNS_FLAG_OPCODE_IQUERY               (1) /* RFC 1035, 3425 */
#define DNS_FLAG_OPCODE_STATUS               (2) /* RFC 1035 */
#define DNS_FLAG_OPCODE_NOTIFY               (4) /* RFC 1996 */
#define DNS_FLAG_OPCODE_UPDATE               (5) /* RFC 2136 */

#define DNS_FLAG_AUTHORITIVE_POS            (10)
#define DNS_FLAG_AUTHORITIVE_YES             (1)
#define DNS_FLAG_AUTHORITIVE_NO              (0)

#define DNS_FLAG_TRUNCATED_POS               (9)
#define DNS_FLAG_TRUNCATED_YES               (1)
#define DNS_FLAG_TRUNCATED_NO                (0)

#define DNS_FLAG_DESIRE_RECURS_POS           (8)
#define DNS_FLAG_DESIRE_RECURS_YES           (1)
#define DNS_FLAG_DESIRE_RECURS_NO            (0)

#define DNS_FLAG_RECURS_AVAIL_POS            (7)
#define DNS_FLAG_RECURS_AVAIL_YES            (1)
#define DNS_FLAG_RECURS_AVAIL_NO             (0)

#define DNS_FLAG_AUTHENTICATED_POS           (5)
#define DNS_FLAG_AUTHENTICATED_YES           (1)
#define DNS_FLAG_AUTHENTICATED_NO            (0)

#define DNS_FLAG_ACCEPT_NON_AUTHENT_POS      (4)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_YES      (1)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_NO       (0)

#define DNS_FLAG_REPLY_CODE_POS              (0)
#define DNS_FLAG_REPLY_CODE_NO_ERROR         (0)


#define DNS_UDP_PORT                        (53)


/*DNS套接字超时值（以毫秒为单位）。 修改此设置以使线程退出更具响应性*/
#define DNS_SOCKET_TIMEOUT                 (500)

#define DNS_MAX_DOMAIN_LEN                 (255)
#define DNS_QUESTION_TYPE_CLASS_SIZE         (4)
#define DNS_IPV4_ADDRESS_SIZE                (4)

#define DNS_MAX_PACKET_SIZE   ( sizeof(dns_header_t) + DNS_MAX_DOMAIN_LEN + DNS_QUESTION_TYPE_CLASS_SIZE + sizeof(response_answer_array) + DNS_IPV4_ADDRESS_SIZE )

/******************************************************
 *                   枚举
 ******************************************************/

/******************************************************
 *                类型定义
 ******************************************************/

/******************************************************
 *                    结构体
 ******************************************************/
/* DNS 数据结构 */
typedef struct
{
    uint16_t  transaction_id;
    uint16_t  flags;
    uint16_t  num_questions;
    uint16_t  num_answer_records;
    uint16_t  num_authority_records;
    uint16_t  num_additional_records;

} dns_header_t;



/******************************************************
 *               静态函数声明
 ******************************************************/
static void dns_thread( void * thread_input );

/******************************************************
 *              变量定义
 ******************************************************/
static const char response_answer_array[] =
{
        0xC0,                   /*指示下一个值是名称的偏移量，而不是紧随其后的段的长度 */
        0x0C,                   /* 第一个问题名称的偏移量 */
        0x00, 0x01,             /* A型-主机地址 */
        0x00, 0x01,             /* 网络课 */
        0x00, 0x00, 0x00, 0x3C, /* 生存时间-60秒-避免弄乱适当的Internet DNS缓存*/
        0x00, 0x04,             /* 数据长度-IPv4地址的4个字节*/
};

static volatile char    dns_quit_flag = 0;
static int              dns_socket_handle            = -1;
static xTaskHandle      dns_thread_handle;
static char             dns_packet_buff[ DNS_MAX_PACKET_SIZE ];


/******************************************************
 *               函数声明
 ******************************************************/

void start_dns_server( uint32_t local_addr )
{
    xTaskCreate( dns_thread, "DNS thread", DNS_STACK_SIZE/sizeof( portSTACK_TYPE ), (void*)local_addr, DEFAULT_THREAD_PRIO, &dns_thread_handle);
}

void quit_dns_server( void )
{
    dns_quit_flag = 1;
}

/**
 *  实现一个非常简单的DNS服务器。
 *
 * @param my_addr :绑定服务器端口的本地IP地址。
 */

static void dns_thread( void * thread_input )
{
    struct sockaddr_in    my_addr_store;
    struct sockaddr_in*   my_addr = &my_addr_store;
    struct sockaddr_in    source_addr;
    struct sockaddr_in    destination_addr;
    char * loc_ptr;
    int                   status;
    socklen_t             source_addr_len;
    static dns_header_t* dns_header_ptr = (dns_header_t*) &dns_packet_buff;
    int                   recv_timeout = DNS_SOCKET_TIMEOUT;

    my_addr->sin_family = AF_INET;
    my_addr->sin_addr.s_addr = (u32_t) thread_input;

    /* 将端口号添加到IP地址 */
    my_addr->sin_port = htons( DNS_UDP_PORT );

    /* 创建DNS套接字 */
    dns_socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt( dns_socket_handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout, sizeof( recv_timeout ) );

    /* 将套接字绑定到本地IP地址 */
    status = bind(dns_socket_handle, (struct sockaddr*)my_addr, sizeof(struct sockaddr_in));

    /* 设置目标IP地址和端口号 */
    destination_addr.sin_port   = htons( DNS_UDP_PORT );
    destination_addr.sin_family = AF_INET;
    memset( &destination_addr.sin_addr.s_addr, 0xff, 4 ); /* 广播 */

    /*无限循环*/
    while ( dns_quit_flag == 0 )
    {
        /* 休眠直到从套接字接收到数据。 */
        status = recvfrom( dns_socket_handle, dns_packet_buff, sizeof( dns_packet_buff ), 0 , (struct sockaddr *) &source_addr, &source_addr_len);
        if ( status <= 0 )
        {
            continue;
        }

        /* 修复标题的字节序*/
        dns_header_ptr->flags                  = ntohs( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = ntohs( dns_header_ptr->num_questions );

        /*只回应查询*/
        if ( DNS_FLAG_QR_QUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_QR_POS ) & DNS_FLAG_QR_SIZE ) )
        {
            continue;
        }

        if ( DNS_FLAG_OPCODE_SQUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_OPCODE_POS ) & DNS_FLAG_OPCODE_OPCODE_SIZE ) )
        {
            continue;
        }

        if ( dns_header_ptr->num_questions < 1 )
        {
            continue;
        }


        dns_header_ptr->flags = ( DNS_FLAG_QR_RESPONSE           << DNS_FLAG_QR_POS                 ) |
                                ( DNS_FLAG_OPCODE_SQUERY         << DNS_FLAG_OPCODE_POS             ) |
                                ( DNS_FLAG_AUTHORITIVE_YES       << DNS_FLAG_AUTHORITIVE_POS        ) |
                                ( DNS_FLAG_TRUNCATED_NO          << DNS_FLAG_TRUNCATED_POS          ) |
                                ( DNS_FLAG_DESIRE_RECURS_NO      << DNS_FLAG_DESIRE_RECURS_POS      ) |
                                ( DNS_FLAG_RECURS_AVAIL_NO       << DNS_FLAG_RECURS_AVAIL_POS       ) |
                                ( DNS_FLAG_AUTHENTICATED_YES     << DNS_FLAG_AUTHENTICATED_POS      ) |
                                ( DNS_FLAG_ACCEPT_NON_AUTHENT_NO << DNS_FLAG_ACCEPT_NON_AUTHENT_POS ) |
                                ( DNS_FLAG_REPLY_CODE_NO_ERROR   << DNS_FLAG_REPLY_CODE_POS         );


        dns_header_ptr->num_questions          = 1;
        dns_header_ptr->num_answer_records     = 1;
        dns_header_ptr->num_authority_records  = 0;
        dns_header_ptr->num_additional_records = 0;


        /* 查找问题的结尾 */
        loc_ptr = &dns_packet_buff[ sizeof(dns_header_t) ];

        /*跳过域名-由长度为一字节的部分组成 */
        while ( ( *loc_ptr != 0 ) &&
                ( *loc_ptr <= 64 ) )
        {
            loc_ptr += *loc_ptr + 1;
        }
        loc_ptr++; /*跳过终止null*/

        /* 检查域名对于数据包而言是否太大-可能是由于攻击*/
        if ( loc_ptr >  &dns_packet_buff[ sizeof(dns_header_t) + DNS_MAX_DOMAIN_LEN ] )
        {
            continue;
        }

        loc_ptr += DNS_QUESTION_TYPE_CLASS_SIZE;

        /* 将响应数据复制到 */
        memcpy( loc_ptr, response_answer_array, sizeof(response_answer_array) );
        loc_ptr += sizeof(response_answer_array);

        /*	添加我们的IP地址*/
        memcpy( loc_ptr, &my_addr->sin_addr.s_addr, DNS_IPV4_ADDRESS_SIZE );
        loc_ptr += DNS_IPV4_ADDRESS_SIZE;

        dns_header_ptr->flags                  = htons( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = htons( dns_header_ptr->num_questions );
        dns_header_ptr->num_answer_records     = htons( dns_header_ptr->num_answer_records );
        dns_header_ptr->num_authority_records  = htons( dns_header_ptr->num_authority_records );
        dns_header_ptr->num_additional_records = htons( dns_header_ptr->num_additional_records );


        /* 发送数据包 */
        sendto( dns_socket_handle, (char *)dns_header_ptr, (int)(loc_ptr - (char*)&dns_packet_buff), 0 , (struct sockaddr *) &source_addr, source_addr_len);

    }

    /* 删除 DNS 连接*/
    lwip_close( dns_socket_handle );

    /* 清除这个启动线程*/
    vTaskDelete( dns_thread_handle );
}


