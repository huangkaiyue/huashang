#include "comshead.h"
#include "base/queWorkCond.h"
#include "../studyvoices/qtts_qisc.h"

typedef struct {
	void *data;
	int size;
	int type;
}WeiXinMsg;

static WorkQueue *WeixinEvent=NULL;
static WeiXinMsg *Bak_Message=NULL;
#define WEIXIN_TEXT 	1
#define WEIXIN_VOICES	2

#define WEIXIN_PLAY_LIST_MAX	20


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
					remove(msg->data);//åˆ é™¤è¯­éŸ³æ–‡ä»¶
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
int AddWeiXinMessage_Text(const char *data,int Size){
	return __AddWeiXinMessage(data,Size,WEIXIN_TEXT);
}
int AddWeiXinMessage_Voices(const char *data,int Size){
	return __AddWeiXinMessage(data,Size,WEIXIN_VOICES);
}

int GetWeiXinMessageForPlay(void){
	WeiXinMsg *msg = NULL;
	char *Get=NULL;
	int msgSize=0;
	char bak_voices[256]={0};
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
				free(Bak_Message->data);	//ÊÍ·ÅÉÏÒ»´Î»º´æµÄÓïÒô
				free(Bak_Message);
			}
			if(msg->type ==WEIXIN_TEXT){
				Bak_Message = msg;
			}else if(msg->type ==WEIXIN_VOICES){
				snprintf(bak_voices,256,"cp %s %s",msg->data,"bak.amr");
				system(bak_voices);
				memset(msg->data,0,strlen(msg->data));
				sprintf(msg->data,"%s","bak.amr");
				Bak_Message = msg;
			}
		}else{
			free(msg->data);
			free(msg);
		}
	}else{
		if(Bak_Message==NULL){
			Create_PlayQttsEvent("å°æœ‹å‹æ²¡æœ‰æ¶ˆæ¯å–”ï¼",QTTS_UTF8);
		}else{
			if(Bak_Message->type ==WEIXIN_TEXT){
				Create_PlayQttsEvent(Bak_Message->data,QTTS_UTF8);
			}else if(Bak_Message->type ==WEIXIN_VOICES){
				CreatePlayWeixinVoicesSpeekEvent((const char *)Bak_Message->data);
			}
		}
	}
	return 0;
}

void InitWeixinMeesageList(void){
	WeixinEvent = initQueue();
	if(WeixinEvent==NULL){
		printf("init WeixinEvent list failed \n");
	}
}
void CleanWeixinMeesageList(void){
	if(WeixinEvent){
		free(WeixinEvent);
		WeixinEvent=NULL;
	}
}

