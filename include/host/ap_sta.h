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

extern int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void));//һ������
extern void startServiceWifi(void);
extern int checkInternetFile(void);

#endif
