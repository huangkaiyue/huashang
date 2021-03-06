#include "comshead.h"
#include "base/queWorkCond.h"
#include "host/studyvoices/qtts_qisc.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/callvoices.h"
#include "huashangMusic.h"
#include "StreamFile.h"
#include "base/cJSON.h"

#include "nvram.h"
#include "uart/uart.h"

#define WEIXIN_TEXT 	1
#define WEIXIN_VOICES	2

#define WEIXIN_PLAY_LIST_MAX	20	//允许当前存最大的队列数据

#define NOT_PLAY		0
#define ALREADY_PLAY	1
#define SAMEPLE_PLAY	2

typedef struct {
	void *data;	//添加到队列的消息数据(音频or文字)
	int size;	//添加到对接的数据大小(音频文件 对应--->文件长度  )
	int type;	//添加到队列当中消息类型
}WeiXinMsg;

static WorkQueue *WeixinEvent=NULL;
static WeiXinMsg *Bak_Message=NULL;
static unsigned char newMessageFlag=NOT_MESSAGE;	//新消息标志

typedef struct{
	int playstate;
	char url[256];
}WeixinPushMsg_t;

static WeixinPushMsg_t *PushMsg=NULL;

static void SetWeixinMessageFlag(unsigned char state){
	newMessageFlag=state;
}
int GetWeixinMessageFlag(void){
	return (int)newMessageFlag;
}


static int __AddWeiXinMessage(const char *data,int Size,int type){
	WeiXinMsg *msg = NULL;
	char *Free=NULL;
	int msgSize=0;
	while(1){
		if(getWorkMsgNum(WeixinEvent)>=WEIXIN_PLAY_LIST_MAX){
			getMsgQueue(WeixinEvent,&Free,&msgSize);
			if(Free==NULL){
				continue;
			}
			msg = (WeiXinMsg *)Free;
			if(msg->data){
				if(msg->type ==WEIXIN_VOICES){
					remove(msg->data);//删除语音文件
				}
				free(msg->data);
				msg->data=NULL;
			}	
			free(msg);
			usleep(1000);	
		}else{
			break;
		}	
	}
	msg =(WeiXinMsg *)calloc(1,sizeof(WeiXinMsg));
	if(msg==NULL){
		return -1;
	}
	msg->data=(void *)calloc(1,Size+8);
	if(msg->data==NULL){
		return -1;
	}
	snprintf(msg->data,Size+8,"%s",data);
	msg->type = type;
	msg->size = Size;
	SetWeixinMessageFlag(WEIXIN_MESSAGE);
	return putMsgQueue(WeixinEvent,msg, sizeof(WeiXinMsg));
}
//添加文本消息到队列当中
int AddWeiXinMessage_Text(const char *data,int Size){
	return __AddWeiXinMessage(data,Size,WEIXIN_TEXT);
}
//添加语音消息消息到队列当中
int AddWeiXinMessage_Voices(const char *data,int Size){
	return __AddWeiXinMessage(data,Size,WEIXIN_VOICES);
}

static void SaveHuashangEvenydayMsg(const char *url,int playstate){
	char file[128]={0};
	if(access(TF_SYS_PATH, F_OK)==0){
		snprintf(file,128,"%s%s",TF_SYS_PATH,HUASHANG_DAYSHOW_JSON);
		FILE *fp =fopen(file,"w+");
		if(fp==NULL){
			return ;
		}
		cJSON *root;     
		root=cJSON_CreateObject();  
		cJSON_AddStringToObject(root,"url", url);
		cJSON_AddNumberToObject(root,"playstate",playstate); 
		char *out =cJSON_Print(root);
		fwrite(out,strlen(out),1,fp);
		cJSON_Delete(root); 
		free(out);
		fclose(fp);
	}		
}
static int LoadSdcardEvenydayMsg(WeixinPushMsg_t *PushMsg){
	if(!strcmp(PushMsg->url,"")){
		if(access(TF_SYS_PATH, F_OK)==0){
			char file[128]={0};
			snprintf(file,128,"%s%s",TF_SYS_PATH,HUASHANG_DAYSHOW_JSON);
			char *data =readFileBuf((const char * )file);
			if(data==NULL){
				return -1;
			}
			cJSON *root=cJSON_Parse(data);
			cJSON *url = cJSON_GetObjectItem(root,"url")->valuestring;
			snprintf(PushMsg->url,sizeof(PushMsg->url),"%s",url);
			PushMsg->playstate=cJSON_GetObjectItem(root,"playstate")->valueint;
			return 0;
		}
	}
	return -1;
}

