#include "comshead.h"
#include "host/ap_sta.h"
#include "host/mtkwifi.h"
#include "nvram.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "host/voices/callvoices.h"
#include "config.h"

//#define MAIN_TEST

static unsigned char connetState=UNLOCK_SMART_CONFIG_WIFI;
int checkconnetState(void){
	if(connetState==UNLOCK_SMART_CONFIG_WIFI)
		return 0;
	else
		return -1;
}

static void createSmartConfigLock(void){
	FILE *fp = fopen(SMART_CONFIG_FILE_LOCK,"w+");
	if(fp){
		fclose(fp);
	}
}

static void delSmartConfigLock(void){
	remove(SMART_CONFIG_FILE_LOCK);
}
static void createInternetLock(void){
	FILE *fp =fopen(INTEN_NETWORK_FILE_LOCK,"w+");
	if(fp){
		fclose(fp);
	}
}
static void delInternetLock(void){
	remove(INTEN_NETWORK_FILE_LOCK);
}

int SendSsidPasswd_toNetServer(const char *ssid,const char *passwd,int random){
	int ret=0;
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",SMART_CONFIG_OK);
	cJSON_AddStringToObject(pItem, "ssid",ssid);
	cJSON_AddStringToObject(pItem, "passwd",passwd);
	cJSON_AddNumberToObject(pItem, "random",random);
	szJSON = cJSON_Print(pItem);
	ret= SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
	return ret ;
}
/*******************************************************
函数功能: 检查配网文件
参数: 无
返回值: 无
********************************************************/
int checkInternetFile(void){
	if(access(INTEN_NETWORK_FILE_LOCK,0) < 0){
		return -1;
	}
	return 0;
}
static int GetSsidAndPasswd(char *smartData,char *ssid,char *passwd,int *random){
	int smartconfig_headlen=strlen(SMART_CONFIG_HEAD);	
	char wifidata[200]={0};
	snprintf(wifidata,200,"%s",smartData+smartconfig_headlen);
	WiterSmartConifg_Log("wifidata:",wifidata);
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
			pSub = cJSON_GetObjectItem(pJson, "random");
			if(pSub){
				printf("recv random = %d\n",pSub->valueint);
				*random=pSub->valueint;
			}
			WiterSmartConifg_Log(ssid,passwd);
			return 0;
		}
	}	
	return -1;
}
//检查网络检查运行状态,异常状态启动，自动启动联网进程
static void *CheckNetWork_taskRunState(void *arg){
	int timeOut=0;
	while(1){
		if(++timeOut>30){
			break;
		}
		CheckNetManger_PidRunState();
		if(!checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
			printf("network is ok\n");
			break;
		}
		if(connetState==LOCK_SMART_CONFIG_WIFI){
			break;
		}
		sleep(2);
	}
	return NULL;
}
static void *RunSmartConfig_Task(void *arg){
  	FILE *fp=NULL;
   	char *buf, *ptr;
	char ssid[64]={0},pwd[64]={0};
	int ret=-1,timeout=0;
	int random=0;
	ConnetWIFI *wifi = (ConnetWIFI *)arg;
	buf = (char *)calloc(512,1);
	if(buf==NULL){
		perror("calloc failed ");
		goto exit0;
	}	
	createSmartConfigLock();
	system("iwpriv apcli0 elian start");
	while (++timeout<400){	
		//必须实时打开管道，才能读取到更新的数据
		if((fp = popen("iwpriv apcli0 elian result", "r"))==NULL){
			fprintf(stderr, "%s: iwpriv apcli0 elian result failed !\n", __func__);
			break;
		}
		memset(buf,0,512);
		fgets(buf, 512, fp);
		ptr =buf; 

		if(!GetSsidAndPasswd(ptr,ssid,pwd,&random)){
			DEBUG_AP_STA("ssid:%s   pwd:%s\n",ssid,pwd);
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
	if(ret==0){
		snprintf(wifi->ssid,64,"%s",ssid);
		snprintf(wifi->passwd,64,"%s",pwd);
		//wifi->connetEvent(SMART_CONFIG_OK);	//已经接收到ssid 和 passwd
		SendSsidPasswd_toNetServer(ssid,pwd,random);
		delSmartConfigLock();
		sleep(5);
		while(++timeout<40){	//等待配网成功后，使能按键
			sleep(1);
			if(checkInternetFile()){
				sleep(5);
				break;
			}
		}
		if(timeout>=40){
			delInternetLock();		//防止联网进程出现问题，需要手动删除  2017-03-05-22:57
		}
	}else{
		wifi->connetEvent(SMART_CONFIG_FAILED); //没有收到app发送过来的ssid和passwd
		delSmartConfigLock();
		delInternetLock();	//上半段解文件锁
	}
	free(buf);
	connetState=UNLOCK_SMART_CONFIG_WIFI;
exit0:
	wifi->enableGpio();
	free(wifi);
	wifi=NULL;
	pool_add_task(CheckNetWork_taskRunState, NULL);
	return NULL;
}
int startSmartConfig(void ConnetEvent(int event),void EnableGpio(void)){
	WiterSmartConifg_Log("startSmartConfig ","start");
	int ret=-1;
	if(connetState==LOCK_SMART_CONFIG_WIFI){
		WiterSmartConifg_Log("startSmartConfig  ","failed");
		return -1;
	}
	ConnetEvent(START_SMARTCONFIG);	//通知用户输入wifi 密码，进行配网
	
	connetState = LOCK_SMART_CONFIG_WIFI;
	ConnetWIFI *wifi =(ConnetWIFI *)calloc(1,sizeof(ConnetWIFI));
	if(wifi==NULL){
		perror("startSmartConfig :calloc failed ");
		ret=-1;
		goto exit1;
	}
	wifi->connetEvent = ConnetEvent;
	wifi->enableGpio = EnableGpio;
	WiterSmartConifg_Log("startSmartConfig ","pool_add_task ok");
	createInternetLock();	//上半段上文件锁
	pool_add_task(RunSmartConfig_Task, wifi);
	ret=0;
	return ret;
exit1:
	connetState=UNLOCK_SMART_CONFIG_WIFI;
	delInternetLock();	//上半段解文件锁
	return ret;
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

	return 0;
}
#endif	//end  MAIN_TEST

