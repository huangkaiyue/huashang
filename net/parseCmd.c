#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "base/pool.h"
#include "base/cJSON.h"
#include "network.h"
#include "../uart/uart.h"
#include "madplay.h"
#include "StreamFile.h"
#include "host/voices/wm8960i2s.h"

#define STREAM_EXIT				MAD_EXIT	//ֹͣ	
#define STREAM_PLAY 			MAD_PLAY	//����
#define STREAM_PAUSE			MAD_PAUSE	//��ͣ
#define STREAM_NEXT				MAD_NEXT
#define QTTS_SYS	0
#define QTTS_APP	1

#define 	GET_NET_STATE	29	//��ȡ����״̬

//#define DBG_TCP
#ifdef DBG_TCP
#define DEBUG_TCP(fmt, args...) printf("network: " fmt, ## args)
#else   
#define DEBUG_TCP(fmt, args...) { }
#endif	//end DBG_AP_STA

#define VOL_DATA(x) (x-77)*2

unsigned char gpio_look=0;
/*
@��������:	�����Ӽ������Լ����غ���
@����:	size �����Ӽ���־λ
@		0	������
@		1	������
*/
void ack_VolCtr(int sockfd,char *dir,int data)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
//	DEBUG_TCP("ack_VolCtr : add and sub !!!\n");
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "vol");
	cJSON_AddStringToObject(pItem, "dir", dir);
	cJSON_AddNumberToObject(pItem, "data", VOL_DATA(data));
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);		
	DEBUG_TCP("ack_VolCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
#ifdef VOICS_CH
/*
@��������:	��Ů�������Լ����غ���
@����:	size ��Ů����־λ
@		1	����
@		0	Ů��
*/
void ack_chCtr(int sockfd,int type)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "setvoices");	
	cJSON_AddNumberToObject(pItem, "type", type); 
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_chCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
#endif //end VOICS_CH
/*
@��������:	���ػ����غ���
@����:	size ��Ů����־λ
@		0	��
@		1	��
@		recvdata ���ػ�ʱ��
*/
void ack_hostCtr(int sockfd,char *recvdata,char *type)
{
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
}
/*
@��������:	��ص������غ���
@����:	recvdata ��ص���
*/
void ack_batteryCtr(int recvdata,int power)
{
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
}
/*
@��������:	������Ϣ���غ���
@����:	recvdata ��ص���
*/
void ack_alluserCtr(const int sockfd,int state,int power)
{
	char stropenTime[10]={0};
	char strcloseTime[10]={0};
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "sys");
	cJSON_AddNumberToObject(pItem, "state",state);
	cJSON_AddNumberToObject(pItem, "power",power);
	get_host_time(1,stropenTime);
	cJSON_AddStringToObject(pItem, "openTime",stropenTime);
	get_host_time(0,strcloseTime);
	cJSON_AddStringToObject(pItem, "closeTime",strcloseTime);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_alluserCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
static void CreateState(cJSON* pItem,unsigned char playState)
{
	if(playState==STREAM_EXIT){
		cJSON_AddStringToObject(pItem, "state","stop");
		DEBUG_TCP("========================stop mp3 !!!================================\n");
#ifdef CLOSE_VOICE
#ifdef MUTE_TX
		mute_recorde_vol(1);
#endif
		close_wm8960_voices();
		//SET_MUTE_DISABLE();
#endif
	}else if(playState==STREAM_PLAY){
		cJSON_AddStringToObject(pItem, "state","play");
	}else if(playState==STREAM_PAUSE){
		cJSON_AddStringToObject(pItem, "state","pause");
	}else if(playState==STREAM_NEXT){
		cJSON_AddStringToObject(pItem, "state","switch");
	}
}
void ack_allplayerCtr(void *data,Player_t *player)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int sockfd = *(int *)data;
	
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");
	CreateState(pItem,player->playState);

	cJSON_AddStringToObject(pItem, "url",player->playfilename);
	cJSON_AddNumberToObject(pItem, "lock",gpio_look);
	cJSON_AddNumberToObject(pItem, "vol",VOL_DATA(player->vol));
	cJSON_AddNumberToObject(pItem, "progress",player->progress);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_allplayerCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