int AddWeiXinpushMessage_Voices(const char *data,int Size){
	if(PushMsg){
		SetWeixinMessageFlag(WEIXIN_PUSH_MESSAGE);
		if(!strcmp(PushMsg->url,data)){
			return -1;
		}
		memset(PushMsg->url,0,sizeof(PushMsg->url));
		snprintf(PushMsg->url,sizeof(PushMsg->url),"%s",data);
		PushMsg->playstate = NOT_PLAY;
		SaveHuashangEvenydayMsg((const char *)PushMsg->url,NOT_PLAY);
	}
}
//获取微信消息队列进行播放
int GetWeiXinMessageForPlay(void){
	WeiXinMsg *msg = NULL;
	char *Get=NULL;
	int msgSize=0;
	time_t t;
	int ret=-1;	
	char bak_voices[256]={0};
	static int ti=1;
	if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
		CreateSystemPlay_ProtectMusic((const char *)AMR_44_WEIXIN_WARN);
		goto exit0;
	}
	if(!LoadSdcardEvenydayMsg(PushMsg)){
		if(PushMsg->playstate==NOT_PLAY){
			SetWeixinMessageFlag(WEIXIN_PUSH_MESSAGE);
		}
	}
	if(GetWeixinMessageFlag()==WEIXIN_PUSH_MESSAGE){
		Player_t *player = (Player_t *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			return -1;
		}
		if(player){				
			snprintf(player->playfilename,128,"%s",PushMsg->url);
			SaveHuashangEvenydayMsg((const char *)PushMsg->url,ALREADY_PLAY);
			if(__AddNetWork_UrlForPaly(player)==0){
				if(getWorkMsgNum(WeixinEvent)>0){
					SetWeixinMessageFlag(WEIXIN_MESSAGE);
				}else{
					SetWeixinMessageFlag(NOT_MESSAGE);
				}
			}
		}		
		return 0;
	}
	if(getWorkMsgNum(WeixinEvent)>0){
		getMsgQueue(WeixinEvent,&Get,&msgSize);
		if(Get==NULL){
			return -1;
		}
		msg = (WeiXinMsg *)Get;
		if(msg->type ==WEIXIN_TEXT){
			Create_PlayQttsEvent(msg->data,QTTS_UTF8);
		}else if(msg->type ==WEIXIN_VOICES){
			CreatePlayWeixinVoicesSpeekEvent((const char *)msg->data);
		}
		if(getWorkMsgNum(WeixinEvent)==0){
			SetWeixinMessageFlag(NOT_MESSAGE);//微信消息已经取完
			if(Bak_Message!=NULL){
				if(Bak_Message->type ==WEIXIN_VOICES)
					remove(Bak_Message->data);
				free(Bak_Message->data);	
				free(Bak_Message);
			}
			if(msg->type ==WEIXIN_TEXT){
				Bak_Message = msg;
			}else if(msg->type ==WEIXIN_VOICES){
				++ti;
				snprintf(bak_voices,256,"cp %s /Down/%d.amr",msg->data,ti);
				system(bak_voices);
				memset(msg->data,0,strlen(msg->data));	
				sprintf(msg->data,"/Down/%d.amr",ti);
				Bak_Message = msg;
			}
		}else{
			free(msg->data);
			free(msg);
		}
	}else{
		if(Bak_Message==NULL){
			//Create_PlayQttsEvent("小朋友没有消息喔！",QTTS_UTF8);
			CreateSystemPlay_ProtectMusic((const char *)AMR_44_WEIXIN_WARN);
		}else{
			if(Bak_Message->type ==WEIXIN_TEXT){
				Create_PlayQttsEvent(Bak_Message->data,QTTS_UTF8);
			}else if(Bak_Message->type ==WEIXIN_VOICES){
				++ti;
				snprintf(bak_voices,256,"cp %s /Down/%d.amr",Bak_Message->data,ti);
				system(bak_voices);
				CreatePlayWeixinVoicesSpeekEvent((const char *)Bak_Message->data);	
				memset(Bak_Message->data,0,strlen(Bak_Message->data));	
				sprintf(Bak_Message->data,"/Down/%d.amr",ti);
			}
		}
	}
	return 0;
