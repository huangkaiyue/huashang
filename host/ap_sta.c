#include "comshead.h"
#include "host/ap_sta.h"
#include "host/mtkwifi.h"
#include "nvram.h"
#include "base/cJSON.h"

//#define MAIN_TEST

//#define DBG_AP_STA
#ifdef DBG_AP_STA 
#define DEBUG_AP_STA(fmt, args...) printf("ap sta: " fmt, ## args)
#else   
#define DEBUG_AP_STA(fmt, args...) { }
#endif	//end DBG_AP_STA

//#define TEST_WIFI
#ifdef TEST_WIFI
#define TEST_WIFI_DATA "apcli0    elian:AM=0, ssid=TP-LINK_2294F4, pwd=369852369, user=, cust_data_len=0, cust_data=,"
#endif	//end TEST_WIFI

static unsigned char connetState=0;
int checkconnetState(void){
	if(connetState==0)
		return 0;
	else
		return -1;
}

void checkConnectFile(void)
{
	if(access("/var/connetc.lock",0) < 0){
		fopen("/var/connetc.lock","w+");
	}else{
		DEBUG_AP_STA("please delete /var/connetc.lock \n");
		exit(1);
	}
}

static int sendSsidPasswd(char *ssid,char *passwd)
{
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
	return ret ;
}
/*******************************************************
函数功能: 检查配网文件
参数: 无
返回值: 无
********************************************************/
int checkInternetFile(void)
{
	if(access("/var/internet.lock",0) < 0){
		return -1;
	}
	return 0;
}
static int smartGetSsid(char *smartData,char *ssid,char *passwd)
{
	char getssid[64]={0},getpwd[64]={0};
	char *p = strstr(smartData,"ssid");
	if(p==NULL)
		return -1;
	//printf("p= %s\n",p);
	sscanf(p,"%s %s",getssid,getpwd);
	sprintf(ssid,"%s",getssid+5);
	sprintf(passwd,"%s",getpwd+4);	
	int len = strlen(ssid);
	if(len==1)
	{
		DEBUG_AP_STA("not find ssid\n");
		return -1;
	}
	ssid[len-1]='\0';
	len= strlen(passwd);
	passwd[len-1]='\0';
	//printf("ssid:%s pwd: %s \n",ssid,passwd);
	return 0;
}
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
	if(buf==NULL)
	{
		perror("calloc failed ");
		return ret ;
	}	
	createSmartConfigLock();
	system("iwpriv apcli0 elian start");
	while (++timeout<280){	
		//必须实时打开管道，才能读取到更新的数据
		if((fp = popen("iwpriv apcli0 elian result", "r"))==NULL){
			fprintf(stderr, "%s: iwpriv apcli0 elian result failed !\n", __func__);
			break;
		}
		fgets(buf, 512, fp);
		ptr =buf; 
		if(!smartGetSsid(ptr,ssid,pwd)){
			DEBUG_AP_STA("smartGetSsid : ssid:%s   pwd:%s\n",ssid,pwd);
			ret=0;
			break;
		}
		usleep(100000);
		
		memset(ssid,0,64);
		memset(pwd,0,64);
		pclose(fp);
	}
	timeout=0;
	system("iwpriv apcli0 elian stop");
	system("iwpriv apcli0 elian clear");
	if(ret==0)
	{
		snprintf(wifi->ssid,64,"%s",ssid);
		snprintf(wifi->passwd,64,"%s",pwd);
		wifi->connetEvent(SMART_CONFIG_OK);	//已经接收到ssid 和 passwd
		sendSsidPasswd(ssid,pwd);
		delSmartConfigLock();
		sleep(5);
		while(++timeout<30){	//等待配网成功后，使能按键
			sleep(1);
			if(checkInternetFile()){
				sleep(5);
				break;
			}
		}
	}else{
		wifi->connetEvent(SMART_CONFIG_FAILED); //没有收到app发送过来的ssid和passwd
		delSmartConfigLock();
		delInternetLock();	//上半段解文件锁
	}
	wifi->enableGpio();
	free(wifi);
	wifi=NULL;
	free(buf);
	connetState=0;
	return ret;
}
int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void))
{
	DEBUG_AP_STA("startSmartConfig...\n");
	smartConifgLog("startSmartConfig start \n");
	if(connetState){
		smartConifgLog("startSmartConfig failed \n");
		return -1;
	}
	ConnetEvent(START_SMARTCONFIG);	//通知用户输入wifi 密码，进行配网
	int ret=0;
	connetState=1;
	createInternetLock();	//上半段上文件锁
	ConnetWIFI *wifi =(ConnetWIFI *)calloc(1,sizeof(ConnetWIFI));
	if(wifi==NULL)
	{
		perror("startSmartConfig :calloc failed ");
		ret=-2;
		EnableGpio();
		goto exit0;
	}
	wifi->connetEvent=ConnetEvent;
	wifi->enableGpio=EnableGpio;
	smartConifgLog("startSmartConfig pool_add_task ok\n");
	pool_add_task(SmartConfig, wifi);
exit0:
	connetState=0;
	delInternetLock();	//上半段解文件锁
	return ret;
}

void startServiceWifi(void)
{
	int ret=0;
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",START_SERVICES);
	szJSON = cJSON_Print(pItem);
	ret= SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	return ret ;	
}

#ifdef MAIN_TEST
#define USAGE "Usage:  ./ap_sta 1 \n"

static void test_ConnetEvent(int event)
{
	DEBUG_AP_STA("event =%d\n",event);
}
int main(int argc,char **argv)
{
	checkConnectFile();
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

