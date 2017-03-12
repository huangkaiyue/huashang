#include "comshead.h"
#include "host/ap_sta.h"
#include "host/mtkwifi.h"
#include "nvram.h"
#include "base/cJSON.h"

//#define MAIN_TEST

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

#define NEW_GET_WIFI
#ifdef NEW_GET_WIFI
#define SMART_CONFIG_HEAD	"apcli0    elian:"
#endif

#define LOCK_SMART_CONFIG_WIFI		1	//��������������
#define UNLOCK_SMART_CONFIG_WIFI	0	//���������н���
static unsigned char connetState=UNLOCK_SMART_CONFIG_WIFI;
int checkconnetState(void){
	if(connetState==0)
		return 0;
	else
		return -1;
}

static int sendSsidPasswd(const char *ssid,const char *passwd){
	int ret=0;
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",SMART_CONFIG_OK);
	cJSON_AddStringToObject(pItem, "ssid",ssid);
	cJSON_AddStringToObject(pItem, "passwd",passwd);
	szJSON = cJSON_Print(pItem);
	ret= SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
	return ret ;
}
/*******************************************************
��������: ��������ļ�
����: ��
����ֵ: ��
********************************************************/
int checkInternetFile(void){
	if(access("/var/internet.lock",0) < 0){
		return -1;
	}
	return 0;
}
#ifdef NEW_GET_WIFI
static int NewGetSmartConfigWifi(char *smartData,char *ssid,char *passwd){
	int smartconfig_headlen=strlen(SMART_CONFIG_HEAD);	
	char wifidata[200]={0};
	snprintf(wifidata,200,"%s",smartData+smartconfig_headlen);
	FILE *fplog = fopen("/home/airkiss_wifi.txt","w+");
	if(fplog){
		fprintf(fplog,"wifidata:\n%s\n",wifidata);
		fclose(fplog);
	}
	cJSON *pJson = cJSON_Parse((const char *)wifidata);
	if(NULL == pJson){
		return -1;
	}
	cJSON * pSub = cJSON_GetObjectItem(pJson, "ssid");
	if(pSub!=NULL){
		if(strcmp(pSub->valuestring,"")){
			sprintf(ssid,"%s",pSub->valuestring);
			pSub = cJSON_GetObjectItem(pJson, "pwd");
			sprintf(passwd,"%s",pSub->valuestring);
			FILE *fplog = fopen("/home/airkiss_wifi.txt","a+");
			if(fplog){
				fprintf(fplog,"ssid:%s\n",ssid);
				fprintf(fplog,"passwd:%s\n",passwd);
				fclose(fplog);
			}
			return 0;
		}
	}	
	return -1;
}
#else
static int smartGetSsid(char *smartData,char *ssid,char *passwd){
	char getssid[64]={0},getpwd[64]={0};
	char *p = strstr(smartData,"ssid");
	if(p==NULL)
		return -1;
	//printf("p= %s\n",p);
	sscanf(p,"%s %s",getssid,getpwd);
	sprintf(ssid,"%s",getssid+5);
	sprintf(passwd,"%s",getpwd+4);	
	int len = strlen(ssid);
	if(len==1){
		DEBUG_AP_STA("not find ssid\n");
		return -1;
	}
	ssid[len-1]='\0';
	len= strlen(passwd);
	passwd[len-1]='\0';
	
	FILE *fplog=NULL;
	fplog = fopen("/home/airkiss_wifi.txt","a+");
	if(fplog){
		fprintf(fplog,"ssid:===(%s)===pwd:===(%s)===\n",ssid,passwd);
		fclose(fplog);
	}
	return 0;
}
#endif
static void createSmartConfigLock(void){
	fopen("/var/SmartConfig.lock","w+");
}