exit0:
	return ret;
}
static unsigned char RunState =0;
typedef struct{
	unsigned int versionCode;
	unsigned int  id;
	unsigned int size;	
	char mac[32];
	char url[128];
	char md5[40];
}UpdateMessage_t;
static int checkNumStr(const char *nums){
        int i=0;
        int ret =0;
        for(i = 0; nums[i] != 0; i++){
                if(!isdigit(nums[i])){
                        ret =-1;
                        break;
                }
        }
        return ret;
}
void setVersionState(const char *data){
        char buf_s[128]={0};
        sprintf(buf_s,"nvram_set 2860 sversion %s",data);
        system(buf_s);
}
static void *runUpdateVersion(void *arg){
	UpdateMessage_t *upMsg = (UpdateMessage_t *)arg;
	char eepromVesion[8]={0};
	int systemVersionCode=0;
	char *runVersion=NULL;
	char *sversion= nvram_bufget(RT2860_NVRAM, "sversion");
	if(!strcmp(sversion,"")){
		readVersion_Eeprom(eepromVesion);
		if(!strcmp(eepromVesion,"")){
			systemVersionCode = 9;
		}else{
			setVersionState((const char *)eepromVesion);
			systemVersionCode = atoi(eepromVesion);
		}
	}else{
		systemVersionCode = atoi(sversion);
	}
	if(upMsg->versionCode!=systemVersionCode){		//start update system
		updateCurrentEventNums();	// interrupt current event
		SocSendMenu(BATTYPE,NULL);	
		sleep(1);
#if 1		
		int batVaule = Get_batteryVaule();
		if(batVaule<50){
			CreateSystemPlay_ProtectMusic((const char *)AMR_LOW_UPFAILED);
			sleep(3);
			unlockWeixin();
			goto exit0;		
		}
#endif		
		runVersion = (char *)calloc(1,1024);
		if(runVersion==NULL){
			unlockWeixin();
			goto exit0;			
		}
		snprintf(runVersion,1024,"versionServer http %s %s  %s %d %d %d &",upMsg->mac,upMsg->url,upMsg->md5,upMsg->id,upMsg->size,upMsg->versionCode);
		Unlock_EventQueue();
		//WriteVersionMessage((const char *)upMsg->url,(const char *)upMsg->md5,batVaule);
		CreateSystemPlay_ProtectMusic((const char *)AMR_55_NEW_VERSION);
		usleep(1000);
		Lock_EventQueue();
		disable_gpio();
		printf("%s : %s\n",__func__,runVersion);
		system(runVersion);
		sleep(2);
		free(runVersion);
	}else{				//current is news version Image
		runVersion = (char *)calloc(1,1024);
		if(runVersion==NULL){
			unlockWeixin();
			goto exit0;			
		}
		CreateSystemPlay_ProtectMusic((const char *)AMR_NEW_VERSION);
		snprintf(runVersion,1024,"versionServer new %s %s  %s %d %d %d &",upMsg->mac,upMsg->url,upMsg->md5,upMsg->id,upMsg->size,upMsg->versionCode);
		system(runVersion);
		unlockWeixin();
		sleep(2);
		free(runVersion);
	}

exit0:
	RunState=0;
	free(upMsg);
	return NULL;
}
void runSystemUpdateVersion(const char *mac,int id,const char *url,const char *md5,int size,int versionCode){
	if(RunState){
		//printf(" is run update version pthread");
		return ;
	}
	RunState=1;		
	UpdateMessage_t *upMsg = (UpdateMessage_t *)calloc(1,sizeof(UpdateMessage_t));
	if(upMsg==NULL){
		return;
	}
	snprintf(upMsg->mac,32,"%s",mac);
	upMsg->id = id;
	snprintf(upMsg->url,128,"%s",url);
	snprintf(upMsg->md5,40,"%s",md5);
	upMsg->size = size;
	upMsg->versionCode = versionCode;
	pthread_create_attr(runUpdateVersion,upMsg);
	usleep(1000);
}

//初始化微信消息队列
void InitWeixinMeesageList(void){
	WeixinEvent = initQueue();
	if(WeixinEvent==NULL){
		printf("init WeixinEvent list failed \n");
	}
	PushMsg = (WeixinPushMsg_t *)calloc(1,sizeof(WeixinPushMsg_t));
	if(PushMsg==NULL){
		perror("calloc pushmsg failed ");
	}
}
//清除微信消息队列
void CleanWeixinMeesageList(void){
	if(WeixinEvent){
		free(WeixinEvent);
		WeixinEvent=NULL;
	}
}