typedef struct{
	char *url;
	char *state;
}AckState;

#if 0
void ack_playCtr(char *url,unsigned char state)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");
	CreateState(pItem,state);
	cJSON_AddStringToObject(pItem, "url",url);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_playCtr: %s\n",szJSON);
	sendAll_Ack(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
#else
void ack_playCtr(int nettype,Player_t *play,unsigned char playState)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");
	CreateState(pItem,playState);
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
}	
#endif
void ack_gpioCtr(int recvdata)
{
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
}

void GetNetState(void)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "ServerWifi");
	cJSON_AddNumberToObject(pItem, "event",GET_NET_STATE);
	szJSON = cJSON_Print(pItem);
	SendtoServicesWifi(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
#ifdef SPEEK_VOICES
void uploadVoicesToaliyun(const char *filename){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "speek");
	cJSON_AddStringToObject(pItem, "filename",filename);
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
}
#endif

void handler_CtrlMsg(int sockfd,char *recvdata,int size,struct sockaddr_in *peer)
{
	cJSON * pJson = cJSON_Parse(recvdata);
	if(NULL == pJson)
	{
		return -1;
	}
	//DEBUG_TCP("handler_CtrlMsg = %s\n",recvdata);
	cJSON * pSub = cJSON_GetObjectItem(pJson, "handler");
	if(NULL == pSub)
	{
		DEBUG_TCP("get json data  failed\n");
		goto exit;
	}
	if(!strcmp(pSub->valuestring,"vol"))
	{
		// get number from json
		DEBUG_TCP("handler_CtrlMsg = %s\n",recvdata);
		pSub = cJSON_GetObjectItem(pJson, "dir");
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		DEBUG_TCP("dir = %s\n", pSub->valuestring);
		if(!strcmp(pSub->valuestring,"add")){
			SetVol(VOL_ADD,0);
			ack_VolCtr(sockfd,"add",GetVol());//----------->������
		}else if (!strcmp(pSub->valuestring,"sub")){
			SetVol(VOL_SUB,0);
			ack_VolCtr(sockfd,"sub",GetVol());//----------->������
		}else if (!strcmp(pSub->valuestring,"no")){
			SetVol(VOL_SET,cJSON_GetObjectItem(pJson, "data")->valueint);
			ack_VolCtr(sockfd,"no",GetVol());//----------->���ù̶�����
		}
	}//end vol ������С
#ifdef VOICS_CH
	else if(!strcmp(pSub->valuestring,"setvoices")){
		pSub = cJSON_GetObjectItem(pJson, "type");
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(pSub->valueint==1)
		{
			set_vol_ch(0);
			ack_chCtr(sockfd,1);//----------->����
		}else if (pSub->valueint==2){
			set_vol_ch(1);
			ack_chCtr(sockfd,0);//----------->Ů��
		}
	}//end setvoices
#endif	//end VOICS_CH
	else if(!strcmp(pSub->valuestring,"lock")){
		pSub = cJSON_GetObjectItem(pJson, "state");
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(pSub->valueint==0)
		{
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
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"get"))
		{
			getStreamState(&sockfd,ack_allplayerCtr);//----------->app��½��ȡ��������Ϣ
		}else if(!strcmp(pSub->valuestring,"switch")){
			mute_recorde_vol(UNMUTE);
			Player_t *player = (Player_t *)calloc(1,sizeof(Player_t));
			char *musicname =cJSON_GetObjectItem(pJson, "name")->valuestring;
			if(player){					//������jsonЭ�飬����ͬ������ 2016-10-11 14:00
				snprintf(player->playfilename,128,"%s",cJSON_GetObjectItem(pJson, "url")->valuestring);
				if(musicname){
					snprintf(player->musicname,64,"%s",musicname);
					player->musicTime = cJSON_GetObjectItem(pJson, "time")->valueint;
				}
				createPlayEvent(player);
			}
		}else if (!strcmp(pSub->valuestring,"pause")){
			mute_recorde_vol(MUTE);
			StreamPause();
		}else if (!strcmp(pSub->valuestring,"stop")){
			mute_recorde_vol(MUTE);
			CleanUrlEvent();
		}else if (!strcmp(pSub->valuestring,"play")){
			mute_recorde_vol(UNMUTE);
			StreamPlay();
		}else if (!strcmp(pSub->valuestring,"seekto")){
			seekToStream(cJSON_GetObjectItem(pJson, "progress")->valueint);
		}
	}//end mplayer ��������Ϣ
	else if(!strcmp(pSub->valuestring,"host")){
		pSub = cJSON_GetObjectItem(pJson, "type");
		if(NULL == pSub)
		{
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"open"))
		{
			//��ʱ����
			char *stropenTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("stropenTime = %s\n",stropenTime);
			set_host_time(1,stropenTime);
			ack_hostCtr(sockfd,stropenTime,"open");
#ifdef UART
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(1,stropenTime);//----------->��ʱ����
			usleep(100*1000);
#endif
		}else if (!strcmp(pSub->valuestring,"close")){
			//��ʱ�ػ�
			char *strcloseTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("strcloseTime = %s\n",strcloseTime);
			set_host_time(0,strcloseTime);
			ack_hostCtr(sockfd,strcloseTime,"close");
			
#ifdef UART
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(2,strcloseTime);//----------->��ʱ�ػ�
			usleep(100*1000);
#endif
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
			QttsPlayEvent("���°汾����Ҫ����",QTTS_SYS);//send to app
		}else if(!strcmp(pSub->valuestring,"start")){	//�������ع̼�
			QttsPlayEvent("�������ع̼�",QTTS_SYS);
		}else if(!strcmp(pSub->valuestring,"error")){	//���ع̼�����
			QttsPlayEvent("���ع̼�����",QTTS_SYS);
		}else if(!strcmp(pSub->valuestring,"end")){ 	//���ع̼�����
			QttsPlayEvent("���ع̼�����",QTTS_SYS);
		}else if(!strcmp(pSub->valuestring,"progress")){		//���ؽ���
			pSub = cJSON_GetObjectItem(pJson, "value");
			if(pSub->valueint==25)
				QttsPlayEvent("���ص��ٷ�֮��ʮ��",QTTS_SYS);			
			else if(pSub->valueint==50)
				QttsPlayEvent("���ص��ٷ�֮��ʮ",QTTS_SYS);		
			else if(pSub->valueint==75)
				QttsPlayEvent("���ص��ٷ�֮��ʮ��",QTTS_SYS);
		}
	}//  end updateHost
	else if(!strcmp(pSub->valuestring,"updateImage")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"start")){ 		//��ʼ���¹̼�
			QttsPlayEvent("��ʼ���¹̼�",QTTS_SYS);
		}else if(!strcmp(pSub->valuestring,"error")){	//���¹̼�����
			QttsPlayEvent("���¹̼�����",QTTS_SYS);
		}else if(!strcmp(pSub->valuestring,"end")){ 	//���¹̼�����
			//QttsPlayEvent("���¹̼�����",QTTS_SYS);
			create_event_system_voices(6);
		}
	}//  end updateImage                // end----------->�ɰ汾�����̷��͹���
	else if(!strcmp(pSub->valuestring,"newImage")){	// app��ȷ�ϻ���ȡ�����²���
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){		//app ȷ�ϸ���
			
		}else if(!strcmp(pSub->valuestring,"miss")){//app ȡ������

		}
	}else if(!strcmp(pSub->valuestring,"ServerWifi")){
		int event = cJSON_GetObjectItem(pJson, "event")->valueint;
		create_event_system_voices(event);
	}else if(!strcmp(pSub->valuestring,"TestNet")){
		test_brocastCtr(sockfd,peer,recvdata);
	}else if(!strcmp(pSub->valuestring,"qtts")){
		QttsPlayEvent(cJSON_GetObjectItem(pJson, "text")->valuestring,QTTS_APP);
	}
#ifdef SPEEK_VOICES
	else if (!strcmp(pSub->valuestring,"speek")){
		CreateSpeekEvent((const char *)cJSON_GetObjectItem(pJson, "file")->valuestring);
	}
#endif	
exit:
	cJSON_Delete(pJson);
	return 0;
}
