#include "comshead.h"
#include "base/pool.h"
#include "base/cJSON.h"
#include "network.h"
#include "mplay.h"
#include "StreamFile.h"
#include "host/voices/wm8960i2s.h"
#include "../host/studyvoices/qtts_qisc.h"
#include "../host/voices/gpio_7620.h"
#include "host/ap_sta.h"
#include "host/voices/callvoices.h"
#include "sysdata.h"
#include "uart/uart.h"
#include "config.h"

#define STREAM_EXIT				MAD_EXIT	//ֹͣ	
#define STREAM_PLAY 			MAD_PLAY	//����
#define STREAM_PAUSE			MAD_PAUSE	//��ͣ
#define STREAM_NEXT				MAD_NEXT

#define 	GET_NET_STATE	29	//��ȡ����״̬

//#define DBG_TCP
#ifdef DBG_TCP
#define DEBUG_TCP(fmt, args...) printf("network: " fmt, ## args)
#else   
#define DEBUG_TCP(fmt, args...) { }
#endif	//end DBG_AP_STA

#define VOL_DATA(x) (x-90)*4

unsigned char gpio_look=0;

static int SendTo(int sockfd,char *data,int size,struct sockaddr_in *peer){
	char *cachedata = (char *)calloc(1,size+16);
	if(cachedata==NULL){
		perror("calloc error !!!");
		return -1;
	}
	snprintf(cachedata,16,"%s%d%s","head:",size,":");
	memcpy(cachedata+16,data,size);
	sendto(sockfd, (char *)cachedata, size+16, 0,(struct sockaddr*)peer, sizeof(struct sockaddr));
	free(cachedata);
	return 0;
}
static void recv_brocastCtr(int sockfd,struct sockaddr_in *peer,char *recvdata){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char IP[20]={0};
	GetNetworkcardIp("apcli0",IP);
	//GetNetworkcardIp("eth2.2",IP);
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "brocast");
	cJSON_AddStringToObject(pItem, "ip",IP); 
	cJSON_AddNumberToObject(pItem, "port", 20000);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	printf("recv_brocastCtr :\n%s(%d)\n",szJSON,strlen(szJSON));
	int i;
	for(i=0;i<4;i++){
		SendTo(sockfd, (char *)szJSON, strlen(szJSON),peer);
		usleep(10*1000);
	}
	cJSON_Delete(pItem);
	free(szJSON);
}

static void test_brocastCtr(int sockfd,struct sockaddr_in *peer,char *recvdata){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char IP[20]={0};
	GetNetworkcardIp("apcli0",IP);
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "TestNet");
	cJSON_AddStringToObject(pItem, "ip",IP); 
	cJSON_AddNumberToObject(pItem, "port", 20000);
	if(!strcmp(IP,""))
	{
		cJSON_AddStringToObject(pItem, "status","failed");
	}else{
		cJSON_AddStringToObject(pItem, "status","ok");
	}
	szJSON = cJSON_Print(pItem);
	printf("test_brocastCtr :\n%s(%d)\n",szJSON,strlen(szJSON));
	int i;
	for(i=0;i<2;i++){
		SendTo(sockfd, (char *)szJSON, strlen(szJSON),peer);
		usleep(10*1000);
		}
	cJSON_Delete(pItem);
	free(szJSON);
}

/*
@��������:	�����Ӽ������Լ����غ���
@����:	size �����Ӽ���־λ
@		0	������
@		1	������
*/
void ack_VolCtr(char *dir,int data){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "vol");
	cJSON_AddStringToObject(pItem, "dir", dir);
	cJSON_AddNumberToObject(pItem, "data", VOL_DATA(data));
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);		
	DEBUG_TCP("ack_VolCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
/*
@��������:	���ػ����غ���
@����:	size ��Ů����־λ
@		0	��
@		1	��
@		recvdata ���ػ�ʱ��
*/
void ack_hostCtr(int sockfd,char *recvdata,char *type){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	char buffer[32];
	memset(buffer,0,32);
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "host");
	cJSON_AddStringToObject(pItem, "type", type);
	cJSON_AddStringToObject(pItem, "time",recvdata);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_hostCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
/*
@��������:	��ص������غ���
@����:	recvdata ��ص���
*/
void ack_batteryCtr(int recvdata,int power){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "battery");
	cJSON_AddNumberToObject(pItem, "state",recvdata);
	cJSON_AddNumberToObject(pItem, "power",power);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_batteryCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}

