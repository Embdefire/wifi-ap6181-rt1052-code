#include "wifi_base_config.h"
#include "appliance.h"

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "debug.h"
#include "fsl_iomuxc.h"
/* FreeRTOSͷ�ļ� */
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
#include "./delay/core_delay.h"   

static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */
static void AppTaskCreate(void);/* ���ڴ������� */
static void Receive_Task(void* parameter);
extern void TCPIP_Init(void);

#define RECV_DATA         (1024)
#define LOCAL_PORT                 5001
extern SemaphoreHandle_t BinarySem_Handle;
char *recv_data;


volatile uint32_t JPEG_len=0; //��Ƭ����

int connected;

extern uint32_t fullCameraBufferAddr;    //�ɼ���ɵĻ�������ַ 
extern uint8_t jpeg_data_ok;
extern void send_jpeg_dev(uint8_t *img_buf,uint32_t img_buf_len);
void img_sed_uart()
{
	uint32_t i,jpgstart,jpglen; 
	uint8_t *p;
	uint8_t headok=0;	
	if(jpeg_data_ok==0)
			{
							/* ��ʼ�ɼ�ͼ��*/
					CAMERA_RECEIVER_Start(&cameraReceiver);

					CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, fullCameraBufferAddr);			
					/* �ȴ���ȡ����֡����������ʾ */
//					while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr))
//					{
//					}
					CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr);
					CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &fullCameraBufferAddr);
					if(fullCameraBufferAddr == (uint32_t)image_csi1[0])
					{
						PRINTF("֡ ok \r\n ");
					}
					//CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
			}
			
			if(jpeg_data_ok==1)
			{
					PRINTF("ֹͣ�ɼ� ��ȡͼƬ�� \r\n ");
					p=(uint8_t*)user_image[0];
					jpglen=0;	//����jpg�ļ���СΪ0
					headok=0;	//���jpgͷ���
					
					while(1)//����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С
					{
						if((p[i]==0XFF)&&(p[i+1]==0XD8))//�ҵ�FF D8
						{
							jpgstart=i;
							headok=1;	//����ҵ�jpgͷ(FF D8)
							PRINTF("find jpeg head \r\n ");
						}
						if((p[i]==0XFF)&&(p[i+1]==0XD9)&&headok)//�ҵ�ͷ�Ժ�,����FF D9
						{
							jpglen=i-jpgstart+2;
							
							JPEG_len=jpglen;
							p+=jpgstart;			//ƫ�Ƶ�0XFF,0XD8��
							PRINTF("find jpeg end \r\n ");
							
							PRINTF("start send img buf \r\n ");
							//send_jpeg_dev(p,JPEG_len);
							
							write(connected,p,JPEG_len);

							
							PRINTF("img buf send over \r\n ");
							//return p;
							break;
						}
						i++;
						if(i>1024*10)//����һ�� ���ⷢ��Ӳ������
						{
							i=0;
						}
					}	
			}

}

/**
 * @brief app_main
 *
 */
void app_main( void )
{
		/*����wifi lwip��Ϣ*/
			
//		/* ��ʼ������ͷ��PDN��RST���� */
//    gpio_pin_config_t pinConfig = {
//        kGPIO_DigitalOutput, 1,
//    };
//    GPIO_PinInit(CAMERA_PWR_GPIO, CAMERA_PWR_GPIO_PIN, &pinConfig);
//		GPIO_PinInit(CAMERA_RST_GPIO, CAMERA_RST_GPIO_PIN, &pinConfig);
//		GPIO_PinWrite(CAMERA_PWR_GPIO, CAMERA_PWR_GPIO_PIN, 0);//��������
//		
//	
		Config_WIFI_LwIP_Info();

		int sock = -1;
		

		struct sockaddr_in server_addr,client_addr;
		socklen_t sin_size;
		int recv_data_len;
		
		PRINTF("���ض˿ں���%d\n\n",LOCAL_PORT);
		
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
			

//		
		while(1)
		{

			 
			sin_size = sizeof(struct sockaddr_in);

			connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

			PRINTF("new client connected from (%s, %d)\n",
							inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			{
				int flag = 1;
				
				setsockopt(connected,
									 IPPROTO_TCP,     /* set option at TCP level */
									 TCP_NODELAY,     /* name of option */
									 (void *) &flag,  /* the cast is historical cruft */
									 sizeof(int));    /* length of option value */
			}

			Camera_Init();////����ͷ��ʼ��
			while(1)
			{
				recv_data_len = recv(connected, recv_data, RECV_DATA, 0);

				if (recv_data_len <= 0) 
					break;
			
				write(connected,recv_data,recv_data_len);
				
				
				
//				Camera_Init();////����ͷ��ʼ��				
//				
//				CPU_TS_Tmr_Delay_MS(1000);
//				CPU_TS_Tmr_Delay_MS(1000);
//				PRINTF("��ʱ����\r\n");
//				
//				while(1)
//				{	
//				//	PRINTF("tefgyuasdgfuugasyuffgassk\r\n");
//					img_sed_uart();
//						//CAMERA_RECEIVER_Stop(&cameraReceiver);//ֹͣ�ɼ�
//						//
//        }
				
				
				
				
			}
			if (connected >= 0) 
				closesocket(connected);
			
			connected = -1;
		}
	__exit:
		if (sock >= 0) closesocket(sock);
		if (recv_data) free(recv_data);

		
//	  
//		while(1)
//		{	
//			img_sed_uart(); 
//		}
		
}




