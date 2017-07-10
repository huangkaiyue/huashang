#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/pool.h"
#include "base/cJSON.h"
#include "host/voices/callvoices.h"
#include "host/ap_sta.h"
#include "nvram.h"
#include "../../net/network.h"
#include "host/voices/WavAmrCon.h"
#include "systools.h"
#include "gpio_7620.h"

#include "../studyvoices/qtts_qisc.h"
#include "../sdcard/musicList.h"
#include "uart/uart.h"
#include "config.h"
#include "log.h"

SysMessage sysMes;
//------------------------config network and set system network state---------------------------------------------------------
static void CloseWifi(void){
	if(sysMes.wifiState==0){
		sysMes.wifiState=1;
		system("ifconfig ra0 down &");
	}
}
void OpenWifi(void){
	if(sysMes.wifiState==1){
		sysMes.wifiState=0;
		system("ifconfig ra0 up &");
	}
}
/*
@ ��������״̬
@ 
@
*/
static void setNetWorkLive(unsigned char state){
	sysMes.netstate=state;
}
static int getNetWorkLive(void){
	return sysMes.netstate;
}
/*
@ �������״̬ 
@ enablePlayVoices :���ڿ��Ʋ���wifi ����״̬����
@ 0: ������������  -1: ��������ʧ��
*/
int checkNetWorkLive(unsigned char enablePlayVoices){
	if(getNetWorkLive()==NETWORK_ER||getNetWorkLive()==NETWORK_UNKOWN){
		//����̨��
		if(getEventNum()>0){	//����Ƿ���ӹ��¼�
			return -1;
		}
		//���ϵͳ����ȥ���ţ���ʾ�û���������
		if(enablePlayVoices==ENABLE_CHECK_VOICES_PLAY)
			Create_PlaySystemEventVoices(NETWORK_ERROT_PLAY);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
	return -1;
}
/*******************************************************
��������: �鿴NetManger�����Ƿ����pid��
����: pid_name	��������
����ֵ: ��
********************************************************/
static int Compare_PIDName(char *pid_name){
	if(!strcmp(pid_name,"NetManger")){
		return 0;
	}
	return -1;
}
//��������NetManger �������
static void Restart_RunNetManger(void){
	system("NetManger -t 2 -wifi on &");
	sleep(1);
}
//����̨��������(NetManger)����״̬
void CheckNetManger_PidRunState(void){
	if(judge_pid_exist(Compare_PIDName)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);	
		Restart_RunNetManger();
	}
}
//�̰������»�ȡwifi ���ֲ�����
void ShortKeyDown_ForPlayWifiMessage(void){
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
		if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
			return;
		}
		char wifiMessage[128]={0};
		char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
		if(strlen(wifi)<=0){
			printf("read wifi failed \n");
			return ;
		}
#ifdef DEBUG_PLAY_SYSTEM_IP
		char IP[20]={0};
		GetNetworkcardIp((char * )"apcli0",IP);
		snprintf(wifiMessage,128,"������ wifi %s  IP��ַ�� %s",wifi,IP);
#else
		snprintf(wifiMessage,128,"������ wifi %s ",wifi);
#endif	
		Create_PlayQttsEvent(wifiMessage,QTTS_GBK);
	}
}

/*******************************************************
��������: ���������������¼�
����: ��
����ֵ: ��
********************************************************/
void LongNetKeyDown_ForConfigWifi(void){
	CheckNetManger_PidRunState();
	WiterSmartConifg_Log("Network key down","ok");
	if(!checkInternetFile()){
		WiterSmartConifg_Log("startSmartConfig checkInternetFile","failed");
		return;
	}
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){//���ڲ����¼����У�����������
		disable_gpio();
		startSmartConfig(Create_PlaySystemEventVoices,enable_gpio);	
	}else{		
		WiterSmartConifg_Log("startSmartConfig  failed ","is not RECODE_PAUSE");
	}
}
//���ӳɹ����ù���ָʾ��,����mucʱ��
static void Link_NetworkOk(void){
	Led_vigue_close();
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	led_lr_oc(openled);
#elif defined(DATOU_JIANG)
	led_lr_oc(closeled);
#endif
	SocSendMenu(3,0);			//���ͱ���ʱ���mcu
	usleep(100*1000);
	setNetWorkLive(NETWORK_OK);		//��������״̬
}
//����ʧ��,��˸ָʾ��
static void Link_NetworkError(void){
	pool_add_task(Led_vigue_open,NULL);
#if defined(TANGTANG_LUO)||defined(QITUTU_SHI)||defined(HUASHANG_JIAOYU)
	led_lr_oc(closeled);

#elif defined(DATOU_JIANG)
	led_lr_oc(openled);
#endif
	setNetWorkLive(NETWORK_ER);
}

//--------------end config network and set system network state---------------------------------------------------------
/*******************************************************
��������: �������URL��ַ�����е��н��в���
����: data ���Ÿ�����Ϣ���Ѿ�malloc�����ڴ���	
����ֵ: ��
********************************************************/
int __AddNetWork_UrlForPaly(Player_t *player){
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
		goto exit0;
	}
	//��ֹ��ӹ���
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d getplayEventNum() =%d\n",getEventNum(),getplayEventNum());
		WritePlayUrl_Log("__AddNetWork_UrlForPaly"," getEventNum ");
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){	
		WritePlayUrl_Log("__AddNetWork_UrlForPaly"," GetRecordeVoices_PthreadState failed ");
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handEvent){
		WritePlayUrl_Log("url"," add ok");
		handEvent->data = (char *)player;
		handEvent->event=URL_VOICES_EVENT;
		handEvent->EventNums=updateCurrentEventNums();
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}else{
		goto exit0;
	}
	return ret;