/*
@��������:	������Ϣ���غ���
@����:	recvdata ��ص���
*/
void ack_alluserCtr(const int sockfd,int state,int power){
	char stropenTime[10]={0};
	char strcloseTime[10]={0};
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "sys");
	cJSON_AddNumberToObject(pItem, "state",state);
	cJSON_AddNumberToObject(pItem, "power",power);
	Get_OpenCloseTime_formRouteTable(OPEN_TIME,stropenTime);
	cJSON_AddStringToObject(pItem, "openTime",stropenTime);
	Get_OpenCloseTime_formRouteTable(CLOSE_TIME,strcloseTime);
	cJSON_AddStringToObject(pItem, "closeTime",strcloseTime);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_alluserCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
#ifdef SPEEK_VOICES
	SendtoaliyunServices(szJSON,strlen(szJSON));	//���͸�΢��
#endif
	cJSON_Delete(pItem);
	free(szJSON);
}
static void CreateState(cJSON* pItem,unsigned char playState){
	if(playState==STREAM_EXIT){
		cJSON_AddStringToObject(pItem, "state","stop");
	}else if(playState==STREAM_PLAY){
		cJSON_AddStringToObject(pItem, "state","play");
	}else if(playState==STREAM_PAUSE){
		cJSON_AddStringToObject(pItem, "state","pause");
	}else if(playState==STREAM_NEXT){
		cJSON_AddStringToObject(pItem, "state","switch");
	}
}
void ack_allplayerCtr(void *data,Player_t *player){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int sockfd = *(int *)data;
	
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");
	CreateState(pItem,player->playState);

	cJSON_AddStringToObject(pItem, "url",player->playfilename);
	cJSON_AddNumberToObject(pItem, "lock",gpio_look);
	cJSON_AddNumberToObject(pItem, "vol",VOL_DATA(player->vol));
	cJSON_AddStringToObject(pItem, "name",player->musicname);
	cJSON_AddNumberToObject(pItem, "time",(int)player->musicTime);
	cJSON_AddNumberToObject(pItem, "progress",player->progress);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_allplayerCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}

void ack_playCtr(int nettype,Player_t *play,unsigned char playState){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");
	CreateState(pItem,playState);
	if(playState==STREAM_EXIT){
#ifndef CLOSE_VOICE 
		Mute_voices(MUTE);
#endif
		CleanI2S_PlayCachedata();
	}
	cJSON_AddStringToObject(pItem, "url",play->playfilename);
	cJSON_AddStringToObject(pItem, "name",play->musicname);
	cJSON_AddNumberToObject(pItem, "time",(int)play->musicTime);
	cJSON_AddNumberToObject(pItem, "progress",(int)play->progress);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	//DEBUG_TCP("ack_playCtr: %s\n",szJSON);
	if(nettype)
		UdpAll_Ack(szJSON,strlen(szJSON));
	else
		sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}	

void ack_gpioCtr(int recvdata){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "lock");
	cJSON_AddNumberToObject(pItem, "state",recvdata);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_gpioCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}

