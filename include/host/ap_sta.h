#ifndef _AP_STA_H
#define _AP_STA_H

#define CONNET_ING			9	//正在连接
#define CONNECT_OK			10	//连接成功
#define START_SMARTCONFIG	11	//启动配网
#define SMART_CONFIG_OK		12	//接受密码成功
#define NOT_FIND_WIFI		13	//没有扫描到wifi
#define SMART_CONFIG_FAILED	14	//没有收到用户发送的wifi
//#define CONNET_WAIT		16

#define START_SERVICES		17	//启动联网服务
#define NOT_NETWORK			18	//板子没有连接上网络
#define CONNET_CHECK		19	//正在检查网络是否可用


//#define DBG_AP_STA
#ifdef DBG_AP_STA 
#define DEBUG_AP_STA(fmt, args...) printf("%s",__func__,fmt, ## args)
#else   
#define DEBUG_AP_STA(fmt, args...) { }
#endif	//end DBG_AP_STA

//#define TEST_WIFI
#ifdef TEST_WIFI
#define TEST_WIFI_DATA "apcli0    elian:AM=0, ssid=TP-LINK_2294F4, pwd=369852369, user=, cust_data_len=0, cust_data=,"
#define TEST_WIFI_1 		"apcli0    elian:AM=0, ssid=TURING ROBOT 2, pwd=turingrobot88888, user=, cust_data_len=0, cust_data=,"
#endif	//end TEST_WIFI

#define SMART_CONFIG_HEAD	"apcli0    elian:"

#define LOCK_SMART_CONFIG_WIFI		1	//对配网进行上锁
#define UNLOCK_SMART_CONFIG_WIFI	0	//对配网进行解锁

extern int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void));//一键配网
extern int checkInternetFile(void);

#endif
