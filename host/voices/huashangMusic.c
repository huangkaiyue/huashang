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
int GetScard_forPlayHuashang_Music(const void *playDir,unsigned char playMode,unsigned char EventSource){
	int ret=-1;
	char playBuf[128]={0};
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(playMode==PLAY_NEXT){
		if(hsUser!=NULL){
			if(++hsUser->PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM-1){
				hsUser->PlayHuashang_MusicIndex=0;
			}
		}
		
	}else if(playMode==PLAY_PREV){
		if(hsUser!=NULL){
			if(--hsUser->PlayHuashang_MusicIndex<=0){
				hsUser->PlayHuashang_MusicIndex=HUASHANG_MUSIC_TOTAL_NUM-1;
			}
		}
	}else if(playMode==PLAY_RANDOM){
	}	
	snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,hsUser->PlayHuashang_MusicIndex);
	if(access(playBuf, F_OK)==0){
		Write_huashang_log((const char *)"get play file ok",(const char * )playBuf,2);
		ret=__AddLocalMp3ForPaly((const char *)playBuf,EventSource);
	}else{
		Write_huashang_log((const char *)"get play file failed",(const char * )playBuf,3);
	}	
	return ret;
}
//关机保存华上教育内容播放记录数据
void closeSystemSave_huashangData(void){
	char jsonfile[128]={0};
	snprintf(jsonfile,128,"%s%s",TF_SYS_PATH,HUASHANG_JIAOYU_PLAY_JSON_FILE);

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
	FILE *fp =fopen(jsonfile,"w+");
	if(fp){
		fwrite((szJSON),strlen(szJSON),1,fp);
		fclose(fp);
	}
	free(szJSON);
	if(hsUser!=NULL){
		free(hsUser);
		hsUser=NULL;
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
		if(++hsUser->playVoicesNameNums>3){
			hsUser->playVoicesNameNums=0;
		}
	}	
}
/**
获取播音人
**/
void GetPlayVoicesName(char *playVoicesName,int *speek){
	if(hsUser->playVoicesNameNums==NULL){
		snprintf(playVoicesName,8,"tuling");
		*speek =50;
		return ;
	}
	switch(hsUser->playVoicesNameNums){
		case 0:
			snprintf(playVoicesName,8,"tuling");
			break;
		case 1:
			snprintf(playVoicesName,8,"vinn");
			*speek =70;
			break;
		case 2:
			snprintf(playVoicesName,8,"aisduck");
			*speek =50;
			break;
		case 3:
			snprintf(playVoicesName,8,"xiaoqi");
			*speek =50;
			break;
		default:
			snprintf(playVoicesName,8,"aisduck");
			*speek =50;
			break;
	}
}

#endif

