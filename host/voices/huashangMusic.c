#include "comshead.h"
#include "systools.h"
#include "config.h"
#include "host/voices/callvoices.h"
#include "../sdcard/musicList.h"
#include "base/cJSON.h"
#include "gpio_7620.h"
#include "huashangMusic.h"

#ifdef HUASHANG_JIAOYU		//获取华上教育sdcard当中国学的歌曲内容

//------------------------------------------------------------------------------
typedef struct{
	unsigned char playVoicesNameNums;
	int PlayHuashang_MusicIndex;	//播放华上教育歌曲下表编号 
	int Huashang_MusicTotal;
#ifdef XUN_FEI_OK
	unsigned char AifiState;
#endif
}HuashangUser_t;

static HuashangUser_t *hsUser=NULL;
//开机获取华上教育内容播放记录
void openSystemload_huashangData(void){
	char jsonfile[128]={0};
	hsUser = (HuashangUser_t *)calloc(1,sizeof(HuashangUser_t));
	if(hsUser==NULL){
		perror("calloc hsUser failed ");
		return ;
	}

	snprintf(jsonfile,128,"%s%s",TF_SYS_PATH,HUASHANG_JIAOYU_PLAY_JSON_FILE);
	char *filebuf = readFileBuf(jsonfile);
	if(filebuf==NULL){
		Write_huashang_log((const char *)"openSystemload_huashangData",(const char * )"read filefailed",1);
		return ;
	}
	cJSON * pJson = cJSON_Parse(filebuf);
	if(NULL == pJson){
		goto exit0;
	}
	cJSON * pSub = cJSON_GetObjectItem(pJson, "playindex");
	if(pSub==NULL){
		goto exit0;
	}
	hsUser->PlayHuashang_MusicIndex = pSub->valueint;
	pSub = cJSON_GetObjectItem(pJson, "total");
	if(pSub==NULL){
		hsUser->Huashang_MusicTotal = HUASHANG_MUSIC_TOTAL_NUM;
		goto exit0;
	}
	hsUser->Huashang_MusicTotal=pSub->valueint;
exit0:	
	free(filebuf);
}
int GetScard_forPlayHuashang_Music(unsigned char playMode,const void *playDir){
	int ret=-1;
	char playBuf[128]={0};
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(playMode==PLAY_NEXT){
		if(hsUser!=NULL){
			if(++hsUser->PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM){
				hsUser->PlayHuashang_MusicIndex=0;
			}
		}
		
	}else if(playMode==PLAY_PREV){
		if(hsUser!=NULL){
			if(--hsUser->PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM){
				hsUser->PlayHuashang_MusicIndex=HUASHANG_MUSIC_TOTAL_NUM;
			}
		}
	}	
	snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,hsUser->PlayHuashang_MusicIndex);
	if(access(playBuf, F_OK)==0){
		Write_huashang_log((const char *)"get play file ok",(const char * )playBuf,2);
		ret=__AddLocalMp3ForPaly((const char *)playBuf);
	}else{
		Write_huashang_log((const char *)"get play file failed",(const char * )playBuf,3);
	}	
	return ret;
}
//关机保存华上教育内容播放记录数据
void closeSystemSave_huashangData(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	if(hsUser!=NULL){
		cJSON_AddNumberToObject(pItem, "playindex", hsUser->PlayHuashang_MusicIndex);
		cJSON_AddNumberToObject(pItem, "total",hsUser->Huashang_MusicTotal); 
	}else{
		cJSON_AddNumberToObject(pItem, "playindex", 0);
		cJSON_AddNumberToObject(pItem, "total",HUASHANG_MUSIC_TOTAL_NUM); 
	}
	szJSON = cJSON_Print(pItem);
	cJSON_Delete(pItem);
	free(szJSON);
	if(hsUser!=NULL){
		free(hsUser);
		hsUser=NULL;
	}
}