exit0:
	free(player);
	return -1;
}
/*******************************************************
��������: ��ӱ������ֵ����н��в���
����: localpath ����MP3����·��
EventSource:�¼���Դ (�ⲿ�¼�����/������)
����ֵ: 0��ӳɹ� -1���ʧ��
********************************************************/
int __AddLocalMp3ForPaly(const char *localpath,unsigned char EventSource){
	if(getEventNum()>0){	//�¼�������ֱ࣬�Ӷ�������ֹ��ӹ��죬���º�������ʱ�����
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == START_SPEEK_VOICES||GetRecordeVoices_PthreadState() == START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() == END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){	
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));	
	if(handEvent){
		Player_t *player= (char *)calloc(1,sizeof(Player_t));
		if(player==NULL){
			perror("calloc error !!!");
			goto exit0;
		}
		player->playListState=EventSource;
		sprintf(player->playfilename,"%s",localpath);	
		handEvent->EventNums=updateCurrentEventNums();
		handEvent->data = (char *)player;
		handEvent->event=LOCAL_MP3_EVENT;	
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}
	return ret;	
exit0:
	free(handEvent);
	return -1;
}
/*
@ ����Ŀ¼�˵���·����ȡ����sdcatd �������ֽ��в���
@ MuiscMenu Ŀ¼�˵�(Ӣ��/�Ƽ�/��ѧ/�ղص����ݷ��� ) 
	MusicDir sdcardĿ¼·�� playMode����ģʽ ��һ��/��һ��
@ 0��ӳɹ� -1���ʧ��
*/
int GetSdcardMusicNameforPlay(unsigned char MuiscMenu,const char *MusicDir, unsigned char playMode){	
	int ret=0;
#if defined(DATOU_JIANG)||defined(QITUTU_SHI)
	char filePath[128]={0};	//����·��������
	char musicName[64]={0};//����������
	
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	int len =snprintf(filePath,128,"%s%s",TF_SYS_PATH,MusicDir);
	if(MuiscMenu==xiai){		//��ȡϲ��Ŀ¼�µĸ���·����
		if(PlayxiaiMusic((const char *)TF_SYS_PATH,MusicDir,musicName, playMode) == -2){
			Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return -1;
		}
		if(!strcmp(musicName,"")){//��ȡ��·����Ϊ�գ�ֱ���˳�
			//Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return -1;
		}
	}else{				//��ȡ�ͻ��Զ����Ÿ���·����
		if(GetSdcardMusic((const char *)TF_SYS_PATH,MusicDir,musicName, playMode)){
			Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
			return ret;
		}
		if(!strcmp(musicName,"")){//��ȡ��·����Ϊ�գ�ֱ���˳�
			return -1;
		}
	}
	snprintf(filePath+len,128-len,"%s",musicName);	//��������·��
	ret=__AddLocalMp3ForPaly((const char *)filePath,EXTERN_PLAY_EVENT);//��Ӹ��������в���
	if(ret==0)
		sysMes.localplayname=MuiscMenu;
#endif	
	return ret;
}
/**
�����²�����ͣ��
**/
void KeydownEventPlayPause(void){
#ifndef DATOU_JIANG
	keydown_flashingLED();	
#endif
	if(GetRecordeVoices_PthreadState()==PLAY_MP3_MUSIC){
		keyStreamPlay();
	}
}
/*******************************************************
��������: ����URL�¼�
����: ��
����ֵ: ��
********************************************************/
void Create_CleanUrlEvent(void){
	sysMes.localplayname=0;
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->event=SET_RATE_EVENT;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
/*******************************************************
��������: ��������QTTS�����¼� --->�ı�ת����������
����: txt QTTS�ı� type :0---GBK ��ʽ 1----UTF8 ��ʽ
����ֵ: ��
********************************************************/
void Create_PlayQttsEvent(const char *txt,int type){
	unsigned char mixMode =NORMAL_PLAY_PCM;
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
		return;
	}	
	if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES){
		return;
	}else if (GetRecordeVoices_PthreadState() == PLAY_MP3_MUSIC||GetRecordeVoices_PthreadState() ==SOUND_MIX_PLAY){	//��ǰ���Ÿ���
		mixMode =MIX_PLAY_PCM;
		//Create_CleanUrlEvent();
		//return;
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->EventNums= updateCurrentEventNums();
		handtext->mixMode=mixMode;
		handtext->data= (char *)calloc(1,strlen(txt)+4);
		if (handtext->data){
			sprintf(handtext->data,"%s%s",txt,",");	//�ı�β�����",",��֤�ı���������
			handtext->event =QTTS_PLAY_EVENT;
			handtext->playLocalVoicesIndex=type;
			AddworkEvent((const char *)handtext,sizeof(HandlerText_t));
		}
	}
}
void Handler_PlayQttsEvent(HandlerText_t *handText){
	char xunPlayname[24]={0};
	int playSpeed=50;	//Ĭ�ϲ����ٶ�Ϊ5
#if defined(HUASHANG_JIAOYU)
	GetPlayVoicesName(xunPlayname,&playSpeed);	
	if(!strncmp(xunPlayname,"tuling",6)){
		memset(xunPlayname,0,24);
		snprintf(xunPlayname,24,"%s","vinn");
	}
#else
	snprintf(xunPlayname,24,"%s","vinn");
#endif
	if(handText->mixMode==NORMAL_PLAY_PCM){
		start_event_play_wav();
		if(!PlayQttsText(handText->data,handText->playLocalVoicesIndex,(const char *)xunPlayname,handText->EventNums,playSpeed)){
			pause_record_audio();
		}
	}else{
		char outFile[]="qtts.pcm";//��������
		if(!QttsTextVoicesFile(handText->data,handText->playLocalVoicesIndex,(const char *)xunPlayname,handText->EventNums,playSpeed,outFile)){
			__playResamplePlayPcmFile((const char *)outFile,handText->EventNums);
			remove(outFile);
		}
	}
	free((void *)handText->data);
	free((void *)handText);
}
/*******************************************************
��������: �Ự���������ź�,����¼�� 
����: ��
����ֵ: ��
********************************************************/
void TulingKeyDownSingal(void){
	updateCurrentEventNums();	
	Write_Speekkeylog((const char *)"TulingKeyDownSingal",0);
	//����΢�ŶԽ�״̬��ֱ���˳�	
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}	
	else if (GetRecordeVoices_PthreadState() == PLAY_MP3_MUSIC){//���ڲ��Ÿ���״̬	
		Create_CleanUrlEvent();
		Write_Speekkeylog((const char *)"PLAY_MP3_MUSIC",GetRecordeVoices_PthreadState());
		Create_PlaySystemEventVoices(CONTINUE_PLAY_MUSIC_VOICES);
	}else{		
		if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������,û������ֱ���˳�����
			return;
		}
		NetStreamExitFile();	//��΢�Ŷ����͸�����û�н��в�����һ�׸�����ʱ��ͻȻ�������ܻỰ��������Ҫ�л������ʣ����ܽ������ܻỰ״̬
		if(SetWm8960Rate(RECODE_RATE,(const char *)"TulingKeyDownSingal set rate")){	//�л�������ʧ�ܣ��˳�(��ֹ���̵߳����л�����Դ��ͻ����)
			return ;
		}
		Show_KeyDownPicture();
		StartTuling_RecordeVoices();
		Write_Speekkeylog((const char *)"StartTuling_RecordeVoices",GetRecordeVoices_PthreadState());
	}
}
//�ػ������ļ���������
void *Close_Mtk76xxSystem(void *arg){
	char token[64]={0};
	GetTokenValue(token);
	Save_TulingToken_toRouteTable((const char *)token);
	SaveVol_toRouteTable(GetVol());		//����������·�ɱ�
#ifdef CLOCKTOALIYUN
	CloseSystemSignToaliyun();			//���͹ػ��źŸ�����
#endif

#ifdef PALY_URL_SD	
	SaveSystemPlayNum();
#endif	
#ifdef HUASHANG_JIAOYU
	closeSystemSave_huashangData();
#endif
	system("sync");	//ͬ�����ݵ�sdcard ����
	return NULL;
}
//----------------------����ϵͳ�����йصġ��¼��Ĳ��������Ѵ���-----------------------------------------------------
//�������ܻỰ�����󴥷�����������
static void TaiBenToTulingNOVoices(unsigned int playEventNums){
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));	
	switch(i){
		case 1:
			PlaySystemAmrVoices(NO_VOICES,playEventNums);
			break;
		case 2:
			PlaySystemAmrVoices(NO_VOICES_1,playEventNums);
			break;
		case 3:
			PlaySystemAmrVoices(NO_VOICES_2,playEventNums);
			break;
		case 4:
			PlaySystemAmrVoices(NO_VOICES_3,playEventNums);
			break;
		case 5:
			PlaySystemAmrVoices(NO_VOICES_4,playEventNums);
			break;
		case 6:
			PlaySystemAmrVoices(NO_VOICES_5,playEventNums);
			break;
		case 7:
			PlaySystemAmrVoices(NO_VOICES_6,playEventNums);
			break;
		case 8:
			PlaySystemAmrVoices(NO_VOICES_7,playEventNums);
			break;
		case 9:
			PlaySystemAmrVoices(NO_VOICES_8,playEventNums);
			break;
		case 10:
			PlaySystemAmrVoices(NO_VOICES_9,playEventNums);
			break;
	}
}
#define TLERNUM 34.0
static void TaiwanToTulingError(unsigned int playEventNums){
	char buf[32]={0};
	int i=(1+(int)(TLERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingError%d_8k.amr",i);
	PlaySystemAmrVoices(buf,playEventNums);
}
/*
@ û�������ʱ�򣬲��ű���ϵͳ�̶�¼��̨��
@
*/
static void Handle_PlayTaiBenToNONetWork(unsigned int playEventNums){
	char file[64]={0};
	int i=(1+(int)(5.0*rand()/(RAND_MAX+1.0)));
	snprintf(file,64,"qtts/network_error_8K_%d.amr",i);
	PlaySystemAmrVoices(file,playEventNums);
}
static void CreateCloseSystemLockFile(void){
	FILE *fp = fopen(CLOSE_SYSTEM_LOCK_FILE,"w+");
	if(fp){
		fclose(fp);
	}
}
/*******************************************************
��������: ����һ������ϵͳ�����¼���
����: sys_voices ϵͳ�����	
����ֵ: ��
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
		Create_CleanUrlEvent();
		usleep(3000);
	}else if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY){
		return;
	}
	char addsys_voices[128]={0};
	sprintf(addsys_voices,"Create_PlaySystemEventVoices%d",sys_voices);
	printf("addsys_voices =%s\n",addsys_voices);
	Write_huashangTextLog(addsys_voices);
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->EventNums =updateCurrentEventNums();
		handtext->playLocalVoicesIndex =sys_voices;
		handtext->event =SYS_VOICES_EVENT;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}	
}
//��Ӳ��Ź��������ػ�������Ҫ����
void Create_PlayImportVoices(int sys_voices){
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->event =SYS_VOICES_EVENT;
		handtext->EventNums=GetCurrentEventNums();
		handtext->playLocalVoicesIndex =sys_voices;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
//�����¼��ص�����
void UartEventcallFuntion(int event){
	updateCurrentEventNums();
	if(event==UART_EVENT_CLOSE_SYSTEM){	//�������˳��¼�
		disable_gpio();					//�ر�gpio�жϹ���,��ֹ�ػ������ٲ��������¼�
		cleanQuequeEvent();				//�����¼����У���֤�ܹ���������
		if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
			printf("Create Clean url event \n");
			Create_CleanUrlEvent();
		}
		showFacePicture(CLOSE_SYSTEM_PICTURE);	
		CreateCloseSystemLockFile();
		pool_add_task(Close_Mtk76xxSystem,NULL);//�ػ�����ͱ����̨����
		Create_PlayImportVoices(END_SYS_VOICES_PLAY);//����һ������ϵͳ������
	}else if(event==UART_EVENT_LOW_BASTERRY){
		Create_PlaySystemEventVoices(LOW_BATTERY_PLAY);
	}	
} 
//--------------------------------------------------------------------------------------------------------
//��ȡ����Ĭ��json ����url��ַ
static int GetDefaultMusicJson_forPlay(char *getUrlBuf,const char *musicType){
	int ret =-1;
	char *readBuf = readFileBuf((const char * )DEFALUT_URL_JSON);
	if(readBuf==NULL){
		return -1;
	}
	cJSON * pJson = cJSON_Parse(readBuf);
	if(NULL == pJson){
		goto exit0;
	}
	cJSON * pArray =cJSON_GetObjectItem(pJson, musicType);
	if(NULL == pArray){
		printf("cJSON_Parse DEFALUT_URL_JSON failed \n");
		goto exit1;
	}
	int iCount = cJSON_GetArraySize(pArray);
	//printf("name	 iCount == %d \n",iCount);
	int randMax=(float)iCount-1;
	time_t ti = time(NULL);
	int randNums = ((int)ti)%randMax;
#if 0
	int i=0;
	for (i=0; i < iCount; ++i) {
		cJSON* pItem = cJSON_GetArrayItem(pArray, i);
		if (NULL == pItem){
			continue;
		}
		char *name = cJSON_GetObjectItem(pItem,"name")->valuestring;
		printf("name[%d] = %s\n",i,name);
	}
#endif
	cJSON* pItem = cJSON_GetArrayItem(pArray, randNums);
	if(pItem){
		cJSON *cjson = cJSON_GetObjectItem(pItem,"name");
		if(cjson){
			snprintf(getUrlBuf,128,"%s",cjson->valuestring);
			ret =0;
		}
	}
exit1:
	cJSON_Delete(pJson);
exit0:
	free(readBuf);
	return ret;
}
//��������Ĭ��url����
void CreatePlayDefaultMusic_forPlay(const char* musicType){
	Player_t *player = (Player_t *)calloc(1,sizeof(Player_t));
	if(player==NULL){
		perror("calloc error !!!");
		return;
	}	
	player->musicTime=100;
	player->playListState=AUTO_PLAY_EVENT;
	snprintf(player->musicname,64,"%s",musicType);//musicname ��ʱ�����������ṹ���������Ų�������
#if defined(HUASHANG_JIAOYU)		
	if(huashang_CreatePlayDefaultMusic_forPlay(player->playfilename,musicType)){
		return ;
	}
#else
	if(GetDefaultMusicJson_forPlay(player->playfilename,musicType)){
		free(player);
		return ;
	}
#endif
	__AddNetWork_UrlForPaly(player);
}
//�ͻ���̨�������͵�����
void Custom_Interface_RunPlayVoices(unsigned int playEventNums){
	int ret =-1;
	char musictype[12]={0};
	if(checkNetWorkLive(DISABLE_CHECK_VOICES_PLAY)){
		return;
	}
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
	showFacePicture(WAIT_CTRL_NUM3);
	if(p->tm_hour+8>=21){
		ret =PlaySystemAmrVoices(TIMEOUT_sleep,playEventNums);
		snprintf(musictype,12,"%s","sleep");	//������������
		goto exit1;
	}
	if(PlaySystemAmrVoices(SPEEK_WARNING,playEventNums)){
		goto exit0;	//�쳣����˳�
	}
	int randNums=(1+(int)(3.0*rand()/(RAND_MAX+1.0)));
	start_event_play_wav();
	switch(randNums){
		case 1:
			ret =PlaySystemAmrVoices(TIMEOUT_music,playEventNums);
			snprintf(musictype,12,"%s","music");	//������������
			break;
		case 2:
			ret =PlaySystemAmrVoices(TIMEOUT_guoxue,playEventNums);
			snprintf(musictype,12,"%s","guoxue");	//���Ź�ѧ����
			break;
		case 3:
			ret =PlaySystemAmrVoices(TIMEOUT_chengyu,playEventNums);
			snprintf(musictype,12,"%s","chengyu");	//���ų������
			break;
		case 4:
			ret =PlaySystemAmrVoices(TIMEOUT_baike,playEventNums);
			snprintf(musictype,12,"%s","baike");	//���Űٿ�֪ʶ
			break;
		default:
			ret =PlaySystemAmrVoices(TIMEOUT_music,playEventNums);
			break;
	}
exit1:	
	if(!ret){
		CreatePlayDefaultMusic_forPlay(musictype);	//musictype 
	}
exit0:
	return;
}

/***
�����б�������������Ƶ��������б��� 
data:���ŵ�����
musicType:��������  �������/���ظ���

***/
void CreatePlayListMuisc(const void *data,int musicType){
	Player_t * player =NULL;
	if(PLAY_MUSIC_NETWORK==musicType){
		__AddNetWork_UrlForPaly((Player_t *)data);
	}else if(PLAY_MUSIC_SDCARD==musicType){
		player= (Player_t *)data;
		__AddLocalMp3ForPaly((const char *)player->playfilename,player->playListState);
	}
}

//--------------------------------------------------------------------------------------------------------

static void *updateHuashangFacePthread(void *arg){
	int eventNums = *(int *)arg;
	int i=0;
	sleep(1);
	for(i=0;i<3;i++){
		if(eventNums!=GetCurrentEventNums()){
			break;
		}
		sleep(1);
	}
	if(eventNums==GetCurrentEventNums()){
		showFacePicture(PLAY_MUSIC_NUM4);	
	}
}
/*******************************************************
��������: ϵͳ���¼�������
����: sys_voices ϵͳ�����
����ֵ: ��
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices,unsigned int playEventNums){
	int vol=0;
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//������
#ifndef HUASHANG_JIAOYU		
			PlaySystemAmrVoices(END_SYS_VOICES,playEventNums);
#endif
			Led_vigue_close();
			Led_System_vigue_close();
#if defined(TANGTANG_LUO) || defined(QITUTU_SHI) || defined(HUASHANG_JIAOYU)
			close_sys_led();
#endif
#ifdef DATOU_JIANG
			open_sys_led();
#endif
			led_lr_oc(closeled);
			usleep(800*1000);
			Mute_voices(MUTE);						//�رչ���
			enable_gpio();
			break;
		case LOW_BATTERY_PLAY:						//�͵�ػ�
#ifdef PALY_URL_SD
			pool_add_task(Close_Mtk76xxSystem,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
			PlaySystemAmrVoices(LOW_BATTERY,playEventNums);			
			break;
		case RESET_HOST_V_PLAY:						//�ָ���������
			PlaySystemAmrVoices(RESET_HOST_V,playEventNums);
			break;
//----------------------�����й�-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//�������������������ʧ��
			PlaySystemAmrVoices(REQUEST_FAILED,playEventNums);
			break;
		case UPDATA_END_PLAY:						//���¹̼�����
			PlaySystemAmrVoices(UPDATA_END,playEventNums);
			break;
		case TIMEOUT_PLAY_LOCALFILE:				//�����������ʱ�����ű����Ѿ�¼�ƺõ���Ƶ
			TaiwanToTulingError(playEventNums);
			break;
//----------------------�����й�-----------------------------------------------------
		case CONNET_ING_PLAY:						//�������ӣ����Ե�
			//showFacePicture(CONNECT_WIFI_ING_PICTURE);//��������wifi 		
			PlaySystemAmrVoices(CHANGE_NETWORK,playEventNums);
			start_event_play_wav();
			PlaySystemAmrVoices(CONNET_TIME,playEventNums);
			break;
		case START_SMARTCONFIG_PLAY:		//��������
			pool_add_task(Led_vigue_open,NULL);
			led_lr_oc(closeled);
			setNetWorkLive(NETWORK_ER);
			PlaySystemAmrVoices(START_INTERNET,playEventNums);
			break;
		case SMART_CONFIG_OK_PLAY:		//��������ɹ�
			PlaySystemAmrVoices(YES_REAVWIFI,playEventNums);
			break;
		case CONNECT_OK_PLAY:			//���ӳɹ�	
			showFacePicture(CONNECT_WIFI_OK_PICTURE);	
#ifdef HUASHANG_JIAOYU
			pool_add_task(updateHuashangFacePthread,&playEventNums);
#endif			
			Link_NetworkOk();			//���ӳɹ��صƣ����ƣ�״̬����
			enable_gpio();
			if(!PlaySystemAmrVoices(LINK_SUCCESS,playEventNums)){
#ifdef HUASHANG_JIAOYU		
				start_event_play_wav();
				PlaySystemAmrVoices(PLAY_START_HUASHANG_MUSIC,playEventNums);
#endif
			}
			break;
		case NOT_FIND_WIFI_PLAY:		//û��ɨ�赽wifi
			PlaySystemAmrVoices(NO_WIFI,playEventNums);
			enable_gpio();
			break;
		case SMART_CONFIG_FAILED_PLAY:	//û���յ��û����͵�wifi
			PlaySystemAmrVoices(NOT_REAVWIFI,playEventNums);
			break;
		case NOT_NETWORK_PLAY:			//û�����ӳɹ�
			PlaySystemAmrVoices(NO_NETWORK_VOICES,playEventNums);
			Link_NetworkError();
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//��������Ƿ����
			ScanWifi_AgainForConnect(enable_gpio);
			//PlaySystemAmrVoices(CHECK_INTERNET,playEventNums);
			break;		
//----------------------�Խ��й�-----------------------------------------------------
		case SEND_OK_PLAY:				//���ͳɹ�
			PlaySystemAmrVoices(SEND_OK,playEventNums);
			break;
		case SEND_ERROR_PLAY:			//����ʧ��
			PlaySystemAmrVoices(SEND_ERROR,playEventNums);
			break;
		case SEND_LINK_PLAY:			//���ڷ���
			PlaySystemAmrVoices(SEND_LINK,playEventNums);
			break;
		case KEY_DOWN_PLAY:				//��������	=---���ڷ���
			PlaySystemAmrVoices(KEY_VOICE_DOWN,playEventNums);
			break;
		case PLAY_ERROT_PLAY:			//����ʧ��
			PlaySystemAmrVoices(PLAY_ERROR,playEventNums);
			break;
		case LIKE_ERROT_PLAY:			//
			PlaySystemAmrVoices(LIKE_ERROR,playEventNums);
			break;
		case TF_ERROT_PLAY:				//TF����ʧ��
			PlaySystemAmrVoices(TF_ERROR,playEventNums);
			break;
//=====================================================================
		case BIND_SSID_PLAY:			//
			PlaySystemAmrVoices(BIND_SSID,playEventNums);
			break;
		case BIND_OK_PLAY:			//
			PlaySystemAmrVoices(BIND_OK,playEventNums);
			break;
		case SEND_LINK_ER_PLAY:			//
			PlaySystemAmrVoices(SEND_LINK_ER,playEventNums);
			break;
		case TALK_CONFIRM_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM,playEventNums);
			break;
		case TALK_CONFIRM_OK_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM_OK,playEventNums);
			break;
		case TALK_CONFIRM_ER_PLAY:			//
			PlaySystemAmrVoices(TALK_CONFIRM_ER,playEventNums);
			break;
#ifdef DOWN_IMAGE			
		case DOWNLOAD_ING_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_ING,playEventNums);
			break;
		case DOWNLOAD_ERROE_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_ERROE,playEventNums);
			break;
		case DOWNLOAD_END_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_END,playEventNums);
			break;
		case DOWNLOAD_25_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_25,playEventNums);
			break;
		case DOWNLOAD_50_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_50,playEventNums);
			break;
		case DOWNLOAD_75_PLAY:			//
			PlaySystemAmrVoices(DOWNLOAD_75,playEventNums);
			break;
		case UPDATA_NEW_PLAY:			//
			PlaySystemAmrVoices(UPDATA_NEW,playEventNums);
			break;
		case UPDATA_START_PLAY:			//
			PlaySystemAmrVoices(UPDATA_START,playEventNums);
			break;
		case UPDATA_ERROR_PLAY:			//
			PlaySystemAmrVoices(UPDATA_ERROR,playEventNums);
			break;
#endif
//==========================================================================
		case NETWORK_ERROT_PLAY:				//��������ʧ��
			Handle_PlayTaiBenToNONetWork(playEventNums);
			break;
		case AI_KEY_TALK_ERROR: 
			TaiBenToTulingNOVoices(playEventNums);
			break;
		case MIN_10_NOT_USER_WARN: 
			Custom_Interface_RunPlayVoices(playEventNums);
			break;
		case TULING_WAIT_VOICES:
			vol =GetVol();
			Setwm8960Vol(VOL_APP_SET,PLAY_PASUSE_VOICES_VOL);
			PlayImportVoices(TULING_WINT,playEventNums);
			Setwm8960Vol(VOL_SET_VAULE,vol);
			break;
		case CONTINUE_PLAY_MUSIC_VOICES:
			PlaySystemAmrVoices(PLAY_CONTINUE_MUSIC,playEventNums);
			break;
#ifdef HUASHANG_JIAOYU				
		case HUASHANG_SLEEP_VOICES:
			if(!PlaySystemAmrVoices(END_SYS_VOICES,playEventNums)){
				usleep(100000);
				showFacePicture(WAIT_CTRL_NUM1);
				SleepRecoder_Phthread();			
			}		
			break;
		case HUASHANG_START_10_VOICES:
			PlaySystemAmrVoices(PLAY_START_HUASHANG_MUSIC,playEventNums);
			break;
#endif				
		default:
			pause_record_audio();
			break;
	}
}
//-------end--------����ϵͳ�����йصġ��¼��Ĳ��������Ѵ���-----------------------------------------------------
//����΢�ŷ��͹��������ļ�  filename ���͹�����΢�������ļ�
void CreatePlayWeixinVoicesSpeekEvent(const char *filename){
	unsigned char mixMode =NORMAL_PLAY_PCM;
	if(GetRecordeVoices_PthreadState() ==START_TAIK_MESSAGE||GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==END_SPEEK_VOICES||GetRecordeVoices_PthreadState() ==SOUND_MIX_PLAY){
		return;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){
		//Create_CleanUrlEvent();
		mixMode =MIX_PLAY_PCM;
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->data= (char *)calloc(1,strlen(filename)+1);
		if(handtext->data==NULL){
			goto exit1;
		}
		sprintf(handtext->data,"%s",filename);
		handtext->event = SPEEK_VOICES_EVENT;
		handtext->mixMode=mixMode;
		handtext->EventNums=updateCurrentEventNums();
		if(AddworkEvent(handtext,sizeof(HandlerText_t))){
			printf("add play amr voices failed ,and remove file \n");
			goto exit2;
		}
	}
	return;
exit2:
	free(handtext->data);
exit1:
	free(handtext);
exit0:
	remove(filename);
}

static Speek_t *speek=NULL;
//�ȹ���¼�����˳�
static void shortVoicesClean(void){
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	pthread_mutex_unlock(&speek->mutex);
}
/*******************************************************
��������: ���������͵��ļ��������ֵ����ص���,д��wavͷ
����:	��
����ֵ: 0 �����ɹ� -1 ����ʧ��
********************************************************/
static int CreateRecorderFile(void){
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	if((speek->savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		pthread_mutex_unlock(&speek->mutex);
		return -1;
	}
	speek->file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,speek->savefilefp);
	DEBUG_EVENT("create save file \n");
	pthread_mutex_unlock(&speek->mutex);
	return 0;
}
/*******************************************************
��������: ֹͣ¼�����������ݷ��͸�app�û�
����:
����ֵ:
********************************************************/
static void StopRecorder_AndSendFile(unsigned int playEventNums){
	char filepath[64]={0};
	pcmwavhdr.size_8 = (speek->file_len+36);
	pcmwavhdr.data_size = speek->file_len;
	DEBUG_EVENT("fseek file \n");
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fseek(speek->savefilefp,0,SEEK_SET);
		fwrite(&pcmwavhdr,1,WAV_HEAD,speek->savefilefp);
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	
		pthread_mutex_unlock(&speek->mutex);
		time_t t;
		t = time(NULL);
		sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
		showFacePicture(CONNECT_WIFI_OK_PICTURE);
		if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
			DEBUG_EVENT("enc_wav_amr_file failed");
			usleep(100000);
			showFacePicture(WAIT_CTRL_NUM4);
			return;
		}
		DEBUG_EVENT("stop save file \n");
		uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
		PlaySystemAmrVoices(KEY_VOICE_DOWN,playEventNums);	//����΢����ʾ������ʾ��Ƶ���ڷ���	
	}else{
		pthread_mutex_unlock(&speek->mutex);
	}
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
��������:΢�������Խ��������º͵����¼�
����:gpioState 0 ����  1 ����
********************************************************/
void Create_WeixinSpeekEvent(unsigned int gpioState){	
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
		return;
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_MP3_MUSIC){		//��ϲ�������
		Create_CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_WAV||GetRecordeVoices_PthreadState()==SOUND_MIX_PLAY||GetRecordeVoices_PthreadState()==START_SPEEK_VOICES){
		return;
	}
	CloseWifi();
	DEBUG_EVENT("state %d\n",gpioState);
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){	
		handtext->EventNums = updateCurrentEventNums();
		handtext->event=TALK_EVENT_EVENT;
		handtext->playLocalVoicesIndex=gpioState;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
/*******************************************************
��������:����΢�����������¼� 
����: gpioState 0 ����  1 ����
********************************************************/
void Handle_WeixinSpeekEvent(unsigned int gpioState,unsigned int playEventNums){
	int endtime,voicesTime=0;
	time_t t;
	if(gpioState==VOLKEYDOWN&&GetRecordeVoices_PthreadState()==RECODE_PAUSE){	//����
		DEBUG_EVENT("gpioState(%d)...\n",gpioState);
		start_speek_wait();
		if(CreateRecorderFile()){		//������Ƶ�ļ��ڵ㣬�����뵽������
			pause_record_audio();
		}else{
			speek->Starttime=time(&t);
			start_event_talk_message();
			speek->freeVoiceNums=5;
		}	
		showFacePicture(WAIT_CTRL_NUM2);
	}else if(gpioState==VOLKEYUP){			//����
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		voicesTime = endtime - speek->Starttime;
		start_event_play_wav();
		if(voicesTime<1||voicesTime>10){//ʱ��̫�̻�̫��
			shortVoicesClean();
			PlaySystemAmrVoices(SEND_ERROR,playEventNums);
			return ;
		}else{
			StopRecorder_AndSendFile(playEventNums);
			pause_record_audio();
		}
	}
}
/*******************************************************
��������:������������ (��Ҫ����Ƶ����ѹ������)
����:  voices_data ԭʼ��Ƶ����  size ԭʼ��Ƶ���ݴ�С
����ֵ:
********************************************************/
void SaveRecorderVoices(const char *voices_data,int size){
	int i=0;
	int endtime;
	time_t t;
#if defined(HUASHANG_JIAOYU)	
	if(speek->freeVoiceNums>0){
		speek->freeVoiceNums--;
		return;
	}
#endif	
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		endtime=time(&t);
		if((endtime - speek->Starttime)>60){
			pause_record_audio();
		}
#if 0	//������
		for(i=2; i<size; i+=4)		//˫��������ת�ɵ���������
		{
			fwrite(voices_data+i,2,1,speek->savefilefp);
		}
#else	//������
		for(i=0; i<size; i+=4){
			fwrite(voices_data+i,2,1,speek->savefilefp);
		}
#endif
		i /=2; 
		speek->file_len +=i;
	}else{
		pause_record_audio();
	}
	pthread_mutex_unlock(&speek->mutex);
}
#ifdef PALY_URL_SD	
//--------------------------------------sdcard �ղ�ϲ������---------------------------------------------------
//���sdcard �Ƿ���أ�û�й��أ����ϵͳ��������ʾ
static int checkSdcard_MountState(void){
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//��ʾ�û���û�м�鵽sdcard
		}
		return -1;
	}	
	return 0;
}