void GetNetState(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",GET_NET_STATE);
	szJSON = cJSON_Print(pItem);
	SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
#ifdef SPEEK_VOICES
void uploadVoicesToaliyun(const char *filename,int fileSize){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "speek");
	cJSON_AddStringToObject(pItem, "filename",filename);
	cJSON_AddNumberToObject(pItem, "fileSize",fileSize);
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
void BindDevToaliyun(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "binddev");
	cJSON_AddStringToObject(pItem, "list","abc");
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
void Ack_CallDev(int recvdata){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "call");
	cJSON_AddStringToObject(pItem, "status","cancel");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_gpioCtr: %s\n",szJSON);
	SendtoaliyunServices(szJSON,strlen(szJSON));	//���͸�΢��
	cJSON_Delete(pItem);
	free(szJSON);
}
#endif
#ifdef CLOCKTOALIYUN
void CloseSystemSignToaliyun(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "clock");
	cJSON_AddStringToObject(pItem, "status","close");
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	test_clock_Interfaces((const char * )szJSON);
	cJSON_Delete(pItem);
	free(szJSON);
}
void SetClockToaliyun(unsigned char clocknum,unsigned char state,const char *time,const char *ringPath){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "clock");
	cJSON_AddStringToObject(pItem, "status","set");
	cJSON_AddNumberToObject(pItem, "clocknum",clocknum);
	cJSON_AddNumberToObject(pItem, "state",state);
	cJSON_AddStringToObject(pItem, "time",time);
	cJSON_AddStringToObject(pItem, "ringPath",ringPath);
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
#endif
void handler_CtrlMsg(int sockfd,char *recvdata,int size,struct sockaddr_in *peer){
	cJSON * pJson = cJSON_Parse(recvdata);
	if(NULL == pJson){
		return ;
	}
	//DEBUG_TCP("handler_CtrlMsg = %s\n",recvdata);
	cJSON * pSub = cJSON_GetObjectItem(pJson, "handler");
	if(NULL == pSub){
		DEBUG_TCP("get json data  failed\n");
		goto exit;
	}
	if(!strcmp(pSub->valuestring,"vol")){
		// get number from json
		DEBUG_TCP("handler_CtrlMsg = %s\n",recvdata);
		pSub = cJSON_GetObjectItem(pJson, "dir");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		DEBUG_TCP("dir = %s\n", pSub->valuestring);
		if(!strcmp(pSub->valuestring,"add")){
			Setwm8960Vol(VOL_ADD,0);
			ack_VolCtr("add",GetVol());//----------->������
		}else if (!strcmp(pSub->valuestring,"sub")){
			Setwm8960Vol(VOL_SUB,0);
			ack_VolCtr("sub",GetVol());//----------->������
		}else if (!strcmp(pSub->valuestring,"no")){
			Setwm8960Vol(VOL_SET,cJSON_GetObjectItem(pJson, "data")->valueint);
			ack_VolCtr("no",GetVol());//----------->���ù̶�����
		}
	}//end vol ������С
	else if(!strcmp(pSub->valuestring,"lock")){
		pSub = cJSON_GetObjectItem(pJson, "state");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(pSub->valueint==0){
			gpio_look=0;
			enable_gpio();//----------->����
			ack_gpioCtr(0);
		}else if (pSub->valueint==1){
			disable_gpio();//----------->����
			gpio_look=1;
			ack_gpioCtr(1);
		}
	}//end lock ������
	else if(!strcmp(pSub->valuestring,"mplayer")){
		pSub = cJSON_GetObjectItem(pJson, "state");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"get")){
			getStreamState(&sockfd,ack_allplayerCtr);//----------->app��½��ȡ��������Ϣ
		}else if(!strcmp(pSub->valuestring,"switch")){
			//mute_recorde_vol(UNMUTE);
			Player_t *player = (Player_t *)calloc(1,sizeof(Player_t));
			if(player==NULL){
				perror("calloc error !!!");
				return;
			}
			char *musicname=NULL;
			if(cJSON_GetObjectItem(pJson, "name")!=NULL){
				musicname=cJSON_GetObjectItem(pJson, "name")->valuestring;
			}
			if(player){					//������jsonЭ�飬����ͬ������ 2016-10-11 14:00
				snprintf(player->playfilename,128,"%s",cJSON_GetObjectItem(pJson, "url")->valuestring);
				if(musicname!=NULL){
					snprintf(player->musicname,48,"%s",musicname);
					player->musicTime = cJSON_GetObjectItem(pJson, "time")->valueint;
				}

				Create_playMusicEvent(player,0);
			}
		}else if (!strcmp(pSub->valuestring,"pause")){
			//mute_recorde_vol(MUTE);
			StreamPause();
		}else if (!strcmp(pSub->valuestring,"stop")){
			//mute_recorde_vol(MUTE);
			Create_CleanUrlEvent();
		}else if (!strcmp(pSub->valuestring,"play")){
			//mute_recorde_vol(UNMUTE);
			StreamPlay();
		}else if (!strcmp(pSub->valuestring,"seekto")){
			seekToStream(cJSON_GetObjectItem(pJson, "progress")->valueint);
		}
	}//end mplayer ��������Ϣ
	else if(!strcmp(pSub->valuestring,"host")){
		pSub = cJSON_GetObjectItem(pJson, "type");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"open")){
			//��ʱ����
			char *stropenTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("stropenTime = %s\n",stropenTime);
			Save_OpenCloseTime_toRouteTable(OPEN_TIME,stropenTime);
			ack_hostCtr(sockfd,stropenTime,"open");
			
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(1,stropenTime);//----------->��ʱ����
			usleep(100*1000);
		}else if (!strcmp(pSub->valuestring,"close")){
			//��ʱ�ػ�
			char *strcloseTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("strcloseTime = %s\n",strcloseTime);
			Save_OpenCloseTime_toRouteTable(CLOSE_TIME,strcloseTime);
			ack_hostCtr(sockfd,strcloseTime,"close");
			
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(MUC_CLOSE_SYSTEM,strcloseTime);//----------->��ʱ�ػ�
			usleep(100*1000);
		}
	}//end host ��ʱ���ػ�
	
	//----------------------------------> udp msg	
	else if(!strcmp(pSub->valuestring,"brocast")){
		// get number from json
		pSub = cJSON_GetObjectItem(pJson, "ip");
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"null"))
		{
			recv_brocastCtr(sockfd,peer,recvdata);
		}
	}//  end brocast	
	else if(!strcmp(pSub->valuestring,"updateHost")){	//----------->�ɰ汾�����̷��͹���
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"newversion")){	//���°汾����Ҫ����
			Create_PlaySystemEventVoices(UPDATA_NEW_PLAY);
		}else if(!strcmp(pSub->valuestring,"start")){	//�������ع̼�
			Create_PlaySystemEventVoices(DOWNLOAD_ING_PLAY);
		}else if(!strcmp(pSub->valuestring,"error")){	//���ع̼�����
			Create_PlaySystemEventVoices(DOWNLOAD_ERROE_PLAY);
		}else if(!strcmp(pSub->valuestring,"end")){ 	//���ع̼�����
			Create_PlaySystemEventVoices(DOWNLOAD_END_PLAY);
		}else if(!strcmp(pSub->valuestring,"progress")){		//���ؽ���
			pSub = cJSON_GetObjectItem(pJson, "value");
			if(pSub->valueint==25){
				Create_PlaySystemEventVoices(DOWNLOAD_25_PLAY);			
			}else if(pSub->valueint==50){
				Create_PlaySystemEventVoices(DOWNLOAD_50_PLAY);	
			}else if(pSub->valueint==75){
				Create_PlaySystemEventVoices(DOWNLOAD_75_PLAY);
			}
		}
	}//  end updateHost
	else if(!strcmp(pSub->valuestring,"updateImage")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"start")){ 		//��ʼ���¹̼�
			Create_PlaySystemEventVoices(UPDATA_START_PLAY);
		}else if(!strcmp(pSub->valuestring,"error")){	//���¹̼�����
			Create_PlaySystemEventVoices(UPDATA_ERROR_PLAY);
		}else if(!strcmp(pSub->valuestring,"end")){ 	//���¹̼�����
			Create_PlaySystemEventVoices(UPDATA_END_PLAY);
		}
	}//  end updateImage                // end----------->�ɰ汾�����̷��͹���
	else if(!strcmp(pSub->valuestring,"newImage")){	// app��ȷ�ϻ���ȡ�����²���
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){		//app ȷ�ϸ���
			
		}else if(!strcmp(pSub->valuestring,"miss")){//app ȡ������

		}
	}else if(!strcmp(pSub->valuestring,"ServerWifi")){
		int event = cJSON_GetObjectItem(pJson, "event")->valueint;
		Create_PlaySystemEventVoices(event);
		if(CONNECT_OK==event){
			//Link_NetworkOk();
		}else if(NOT_NETWORK==event){
			//Link_NetworkError();
		}
	}else if(!strcmp(pSub->valuestring,"TestNet")){
		test_brocastCtr(sockfd,peer,recvdata);
	}else if(!strcmp(pSub->valuestring,"qtts")){
		Create_PlayQttsEvent(cJSON_GetObjectItem(pJson, "text")->valuestring,QTTS_UTF8);
#ifdef	SYSTEMLOCK
		if(!strcmp((cJSON_GetObjectItem(pJson, "text")->valuestring),"***rihuiwangxun_open***"))
			setSystemLock(0);
		if(!strcmp((cJSON_GetObjectItem(pJson, "text")->valuestring),"***rihuiwangxun_close***"))
			setSystemLock(SYSTEMLOCKNUM);
#endif
#ifdef CLOCKTOALIYUN
		if(strstr((cJSON_GetObjectItem(pJson, "text")->valuestring),"set_clock")){
			char *data=cJSON_GetObjectItem(pJson, "text")->valuestring;
			char time[12]={0};
			char path[12]={0};
			char state[12]={0};
			char clocknum[12]={0};
			sscanf(data,"set_clock:%[^:]:%[^:]:%[^:]:%s",clocknum,state,path,time);
			SetClockToaliyun(atoi(clocknum),atoi(state),time,path);
		}
#endif
	}
