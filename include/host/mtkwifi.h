#ifndef _MTKWIFI_H
#define _MTKWIFI_H

//ɨ�赱ǰ������wifi��Ϣ
typedef struct{
	char ssid[64];
	char bssid[20];
	char security[23];	//WPA1PSK/WPA2PSK/AES/NONE/TKIP/OPEN
						//��һ��·��ΪOPEN���������ģʽ��ȡ����ֵΪ:NONE
	char signal[9];	
	char mode[12];		//11b/g/n
	char ext_ch[7];
	char net_type[3];	//In
	char channel[4];	
}WifiResult;

#define CACHE_SIZE	64
typedef struct {
	char ssid[CACHE_SIZE];
	char passwd[CACHE_SIZE];
}WIFI;

typedef struct {
	char ssid[CACHE_SIZE];
	char passwd[CACHE_SIZE];
	void (*connetEvent)(int event);
	void (*enableGpio)(void);
}ConnetWIFI;

#ifdef DEBUG_WIFI
#define DBGWIFI(fmt,args...)	printf("" fmt, ## args)
#else
#define DBGWIFI(fmt,args...)	{}
#endif

extern void debugWifiResult(WifiResult *wifiresult);

/*
@ʹ���м�����
*/
extern void ApcliEnable(char *ssid,char *bssid,char *mode,char *enc,char *passwd,char *channel);
/*
@ mtkϵ��оƬɨ�赱ǰ������ssid
@����:	data����Ĳ��� *size 
		getWifiResult��ȡ����ǰ֮��ص�����,���ط�0 ��ֵ�˳�ɨ��
@����ֵ: 
*/
extern int __mtkScanWifi(void *data,int *size,int getWifiResult(void *data,int *size,WifiResult *wifiresult));
/*
@ mtkϵ��оƬɨ�赱ǰ������ssid
@����:	wifi��ȡ����ǰ���绷����ssid��Ϣ
		len �ܵĳ���
	���ݸ�ʽ:
	ssid&+security&+signal&-
@����ֵ: 
*/
extern void mtk76xxScanWifi(char *wifi,int *len);
/*
@�ָ���������
*/
extern void ResetDefaultRouter(void);
/*
@���ð��ӵķ�����wifi��������
*/
extern void setBasicWifi(char *ssid,char *passwd);
/*
@��ȡ���ӵ�wifi
*/
extern void ReadWifi(viod);
/*
@����ssid������
*/
extern void SaveCacheSsid(char *saveSsid,char *savePasswd);
/*
@��ȡ����wifi����
*/
extern int getCacheWifi(WIFI **wifi,int *cacheSize);
/*
@�ͷŻ�ȡ��������
*/
extern void freeCacheWifi(WIFI **wifi,int cacheSize);

#endif