//ʹ���ղظ��������������߲������֣������ղ�
void Enable_SaveLoveMusicFlag(void){
	if(!checkSdcard_MountState()){
		if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){	//��鵱ǰ�Ƿ��ڲ���url���صĸ���״̬
			printf("%s: this is error save event current not online play music ,cannot save love music to sdcard =%d\n",__func__,GetRecordeVoices_PthreadState);
			return ;
		}
		printf("%s: like music add \n",__func__);
		Save_like_music();
	}
}
//ɾ��ϲ������ delete
void Delete_LoveMusic(void){
	if(!checkSdcard_MountState()){
		if(GetRecordeVoices_PthreadState()!=PLAY_MP3_MUSIC){	//��鲥��״̬
			printf("%s: this is error save event current not online play music ,cannot save love music to sdcard \n",__func__);
			return ;
		}
		printf("Del_like_music like music sub \n");
		Del_like_music();
	}
}
//��������΢������mp3 �¼������̶߳��е���
void Create_SaveWeixinDownMp3_EventToMainQueue(const char *saveFileName){
	int len = strlen(saveFileName);
	char *filename=(char *)calloc(1,len+1);
	if(filename){
		snprintf(filename,len+1,"%s",saveFileName);
		SetMainQueueLock(MAIN_QUEUE_UNLOCK);
		AddDownEvent((const char *)filename,WEIXIN_DOWN_MP3_EVENT);
	}
}
#endif
//----------------------end sdcard �ղ�ϲ������--------------------------------------------------------------

