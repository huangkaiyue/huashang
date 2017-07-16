#include "comshead.h"
#include "systools.h"
#include "config.h"
#include "host/voices/callvoices.h"
#include "base/cJSON.h"
#include "gpio_7620.h"
#include "huashangMusic.h"

//------------------------------------------------------------------------------
typedef struct{
	unsigned char playVoicesNameNums;
	unsigned char dirMenu;
	int PlayHuashang_MusicIndex;	//播放华上教育歌曲下表编号 
	int Huashang_MusicTotal;
}HuashangUser_t;
static HuashangUser_t *hsUser=NULL;

void SetDirMenu(void){
	char playBuf[128]={0};
	char *readBuf = readFileBuf((const char * )DEFALUT_DIR_MENU_JSON);
	if(readBuf==NULL){
		return -1;
	}
	int ret=-1;
	int start=0, end=0;
	cJSON * pJson = cJSON_Parse(readBuf);
	if(NULL == pJson){
		printf("pasre json is failed \n");
		goto exit1;
	}
	cJSON * pArray =cJSON_GetObjectItem(pJson, "menu");
	if(NULL == pArray){
		printf("cJSON_Parse dirmenu failed \n");
		goto exit1;
	}
	int iCount = cJSON_GetArraySize(pArray);
	++hsUser->dirMenu;
	if(hsUser->dirMenu>iCount){
		hsUser->dirMenu=0;
	}
	cJSON* pItem = cJSON_GetArrayItem(pArray, hsUser->dirMenu);
	if (NULL == pItem){
		goto exit1;
	}
	cJSON *cj =cJSON_GetObjectItem(pItem, "start");
	if(cj!=NULL){
			hsUser->PlayHuashang_MusicIndex=cj->valueint;
			snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,hsUser->PlayHuashang_MusicIndex);
			Write_huashang_log((const char *)"SetDirMenu",(const char * )playBuf,2);
			if(access(playBuf, F_OK)==0){
			ret=__AddLocalMp3ForPaly((const char *)playBuf,EXTERN_PLAY_EVENT);
		}
	}
exit1:
	cJSON_Delete(pJson);