static void delSmartConfigLock(void){
	remove("/var/SmartConfig.lock");
}
static int createInternetLock(void){
	fopen("/var/internet.lock","w+");
}
static void delInternetLock(void){
	remove("/var/internet.lock");
}
static int SmartConfig(void *arg){
  	FILE *fp=NULL;
   	char *buf, *ptr;
	char ssid[64]={0},pwd[64]={0};
	int ret=-1,timeout=0;
	ConnetWIFI *wifi = (ConnetWIFI *)arg;
	buf = (char *)calloc(512,1);
	if(buf==NULL){
		perror("calloc failed ");
		goto exit0;
	}	
	createSmartConfigLock();
	system("iwpriv apcli0 elian start");
	while (++timeout<400){	
		//����ʵʱ�򿪹ܵ������ܶ�ȡ�����µ�����
		if((fp = popen("iwpriv apcli0 elian result", "r"))==NULL){
			fprintf(stderr, "%s: iwpriv apcli0 elian result failed !\n", __func__);
			break;
		}
		memset(buf,0,512);
		fgets(buf, 512, fp);
		ptr =buf; 
#ifdef NEW_GET_WIFI
		if(!NewGetSmartConfigWifi(ptr,ssid,pwd)){
			DEBUG_AP_STA("smartGetSsid : ssid:%s   pwd:%s\n",ssid,pwd);
			ret=0;
			break;
		}
#else
		if(!smartGetSsid(ptr,ssid,pwd)){
			DEBUG_AP_STA("smartGetSsid : ssid:%s   pwd:%s\n",ssid,pwd);
			ret=0;
			break;
		}
#endif		
		usleep(100000);
		
		memset(ssid,0,64);
		memset(pwd,0,64);
		pclose(fp);
	}
	timeout=0;
	system("iwpriv apcli0 elian stop");
	system("iwpriv apcli0 elian clear");
	if(ret==0){
		snprintf(wifi->ssid,64,"%s",ssid);
		snprintf(wifi->passwd,64,"%s",pwd);
		wifi->connetEvent(SMART_CONFIG_OK);	//�Ѿ����յ�ssid �� passwd
		sendSsidPasswd(ssid,pwd);
		delSmartConfigLock();
		sleep(5);
		while(++timeout<40){	//�ȴ������ɹ���ʹ�ܰ���
			sleep(1);
			if(checkInternetFile()){
				sleep(5);
				break;
			}
		}
		if(timeout>=40){
			delInternetLock();		//��ֹ�������̳������⣬��Ҫ�ֶ�ɾ��  2017-03-05-22:57
		}
	}else{
		wifi->connetEvent(SMART_CONFIG_FAILED); //û���յ�app���͹�����ssid��passwd
		delSmartConfigLock();
		delInternetLock();	//�ϰ�ν��ļ���
	}
	free(buf);
	connetState=UNLOCK_SMART_CONFIG_WIFI;
exit0:
	wifi->enableGpio();
	free(wifi);
	wifi=NULL;
	return ret;
}
int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void)){
	DEBUG_AP_STA("startSmartConfig...\n");
	smartConifgLog("startSmartConfig start \n");
	int ret=-1;
	if(connetState==LOCK_SMART_CONFIG_WIFI){
		smartConifgLog("startSmartConfig failed \n");
		return -1;
	}
	ConnetEvent(START_SMARTCONFIG);	//֪ͨ�û�����wifi ���룬��������
	
	connetState = LOCK_SMART_CONFIG_WIFI;
	ConnetWIFI *wifi =(ConnetWIFI *)calloc(1,sizeof(ConnetWIFI));
	if(wifi==NULL){
		perror("startSmartConfig :calloc failed ");
		ret=-1;
		goto exit1;
	}
	wifi->connetEvent = ConnetEvent;
	wifi->enableGpio = EnableGpio;
	smartConifgLog("startSmartConfig pool_add_task ok\n");
	createInternetLock();	//�ϰ�����ļ���
	pool_add_task(SmartConfig, wifi);
	ret=0;
	return ret;
exit1:
	connetState=UNLOCK_SMART_CONFIG_WIFI;
	delInternetLock();	//�ϰ�ν��ļ���
	return ret;
}

void startServiceWifi(void){
	int ret=0;
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",START_SERVICES);
	szJSON = cJSON_Print(pItem);
	ret= SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
	return ret ;	
}

#ifdef MAIN_TEST
#define USAGE "Usage:  ./ap_sta 1 \n"

static void test_ConnetEvent(int event){
	DEBUG_AP_STA("event =%d\n",event);
}
int main(int argc,char **argv){
	if(argc<2)
	{
		DEBUG_AP_STA(USAGE);
		return -1;
	}
	int mode = atoi(argv[1]);
	ConnetWIFI *wifi = (ConnetWIFI *)malloc(sizeof(ConnetWIFI));
	if(wifi==NULL){
		perror("calloc error !!!");
		return;
	}
	wifi->connetEvent = test_ConnetEvent;
	switch(mode)
	{
		case 1:
			snprintf(wifi->ssid,64,"%s","TP-LINK_2294F4");
			snprintf(wifi->passwd,64,"%s","lemeitong168");	
			break;
		case 2:
			snprintf(wifi->ssid,64,"%s","5870");
			snprintf(wifi->passwd,64,"%s","12345678");	
			break;
		case 3:
			snprintf(wifi->ssid,64,"%s","5870_1");
			snprintf(wifi->passwd,64,"%s","12345678");	
			break;
		case 4:
			snprintf(wifi->ssid,64,"%s","58700");
			snprintf(wifi->passwd,64,"%s","12345678");	
			break;
		case 5:
			snprintf(wifi->ssid,64,"%s","5870");
			snprintf(wifi->passwd,64,"%s","");	
			break;
		case 10:
			DEBUG_AP_STA("start smartconfig ...... \n");
			SmartConfig((void *)wifi);
			return 0;
			break;
		default:
			break;
	}
	DEBUG_AP_STA("connet wifi =%s ssid=%s\n",wifi->ssid,wifi->passwd);
	hostHonnectNetwork(wifi);
	return 0;
}
#endif	//end  MAIN_TEST