#ifdef LOCAL_MP3
//��������sdcard �������ݿ���Ϣ
static void *waitLoadMusicList(void *arg){
	int timeout=0;
	char dirBuf[128]={0};
#ifdef HUASHANG_JIAOYU	
	InitHuashang();
#endif
	sleep(20);
	while(++timeout<20){
		if(!access(TF_SYS_PATH, F_OK)){		//���tf��
			break;
		}
		//��⵽�ؼ��ļ�����ֱ���˳�����ִ�������������ֹ�ڹػ����̶�дsdcard
		if(access(CLOSE_SYSTEM_LOCK_FILE, F_OK)==F_OK){
			return NULL;
		}
		sleep(1);
	}
	snprintf(dirBuf,128,"%s",MP3_LIKEPATH);
	if(access(MP3_LIKEPATH, F_OK)){			//����ϲ������Ŀ¼
		mkdir(MP3_LIKEPATH,0777);	
	}
	SysOnloadMusicList((const char *)TF_SYS_PATH,(const char *)TF_MP3_PATH,(const char *)TF_STORY_PATH,(const char *)TF_ENGLISH_PATH,(const char *)TF_GUOXUE_PATH);
	timeout=0;
	while(++timeout<35){
		CheckNetManger_PidRunState();	//����������
		sleep(1);
		if(sysMes.netstate==NETWORK_OK){
			break;
		}else if(sysMes.netstate==NETWORK_ER){
			break;
		}
	}
	if(sysMes.netstate==NETWORK_UNKOWN){	//Ĭ����δ֪״̬����ʱ��δ�յ��������̷��͹�����״̬��ֱ��ʹ��gpio
		sysMes.netstate=NETWORK_ER;
		enable_gpio();
	}
#ifdef HUASHANG_JIAOYU
	openSystemload_huashangData();
	//Huashang_changePlayVoicesName();	//���ڲ����ã��л�������
#endif	
	return NULL;
} 
#endif