/***
播放列表歌曲，按键控制单曲还是列表播放 
data:播放的数据
musicType:音乐类型  网络歌曲/本地歌曲

***/
void CreatePlayListMuisc(const void *data,int musicType){
	if(PLAY_MUSIC_NETWORK==musicType){
		__AddNetWork_UrlForPaly(data);
	}else if(PLAY_MUSIC_SDCARD==musicType){
		__AddLocalMp3ForPaly((const char *)data);
	}
}
//------------------------------------------------------------------------------

//华上教育按键按下播放按键
void Huashang_keyDown_playkeyVoices(int state){
	if(state==GPIO_UP){
	}else {

	}
}

void Huashang_changePlayVoicesName(void){
	if(hsUser!=NULL){
		if(++hsUser->playVoicesNameNums>1){
			hsUser->playVoicesNameNums=0;
		}
	}	
}
/**
获取播音人
**/
void GetPlayVoicesName(char *playVoicesName){
	if(hsUser->playVoicesNameNums==NULL){
		snprintf(playVoicesName,8,"tuling");
		return ;
	}
	switch(hsUser->playVoicesNameNums){
		case 0:
			snprintf(playVoicesName,8,"tuling");
			break;
		case 1:
			snprintf(playVoicesName,8,"vinn");
			break;
		case 2:
			snprintf(playVoicesName,8,"aisduck");
			break;
		case 3:
			snprintf(playVoicesName,8,"xiaoqi");
			break;
		default:
			snprintf(playVoicesName,8,"aisduck");
			break;
	}
}

#ifdef XUN_FEI_OK

//设置aifi 语音识别状态
void SetAifi_voicesState(unsigned char aifiState){
	AifiState=aifiState;
}
//获取设置aifi语音识别状态
int GetAifi_voicesState(void){
	return (int)AifiState;
}
/*
*检查图灵aifi 权限  DISABLE_TULING_PLAY :  讯飞离线识别成功，
	不需要播放图灵识别结果
*/
int check_tuingAifiPermison(void){
	int timeout=0;
	int ret=DISABLE_TULING_PLAY;
	while(1){
		switch(AifiState){
			case XUNFEI_AIFI_OK:	//离线识别成功
				goto exit0;
			case XUNFEI_AIFI_ING:	//讯飞正在识别状态
				usleep(1000);
				if(++timeout>3000){
					ret=TIMEOUT_AIFI;
					goto exit0;
				}
				break;
			case XUNFEI_AIFI_FAILED:
				ret=ALLOW_TULING_PLAY;
				goto exit0;
		}
	}
exit0:
	return ret;
}
//发送离线语音识别请求给 网络服务器，进行语音识别
void Huashang_SendnotOnline_xunfeiVoices(const char *filename){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "xunfei");
	cJSON_AddStringToObject(pItem, "musicname",filename); 
	szJSON = cJSON_Print(pItem);
	//设置成正在进行讯飞离线识别
	SetAifi_voicesState(XUNFEI_AIFI_ING);
	int ret= SendtoaliyunServices(szJSON,strlen(szJSON));
	Write_huashang_log((const char *)"Huashang_SendnotOnline_xunfeiVoices",(const char *)szJSON,12);
	cJSON_Delete(pItem);
	free(szJSON);
}
//获取到讯飞识别出来的aifi 语音结果
void GetHuashang_xunfei_aifiVoices(const char *musicname,int playIndex){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	unsigned char aifiState=0;
	char *data = (char *)calloc(1,strlen(musicname)+1);
	if(data==NULL){
		goto exit;
	}
	sprintf(data,"%s",musicname);
	if(AddworkEvent(data,0,XUNFEI_AIFI_EVENT)){
		printf("add xunfei aifi failed \n");
		goto exit;
	}
	//设置失败成功
	SetAifi_voicesState(XUNFEI_AIFI_OK);	
	return ;
exit:
	SetAifi_voicesState(XUNFEI_AIFI_FAILED);
}
//获取讯飞aifi 失败
void GetHuashang_xunfei_aifiFailed(void){
	SetAifi_voicesState(XUNFEI_AIFI_FAILED);
}
#endif
#endif