exit0:
	free(readBuf);
	return ret;
}
int Update_DirMenu(int PlayHuashang_MusicIndex){
	char *readBuf = readFileBuf((const char * )DEFALUT_DIR_MENU_JSON);
	if(readBuf==NULL){
		return -1;
	}
	int ret=-1;
	int start=0, end=0;
	cJSON * pJson = cJSON_Parse(readBuf);
	if(NULL == pJson){
		printf("pasre json is failed \n");
		goto exit1;
	}
	cJSON * pArray =cJSON_GetObjectItem(pJson, "menu");
	if(NULL == pArray){
		printf("cJSON_Parse aliyun failed \n");
		goto exit1;
	}
	int iCount = cJSON_GetArraySize(pArray);
	int i=0;
	for (i=0; i < iCount; ++i) {
		cJSON* pItem = cJSON_GetArrayItem(pArray, i);
		if (NULL == pItem){
			continue;
		}
		cJSON *cj =cJSON_GetObjectItem(pItem, "start");
		if(cj==NULL){
			continue; 
		}
		int start = cj->valueint;
		if(PlayHuashang_MusicIndex>=start){
			hsUser->dirMenu=i;
			break;
		}
	}
exit1:
	cJSON_Delete(pJson);
exit0:
	free(readBuf);
	return ret;

}
void InitHuashang(void){
	hsUser = (HuashangUser_t *)calloc(1,sizeof(HuashangUser_t));
	if(hsUser==NULL){
		perror("calloc hsUser failed ");
		return ;
	}
	hsUser->dirMenu=1;
}
//开机获取华上教育内容播放记录
void openSystemload_huashangData(void){
	char jsonfile[128]={0};
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
//获取sdcard 歌曲编号进行播放
int GetScard_forPlayHuashang_Music(unsigned char playMode,unsigned char EventSource){
	int ret=-1;
	char playBuf[128]={0};
	if(hsUser==NULL){
		return ret;
	}
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(playMode==PLAY_NEXT){
		if(++hsUser->PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM){
			hsUser->PlayHuashang_MusicIndex=0;
		}
	}else if(playMode==PLAY_PREV){
		if(--hsUser->PlayHuashang_MusicIndex<=0){
			hsUser->PlayHuashang_MusicIndex=HUASHANG_MUSIC_TOTAL_NUM;
		}
	}else if(playMode==PLAY_RANDOM){
	}	
	snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,hsUser->PlayHuashang_MusicIndex);
	if(access(playBuf, F_OK)==0){
		Write_huashang_log((const char *)"get play file ok",(const char * )playBuf,2);
		Update_DirMenu(hsUser->PlayHuashang_MusicIndex);
		ret=__AddLocalMp3ForPaly((const char *)playBuf,EventSource);
	}else{
		Write_huashang_log((const char *)"get play file failed",(const char * )playBuf,3);
	}	
	return ret;
}
int huashang_CreatePlayDefaultMusic_forPlay(char *getBuf,const char* musicType){
	char *readBuf = readFileBuf((const char * )DEFALUT_HUASHANG_JSON);
	if(readBuf==NULL){
		return -1;
	}
	int ret=-1;
	int min=0, max=0;
	cJSON * pJson = cJSON_Parse(readBuf);
	if(NULL == pJson){
		printf("pasre json is failed \n");
		goto exit0;
	}
	cJSON * pArray =cJSON_GetObjectItem(pJson, musicType);
	if(NULL == pArray){
		printf("cJSON_Parse DEFALUT_HUASHANG_JSON failed \n");
		goto exit1;
	}
	cJSON* pItem = cJSON_GetArrayItem(pArray, 0);
	if(pItem==NULL){
		goto exit1;
	}
	cJSON *cj = cJSON_GetObjectItem(pItem, "max");
	if(cj!=NULL){
		printf("cj->valueint= %d\n",cj->valueint);
		max = cj->valueint;
	}
	cj = cJSON_GetObjectItem(pItem, "min");
	if(cj!=NULL){
		printf("cj->valueint= %d\n",cj->valueint);
		min = cj->valueint;
	}
	int randMax=0;
	if(max<=min)
		randMax=1;
	else
		randMax=(max-min-1);
	time_t ti = time(NULL);
	int randNums = ((int)ti)%randMax;
	min +=randNums;
	snprintf(getBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,min);
	if(access(getBuf, F_OK)){
		ret=-1;
		goto exit1;
	}
	ret=0;
	WritePlayUrl_Log("getBuf",getBuf);
exit1:
	cJSON_Delete(pJson);
exit0:
	free(readBuf);
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

void updatePlayindex(int playIndex){
	if(hsUser){
		hsUser->PlayHuashang_MusicIndex=playIndex;
		Update_DirMenu(hsUser->PlayHuashang_MusicIndex);
	}
}
//------------------------------------------------------------------------------

void Huashang_changePlayVoicesName(void){
	if(hsUser!=NULL){
		if(++hsUser->playVoicesNameNums>3){
			hsUser->playVoicesNameNums=0;
		}
	}	
}

int WeiXin_playhuaShangMusic(int playIndex){
	int ret=-1;
	char playBuf[128]={0};
	if(access(TF_SYS_PATH, F_OK)){	//检查tf卡
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(playIndex>HUASHANG_MUSIC_TOTAL_NUM||playIndex<0){
		hsUser->PlayHuashang_MusicIndex=0;
	}else{
		hsUser->PlayHuashang_MusicIndex =playIndex;
	}
	snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,hsUser->PlayHuashang_MusicIndex);
	if(access(playBuf, F_OK)==0){	
		ret =__AddLocalMp3ForPaly((const char *)playBuf,EXTERN_PLAY_EVENT);
	}
	return ret;
}
/**
获取播音人
**/
void GetPlayVoicesName(char *playVoicesName,int *speek){
	if(hsUser==NULL){
		snprintf(playVoicesName,8,"tuling");
		*speek =50;
		return ;
	}
	hsUser->playVoicesNameNums=1;
	switch(hsUser->playVoicesNameNums){
		case 0:
			snprintf(playVoicesName,8,"tuling");
			break;
		case 1:
			snprintf(playVoicesName,8,"vinn");
			*speek =50;
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


