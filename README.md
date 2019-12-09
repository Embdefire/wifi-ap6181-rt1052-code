# 野火WiFi模块AP6181-RT1052-PRO配套代码

### **[wifi-ap6181-rt1052-code](https://github.com/Embdefire/wifi-ap6181-rt1052-code)**包含了配套野火RT1052PRO开发板的wifi-ap6181示例程序。

程序配套代码目录如下：

| 序号 | 清单                                                         | 说明                                         |
| ---- | ------------------------------------------------------------ | -------------------------------------------- |
| 0    | [WiFi_SDK](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/WiFi_SDK) | 官方SDK                                      |
| 1    | [RT1052+WiFi+RTOS+LwIP_NETIO_SPEED](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/RT1052%2BWiFi%2BRTOS%2BLwIP_NETIO_SPEED) | 裸机版本                                     |
| 2    | [wifi_lwip_AP](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_AP) | 热点（AP）当有热点链接时会自动分配IP，并打印 |
| 3    | [wifi_lwip_client](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_client) | netcoon 客户端                               |
| 4    | [wifi_lwip_client_socket](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_client_socket) | socket 客户端                                |
| 5    | [wifi_lwip_dhcp](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_dhcp) | DHCP                                         |
| 6    | [wifi_lwip_dns](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_dns) | DNS                                          |
| 7    | [wifi_lwip_http](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_http) | 网页显示图片                                 |
| 8    | [wifi_lwip_http_led](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_http_led) | 网页控制开发板led                            |
| 9    | [wifi_lwip_iperf_all](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_iperf_all) | wifi测速                                     |
| 10   | [wifi_lwip_mqtt_ali_dht11](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_mqtt_ali_dht11) | 接入阿里云                                   |
| 11   | [wifi_lwip_mqtt_baidu_dht11_drive](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_mqtt_baidu_dht11_drive) | 接入百度云                                   |
| 12   | [wifi_lwip_onenet_mqtt_dht11](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_onenet_mqtt_dht11) | 接入OneNET                                   |
| 13   | [wifi_lwip_ping](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_ping) | ping                                         |
| 14   | [wifi_lwip_scan](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_scan) | 扫描周围wifi信息，并打印                     |
| 16   | [wifi_lwip_tcpecho](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_tcpecho) | socket tcp                                   |
| 17   | [wifi_lwip_tcpecho_socket](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_tcpecho_socket) | 开发板作为热点（服务器）                     |
| 18   | [wifi_lwip_udpecho](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_udpecho) | netconn udp                                  |
| 19   | [wifi_lwip_udpecho_socket](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_udpecho_socket) | socket udp                                   |
| 20   | [wifi_lwip_webclient](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/wifi_lwip_webclient) | 获取野火官网网页并打印                       |
| 21   | [网络调试助手-手机版（安卓）](https://github.com/Embdefire/wifi-ap6181-rt1052-code/tree/master/网络调试助手-手机版（安卓）) | 网络调试助手（手机版）                       |

### 使用野火开发板需要配置环境

（1）需要将开发板的天线连接好；

（2）将开发板的跳帽J78、J66安装好；

（3）需要将自己的电脑与开发板处于同一局域网下（部分例程需要连接外网）；

（4）以上三条是所有例程必须要配置的硬件环境，每个例程都有相应的必读说明，必读说明更加详细；

（5）例程是基于lwip 、 freertos 的，相关问题可以参考野火的书籍；