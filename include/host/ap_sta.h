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

extern int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void));//一键配网
extern void startServiceWifi(void);
extern int checkInternetFile(void);

#endif
