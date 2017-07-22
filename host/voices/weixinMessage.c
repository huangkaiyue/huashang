#include "comshead.h"
#include "base/queWorkCond.h"
#include "../studyvoices/qtts_qisc.h"

typedef struct {
	void *data;	//添加到队列的消息数据(音频or文字)
	int size;	//添加到对接的数据大小(音频文件 对应--->文件长度  )
	int type;	//添加到队列当中消息类型
}WeiXinMsg;

static WorkQueue *WeixinEvent=NULL;
static WeiXinMsg *Bak_Message=NULL;
#define WEIXIN_TEXT 	1
#define WEIXIN_VOICES	2

#define WEIXIN_PLAY_LIST_MAX	20	//允许当前存最大的队列数据

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
//获取微信消息队列进行播放
int GetWeiXinMessageForPlay(void){
	WeiXinMsg *msg = NULL;
	char *Get=NULL;
	int msgSize=0;
	time_t t;	
	char bak_voices[256]={0};
	static int ti=1;
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
			Create_PlayQttsEvent("小朋友没有消息喔！",QTTS_UTF8);
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
}
//初始化微信消息队列
void InitWeixinMeesageList(void){
	WeixinEvent = initQueue();
	if(WeixinEvent==NULL){
		printf("init WeixinEvent list failed \n");
	}
}
//清除微信消息队列
void CleanWeixinMeesageList(void){
	if(WeixinEvent){
		free(WeixinEvent);
		WeixinEvent=NULL;
	}
}
