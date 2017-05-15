#ifndef _AP_STA_H
#define _AP_STA_H

#define CONNET_ING			9	//��������
#define CONNECT_OK			10	//���ӳɹ�
#define START_SMARTCONFIG	11	//��������
#define SMART_CONFIG_OK		12	//��������ɹ�
#define NOT_FIND_WIFI		13	//û��ɨ�赽wifi
#define SMART_CONFIG_FAILED	14	//û���յ��û����͵�wifi
//#define CONNET_WAIT		16

#define START_SERVICES		17	//������������
#define NOT_NETWORK			18	//����û������������
#define CONNET_CHECK		19	//���ڼ�������Ƿ����


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

#define LOCK_SMART_CONFIG_WIFI		1	//��������������
#define UNLOCK_SMART_CONFIG_WIFI	0	//���������н���

extern int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void));//һ������
extern int checkInternetFile(void);

#endif
