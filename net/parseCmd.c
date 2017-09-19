#include "comshead.h"
#include "base/pool.h"
#include "base/cJSON.h"
#include "network.h"
#include "mplay.h"
#include "StreamFile.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/qtts_qisc.h"
#include "host/studyvoices/prompt_tone.h"
#include "../host/voices/gpio_7620.h"
#include "host/ap_sta.h"
#include "host/voices/callvoices.h"
#include "sysdata.h"
#include "uart/uart.h"
#include "config.h"

#define STREAM_EXIT				MAD_EXIT	//停止	
#define STREAM_PLAY 			MAD_PLAY	//播放
#define STREAM_PAUSE			MAD_PAUSE	//暂停
#define STREAM_NEXT				MAD_NEXT

#define 	GET_NET_STATE	29	//获取网络状态

//#define DBG_TCP
#ifdef DBG_TCP
#define DEBUG_TCP(fmt, args...) printf("network: " fmt, ## args)
#else   
#define DEBUG_TCP(fmt, args...) { }
#endif	//end DBG_AP_STA

#define VOL_DATA(x) (x-90)*4

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
@函数功能:	音量加减处理以及返回函数
@参数:	size 音量加减标志位
@		0	音量减
@		1	音量加
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
@函数功能:	开关机返回函数
@参数:	size 男女音标志位
@		0	开
@		1	关
@		recvdata 开关机时间
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
@函数功能:	电池电量返回函数
@参数:	recvdata 电池电量
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
void sendDeviceState(void){
	char stropenTime[10]={0};
	char strcloseTime[10]={0};
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "sys");
	cJSON_AddNumberToObject(pItem, "state",Get_batteryVaule());
	//cJSON_AddNumberToObject(pItem, "state",75);
	cJSON_AddNumberToObject(pItem, "power",get_dc_state());
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_alluserCtr: %s\n",szJSON);
	SendtoaliyunServices(szJSON,strlen(szJSON));	//发送给微信
	cJSON_Delete(pItem);
	free(szJSON);
}
/*
@函数功能:	所有信息返回函数
@参数:	recvdata 电池电量
*/
void ack_alluserCtr(const int sockfd,int state,int power){
	char stropenTime[10]={0};
	char strcloseTime[10]={0};
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "sys");
	cJSON_AddNumberToObject(pItem, "state",state);
	cJSON_AddStringToObject(pItem, "openTime","09:00");
	cJSON_AddStringToObject(pItem, "closeTime","22:00");
	cJSON_AddNumberToObject(pItem, "power",power);
	cJSON_AddStringToObject(pItem, "status","ok");
	szJSON = cJSON_Print(pItem);
	DEBUG_TCP("ack_alluserCtr: %s\n",szJSON);
	send_ctrl_ack(sockfd,szJSON,strlen(szJSON));
	SendtoaliyunServices(szJSON,strlen(szJSON));	//发送给微信
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
	cJSON_AddNumberToObject(pItem, "lock",0);
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
		Mute_voices(MUTE);
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
	SendtoaliyunServices(szJSON,strlen(szJSON));	//发送给微信
	cJSON_Delete(pItem);
	free(szJSON);
}
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
//清除微信绑定的用户
void ResetWeixinBindUserMessage(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "cleanbind");
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
void unlockWeixin(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "unlockweixin");
	szJSON = cJSON_Print(pItem);
	SendtoaliyunServices(szJSON,strlen(szJSON));
	cJSON_Delete(pItem);
	free(szJSON);
}
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
			ack_VolCtr("add",GetVol());//----------->音量加
		}else if (!strcmp(pSub->valuestring,"sub")){
			Setwm8960Vol(VOL_SUB,0);
			ack_VolCtr("sub",GetVol());//----------->音量减
		}else if (!strcmp(pSub->valuestring,"no")){
			Setwm8960Vol(VOL_APP_SET,cJSON_GetObjectItem(pJson, "data")->valueint);
			ack_VolCtr("no",GetVol());//----------->设置固定音量
		}
	}//end vol 音量大小
	else if(!strcmp(pSub->valuestring,"lock")){

	}//end lock 按键锁
	else if(!strcmp(pSub->valuestring,"mplayer")){
		pSub = cJSON_GetObjectItem(pJson, "state");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"get")){
			getStreamState(&sockfd,ack_allplayerCtr);//----------->app登陆获取播放器信息
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
			if(player){					//新增加json协议，用于同步界面 2016-10-11 14:00
				char *url =cJSON_GetObjectItem(pJson, "url")->valuestring;
				if(strstr(url,"http")==NULL){			
					CreateSystemPlay_ProtectMusic((const char *)AMR_NOT_MUSIC);
					goto exit;
				}
				snprintf(player->playfilename,128,"%s",url);
				if(musicname!=NULL){
					snprintf(player->musicname,48,"%s",musicname);
					player->musicTime = cJSON_GetObjectItem(pJson, "time")->valueint;
				}

				__AddNetWork_UrlForPaly(player);
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
	}//end mplayer 播放器信息
	else if(!strcmp(pSub->valuestring,"host")){
		pSub = cJSON_GetObjectItem(pJson, "type");
		if(NULL == pSub){
			DEBUG_TCP("get vol failed\n");
			goto exit;
		}
		if(!strcmp(pSub->valuestring,"open")){
			//定时开机
			char *stropenTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("stropenTime = %s\n",stropenTime);
			Save_OpenCloseTime_toRouteTable(OPEN_TIME,stropenTime);
			ack_hostCtr(sockfd,stropenTime,"open");
			
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(1,stropenTime);//----------->定时开机
			usleep(100*1000);
		}else if (!strcmp(pSub->valuestring,"close")){
			//定时关机
			char *strcloseTime = cJSON_GetObjectItem(pJson, "time")->valuestring;
			DEBUG_TCP("strcloseTime = %s\n",strcloseTime);
			Save_OpenCloseTime_toRouteTable(CLOSE_TIME,strcloseTime);
			ack_hostCtr(sockfd,strcloseTime,"close");
			
			SocSendMenu(3,0);
			usleep(100*1000);
			SocSendMenu(MUC_CLOSE_SYSTEM,strcloseTime);//----------->定时关机
			usleep(100*1000);
		}
	}//end host 定时开关机
	
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
	else if(!strcmp(pSub->valuestring,"updateImage")){		
		pSub = cJSON_GetObjectItem(pJson, "status");
		Unlock_EventQueue();
		if(!strcmp(pSub->valuestring,"error")){ 			
			CreateSystemPlay_ProtectMusic((const char *)AMR_DOWN_FAILED);		//down failed
		}else if(!strcmp(pSub->valuestring,"failed")){
			CreateSystemPlay_ProtectMusic((const char *)AMR_UPDATE_FAILED);	//update failed	
		}else if(!strcmp(pSub->valuestring,"end")){ 	//更新固件结束
			Create_PlaySystemEventVoices(CMD_90_UPDATE_OK);
		}
	}//  end updateImage                // end----------->由版本监测进程发送过来
	else if(!strcmp(pSub->valuestring,"ServerWifi")){
		RecvNetWorkConnetState(cJSON_GetObjectItem(pJson, "event")->valueint);
	}else if(!strcmp(pSub->valuestring,"TestNet")){
		test_brocastCtr(sockfd,peer,recvdata);
	}else if(!strcmp(pSub->valuestring,"qtts")){
		if(!strncmp(cJSON_GetObjectItem(pJson, "text")->valuestring,"openwifi",8)){
			OpenWifi();
		}
#if 0
		else if(!strncmp(cJSON_GetObjectItem(pJson, "text")->valuestring,"testmsg",7)){
			CreateSystemPlay_ProtectMusic((const char *)AMR_WEIXIN_RECV_OK);
			AddWeiXinpushMessage_Voices("http://huashang-smart-oss.rhwxun.com/dailyShow/voice/a8628c1d845345d7a1c807391a581013.mp3",64); 
			goto exit;
		}
#endif
#ifdef XUNFEI_QTTS
		CreateSystemPlay_ProtectMusic((const char *)AMR_WEIXIN_RECV_OK);
		char *WeiXintxt =cJSON_GetObjectItem(pJson, "text")->valuestring;
		AddWeiXinMessage_Text((const char *)WeiXintxt,strlen(WeiXintxt));
#else
		char *WeiXintxt =cJSON_GetObjectItem(pJson, "text")->valuestring;
		int runLen = 128+strlen(WeiXintxt);
		char *runQtts=(char *)calloc(1,runLen);
		if(runQtts){
			char token[64]={0};
			char user_id[17]={0};
			GetuserTokenValue(user_id,token);
			sprintf(runQtts,"tulingtts %s %s %s &",user_id,token,WeiXintxt);
			system(runQtts);
			usleep(100000);
			free(runQtts);
		}
#endif
	}
	else if (!strcmp(pSub->valuestring,"speek")){//微信对讲
		CreateSystemPlay_ProtectMusic((const char *)AMR_WEIXIN_RECV_OK);
		char *WeiXinFile =cJSON_GetObjectItem(pJson, "file")->valuestring;
		AddWeiXinMessage_Voices((const char *)WeiXinFile,strlen(WeiXinFile));
	}else if (!strcmp(pSub->valuestring,"tulingtts")){	
		CreateSystemPlay_ProtectMusic((const char *)AMR_WEIXIN_RECV_OK);
		char *WeiXinFile =cJSON_GetObjectItem(pJson, "tts")->valuestring;
		pSub = cJSON_GetObjectItem(pJson, "userId");
		if(pSub){
			char *userId= pSub->valuestring;
			char *token= cJSON_GetObjectItem(pJson, "token")->valuestring;
			Load_useridAndToken((const char *) userId,(const char *) token);
		}
		AddWeiXinMessage_Voices((const char *)WeiXinFile,strlen(WeiXinFile));
	}
	else if (!strcmp(pSub->valuestring,"binddev")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"failed")){
			Create_PlayQttsEvent(TULING_PLAY_TEXT_WEIXIN_FAILED,QTTS_GBK);
		}else if(!strcmp(pSub->valuestring,"ask")){
			EnableBindDev();
			Create_PlaySystemEventVoices(CMD_27_RECV_BIND);
		}
	}
	else if (!strcmp(pSub->valuestring,"uploadfile")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){			//发送成功
		}else if(!strcmp(pSub->valuestring,"failed")){	//发送失败
		}else if(!strcmp(pSub->valuestring,"timeout")){	//发送失败
			Create_PlaySystemEventVoices(CMD_29_NETWORK_FAILED);//"当前网络环境差，语音发送失败，请检查网络。"
		}
	}
	else if (!strcmp(pSub->valuestring,"clock")){
		pSub = cJSON_GetObjectItem(pJson, "status");
		if(!strcmp(pSub->valuestring,"ok")){			//到点报语音
			//报语音
			char *path=NULL;
			if(cJSON_GetObjectItem(pJson, "path")!=NULL){
				path=cJSON_GetObjectItem(pJson, "path")->valuestring;
			}
			CreatePlayWeixinVoicesSpeekEvent(path);
		}else if(!strcmp(pSub->valuestring,"close")){	//关闭设置开机时间
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
			SocSendMenu(7,time_open);	//设置闹钟开机时间
	
		}
	}else if(!strcmp(pSub->valuestring,"getdev")){	//微信获取设备信息
		ack_alluserCtr(sockfd,Get_batteryVaule(),get_dc_state());
	}
	else if (!strcmp(pSub->valuestring,"tuling")){
		Write_tulinglog("recv tuling val\n");
		pSub = cJSON_GetObjectItem(pJson, "userId");
		if(pSub){
			char *userId= pSub->valuestring;
			char *token= cJSON_GetObjectItem(pJson, "token")->valuestring;
			Load_useridAndToken((const char *) userId,(const char *) token);
		}
	}else if(!strcmp(pSub->valuestring,"downmp3")){	//微信端下载歌曲事件，已经下载到 /Down/ 目录下
		char *status= cJSON_GetObjectItem(pJson, "status")->valuestring;
		if(!strcmp(status,"ok")){	//已经下载完
			char *cacheMp3file= cJSON_GetObjectItem(pJson, "mp3file")->valuestring;
			remove(cacheMp3file);
		}else{
		}
	}