#ifdef SPEEK_VOICES		//΢�ŶԽ�
	else if (!strcmp(pSub->valuestring,"speek")){
		CreatePlayWeixinVoicesSpeekEvent((const char *)cJSON_GetObjectItem(pJson, "file")->valuestring);
	}
	else if (!strcmp(pSub->valuestring,"binddev")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"failed")){
			Create_PlayQttsEvent(TULING_PLAY_TEXT_WEIXIN_FAILED,QTTS_GBK);
		}else if(!strcmp(pSub->valuestring,"ask")){
			EnableBindDev();
			Create_PlaySystemEventVoices(BIND_SSID_PLAY);
		}
	}
	else if (!strcmp(pSub->valuestring,"call")){		//΢�Ž��淢�͹����ĺ�������
		EnableCallDev();
		Create_PlaySystemEventVoices(TALK_CONFIRM_PLAY);
	}
	else if (!strcmp(pSub->valuestring,"uploadfile")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){			//���ͳɹ�
		}else if(!strcmp(pSub->valuestring,"failed")){	//����ʧ��
		}else if(!strcmp(pSub->valuestring,"timeout")){	//����ʧ��
			Create_PlaySystemEventVoices(SEND_LINK_ER_PLAY);//"��ǰ���绷�����������ʧ�ܣ��������硣"
		}
	}
	else if (!strcmp(pSub->valuestring,"clock")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){			//���㱨����
			//������
			char *path=NULL;
			if(cJSON_GetObjectItem(pJson, "path")!=NULL){
				path=cJSON_GetObjectItem(pJson, "path")->valuestring;
			}
			CreatePlayWeixinVoicesSpeekEvent(path);
		}else if(!strcmp(pSub->valuestring,"close")){	//�ر����ÿ���ʱ��
			//
			char *time_open=NULL;
			if(cJSON_GetObjectItem(pJson, "time")!=NULL){
				time_open=cJSON_GetObjectItem(pJson, "time")->valuestring;
			}
			printf("clock close (%s)...\n",time_open);
			usleep(500*1000);
			SocSendMenu(3,0);
			usleep(100*1000);
			test_Clock_saveLoute((const char *)time_open);
			SocSendMenu(7,time_open);	//�������ӿ���ʱ��
	
		}
	}else if(!strcmp(pSub->valuestring,"getdev")){	//΢�Ż�ȡ�豸��Ϣ
		ack_alluserCtr(sockfd,Get_batteryVaule(),get_dc_state());
	}
