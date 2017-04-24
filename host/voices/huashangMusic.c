#include "comshead.h"
#include "systools.h"
#include "config.h"
#include "host/voices/callvoices.h"
#include "../sdcard/musicList.h"
#include "base/cJSON.h"
#include "gpio_7620.h"
#include "huashangMusic.h"

#ifdef HUASHANG_JIAOYU		//��ȡ���Ͻ���sdcard���й�ѧ�ĸ�������

//------------------------------------------------------------------------------

static int PlayHuashang_MusicIndex=0;	//���Ż��Ͻ��������±���� 
static int Huashang_MusicTotal=HUASHANG_MUSIC_TOTAL_NUM;		
static unsigned char AifiState=0;

//������ȡ���Ͻ������ݲ��ż�¼
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
	PlayHuashang_MusicIndex = pSub->valueint;
	pSub = cJSON_GetObjectItem(pJson, "total");
	if(pSub==NULL){
		goto exit0;
	}
	Huashang_MusicTotal=pSub->valueint;
exit0:	
	free(filebuf);
}
int GetScard_forPlayHuashang_Music(unsigned char playMode,const void *playDir){
	int ret=-1;
	char playBuf[128]={0};
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(playMode==PLAY_NEXT){
		if(++PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM){
			PlayHuashang_MusicIndex=0;
		}
		
	}else if(playMode==PLAY_PREV){
		if(--PlayHuashang_MusicIndex>HUASHANG_MUSIC_TOTAL_NUM){
			PlayHuashang_MusicIndex=HUASHANG_MUSIC_TOTAL_NUM;
		}
	}	
	snprintf(playBuf,128,"%s%s/%d.mp3",TF_SYS_PATH,HUASHANG_GUOXUE_DIR,PlayHuashang_MusicIndex);
	if(access(playBuf, F_OK)==0){
		Write_huashang_log((const char *)"get play file ok",(const char * )playBuf,2);
		ret=__AddLocalMp3ForPaly((const char *)playBuf);
	}else{
		Write_huashang_log((const char *)"get play file failed",(const char * )playBuf,3);
	}	
	return ret;
}
//�ػ����滪�Ͻ������ݲ��ż�¼����
void closeSystemSave_huashangData(void){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddNumberToObject(pItem, "playindex", PlayHuashang_MusicIndex);
	cJSON_AddNumberToObject(pItem, "total",Huashang_MusicTotal); 
	szJSON = cJSON_Print(pItem);
	cJSON_Delete(pItem);
	free(szJSON);
}

/***
�����б��������������Ƶ��������б����� 
data:���ŵ�����
musicType:��������  �������/���ظ���

***/
void CreatePlayListMuisc(const void *data,int musicType){
	if(PLAY_MUSIC_NETWORK==musicType){
		__AddNetWork_UrlForPaly(data);
	}else if(PLAY_MUSIC_SDCARD==musicType){
		__AddLocalMp3ForPaly((const char *)data);
	}
}
//------------------------------------------------------------------------------

//���Ͻ����������²��Ű���
void Huashang_keyDown_playkeyVoices(int state){
	if(state==GPIO_UP){
	}else {

	}
}


//����aifi ����ʶ��״̬
void SetAifi_voicesState(unsigned char aifiState){
	AifiState=aifiState;
}
//��ȡ����aifi����ʶ��״̬
int GetAifi_voicesState(void){
	return (int)AifiState;
}
/*
*���ͼ��aifi Ȩ��  DISABLE_TULING_PLAY :  Ѷ������ʶ��ɹ���
	����Ҫ����ͼ��ʶ����
*/
int check_tuingAifiPermison(void){
	int timeout=0;
	int ret=DISABLE_TULING_PLAY;
	while(1){
		switch(AifiState){
			case XUNFEI_AIFI_OK:	//����ʶ��ɹ�
				goto exit0;
			case XUNFEI_AIFI_ING:	//Ѷ������ʶ��״̬
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
//������������ʶ������� �������������������ʶ��
void Huashang_SendnotOnline_xunfeiVoices(const char *filename){
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "xunfei");
	cJSON_AddStringToObject(pItem, "musicname",filename); 
	szJSON = cJSON_Print(pItem);
	//���ó����ڽ���Ѷ������ʶ��
	SetAifi_voicesState(XUNFEI_AIFI_ING);
	int ret= SendtoaliyunServices(szJSON,strlen(szJSON));
	Write_huashang_log((const char *)"Huashang_SendnotOnline_xunfeiVoices",(const char *)szJSON,12);
	cJSON_Delete(pItem);
	free(szJSON);
}
//��ȡ��Ѷ��ʶ�������aifi �������
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
	//����ʧ�ܳɹ�
	SetAifi_voicesState(XUNFEI_AIFI_OK);	
	return ;
exit:
	SetAifi_voicesState(XUNFEI_AIFI_FAILED);
}
//��ȡѶ��aifi ʧ��
void GetHuashang_xunfei_aifiFailed(void){
	SetAifi_voicesState(XUNFEI_AIFI_FAILED);
}
#endif