#if defined(HUASHANG_JIAOYU)	//华上微信端点击播放
	else if(!strcmp(pSub->valuestring,"localMp3")){
		Huashang_WeiXinplayMusic(cJSON_GetObjectItem(pJson, "nums")->valueint);
	}
#endif	
	else if(!strcmp(pSub->valuestring,"everyday")){
		char *url= cJSON_GetObjectItem(pJson, "url")->valuestring;
		if(strstr(url,"http")==NULL){
			CreateSystemPlay_ProtectMusic((const char *)AMR_NOT_MUSIC);
			goto exit;
		}
		CreateSystemPlay_ProtectMusic((const char *)AMR_WEIXIN_RECV_OK);
		AddWeiXinpushMessage_Voices(url,strlen(url));
	}else if(!strcmp(pSub->valuestring,"weixinUpdate")){
		int id = cJSON_GetObjectItem(pJson, "id")->valueint;
		char *url= cJSON_GetObjectItem(pJson, "url")->valuestring;
		char *md5= cJSON_GetObjectItem(pJson, "md5")->valuestring;
		cJSON * pj = cJSON_GetObjectItem(pJson, "mac");
		char *mac=NULL;
		if(pj==NULL){
			goto exit;
		}
		mac=pj->valuestring;
		int size = cJSON_GetObjectItem(pJson, "size")->valueint;
		int versionCode = cJSON_GetObjectItem(pJson, "versionCode")->valueint;
		runSystemUpdateVersion(mac,id,(const char *)url,(const char *)md5,size,versionCode);		
	}
exit:
	cJSON_Delete(pJson);
}