#endif	
	else if (!strcmp(pSub->valuestring,"tuling")){
		Write_tulinglog("recv tuling val\n");
		pSub = cJSON_GetObjectItem(pJson, "userId");
		if(pSub){
			char *userId= pSub->valuestring;
			char *token= cJSON_GetObjectItem(pJson, "token")->valuestring;
			Load_useridAndToken((const char *) userId,(const char *) token);
		}
	}else if(!strcmp(pSub->valuestring,"downmp3")){	//΢�Ŷ����ظ����¼����Ѿ����ص� /Down/ Ŀ¼��
		char *status= cJSON_GetObjectItem(pJson, "status")->valuestring;
		if(!strcmp(status,"ok")){	//�Ѿ�������
			char *cacheMp3file= cJSON_GetObjectItem(pJson, "mp3file")->valuestring;
#ifdef PALY_URL_SD			
			Create_SaveWeixinDownMp3_EventToMainQueue(cacheMp3file);
#endif
		}else{
		}
	}
#if defined(HUASHANG_JIAOYU)	//��������ʶ��ӿڣ�ʶ������Ľ��
#ifdef XUN_FEI_OK
	else if(!strcmp(pSub->valuestring,"xunfei")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(pSub){
			if(!strcmp(pSub->valuestring,"ok"))	//��ȷʶ������ŵ�����
				GetHuashang_xunfei_aifiVoices(cJSON_GetObjectItem(pJson, "musicname")->valuestring,cJSON_GetObjectItem(pJson, "playindex")->valueint);
			else
				GetHuashang_xunfei_aifiFailed();
		}
	}
#endif	
#endif	
exit:
	cJSON_Delete(pJson);
}
