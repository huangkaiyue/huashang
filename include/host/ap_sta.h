#ifndef _AP_STA_H
#define _AP_STA_H

#define CONNET_ING         	9						//正在连接
#define CONNECT_OK			10						//连接成功
#define SMART_CONFIG_OK		12
#define NOT_FIND_WIFI		13						//没有扫描到wifi
#define SMART_CONFIG_FAILED	14						//没有收到用户发送的wifi


#define START_SERVICES		17	//启动联网服务
#define NOT_NETWORK			18	//板子没有连接上网络
#define CONNET_CHECK		19	//正在检查网络是否可用

#define INTERRUPT_NETWORK	100	//��ϵ�ǰ��ɨ�������

//#define DBG_AP_STA
#ifdef DBG_AP_STA 
#define DEBUG_AP_STA(fmt, args...) printf("%s",__func__,fmt, ## args)
#else   
#define DEBUG_AP_STA(fmt, args...) { }
#endif	//end DBG_AP_STA

#define SMART_CONFIG_HEAD	"apcli0    elian:"

#define LOCK_SMART_CONFIG_WIFI		1	//对配网进行上锁
#define UNLOCK_SMART_CONFIG_WIFI	0	//对配网进行解锁

extern int checkInternetFile(void);
extern int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void));//一键配网
extern void RecvNetWorkConnetState(int event);

#endif