/******************************************************************
��ʼ��8960��ƵоƬ������8K¼���Ͳ���˫��ģʽ,��ʼ��gpio�����ſ���������
*******************************************************************/
void InitMtkPlatfrom76xx(void){
	speek = (Speek_t *)calloc(1,sizeof(Speek_t));
	if(speek==NULL){
		return ;
	}
	speek->savefilefp=NULL;
	speek->file_len=0;
	pthread_mutex_init(&speek->mutex, NULL);

	InitMtk76xx_gpio();
	InitWm8960Voices();
	InitRecord_VoicesPthread();
	initStream(ack_playCtr,WritePcmData,SetWm8960Rate,GetVol,GetWm8960Rate);
	InitEventMsgPthread();
	
#ifdef LOCAL_MP3
	InitMusicList();
	pool_add_task(waitLoadMusicList, NULL);	//��ֹT��������
#endif
}
/*
@ 
@ ����ƽ̨����������Դ
*/
void CleanMtkPlatfrom76xx(void){
	ExitRecord_Voicespthread();
	System_StateLog("ExitRecord_Voicespthread ok\n");
	CleanEventMsgPthread();
	System_StateLog("CleanEventMsgPthread ok \n");
	DestoryWm8960Voices();
	System_StateLog("DestoryWm8960Voices ok \n");
	CleanMtk76xx_gpio();
	System_StateLog("CleanMtk76xx_gpio ok \n");
	cleanStream();
	System_StateLog("cleanStream ok \n");
}
