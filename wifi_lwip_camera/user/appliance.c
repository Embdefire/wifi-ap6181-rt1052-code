#include "wifi_base_config.h"
#include "appliance.h"

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "debug.h"
#include "fsl_iomuxc.h"
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "./led/bsp_led.h"
#include "tcpecho.h"

#include "lwip/opt.h"
#include <lwip/sockets.h>

#include "lwip/sys.h"
#include "lwip/api.h"
#include "fsl_debug_console.h"
#include "appliance.h"

#include "img_data_deal.h"

static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static void AppTaskCreate(void);/* 用于创建任务 */
static void Receive_Task(void* parameter);
extern void TCPIP_Init(void);

#define RECV_DATA         (1024)
#define LOCAL_PORT                 5001
extern SemaphoreHandle_t BinarySem_Handle;
char *recv_data;


/**
 * @brief app_main
 *
 */
void app_main( void )
{
		/*配置wifi lwip信息*/
			
		Config_WIFI_LwIP_Info();
		Camera_Init();//摄像头初始化
		int sock = -1,connected;

		struct sockaddr_in server_addr,client_addr;
		socklen_t sin_size;
		int recv_data_len;
		
		PRINTF("本地端口号是%d\n\n",LOCAL_PORT);
		
		recv_data = (char *)pvPortMalloc(RECV_DATA);
		
		sock = socket(AF_INET, SOCK_STREAM, 0);

		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(LOCAL_PORT);
		memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
		
		if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
		{}
		if (listen(sock, 5) == -1)
		{}
		while(1)
		{

			 
//			sin_size = sizeof(struct sockaddr_in);

//			connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

//			PRINTF("new client connected from (%s, %d)\n",
//							inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//			{
//				int flag = 1;
//				
//				setsockopt(connected,
//									 IPPROTO_TCP,     /* set option at TCP level */
//									 TCP_NODELAY,     /* name of option */
//									 (void *) &flag,  /* the cast is historical cruft */
//									 sizeof(int));    /* length of option value */
//			}
			img_sed_uart();
//			while(1)
//			{
//				recv_data_len = recv(connected, recv_data, RECV_DATA, 0);
//				xSemaphoreGive( BinarySem_Handle );//给出二值信号量
//				if (recv_data_len <= 0) 
//					break;
//				

//				write(connected,recv_data,recv_data_len);

//				
//			}
//			if (connected >= 0) 
//				closesocket(connected);
//			
//			connected = -1;
		}
//	__exit:
//		if (sock >= 0) closesocket(sock);
//		if (recv_data) free(recv_data);
//	
//		
//		
//		
//		
		
		
		
		
		
		
		
		
		
		
		
//	  
//		while(1)
//		{	
//			img_sed_uart(); 
//		}
		
}




