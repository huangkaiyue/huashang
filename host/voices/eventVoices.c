#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/pool.h"
#include "host/voices/callvoices.h"
#include "host/ap_sta.h"
#include "nvram.h"
#include "../../net/network.h"
#include "host/voices/WavAmrCon.h"
#include "systools.h"
#include "gpio_7620.h"

#include "../host/studyvoices/qtts_qisc.h"
#include "../sdcard/musicList.h"
#include "uart/uart.h"
#include "config.h"

SysMessage sysMes;
//------------------------config network and set system network state---------------------------------------------------------
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
#if 1
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
#else
	return 0;
#endif
}
/*******************************************************
��������: �鿴NetManger�����Ƿ����pid��
����: pid_name	��������
����ֵ: ��
********************************************************/
static int get_pid_name(char *pid_name){
	if(!strcmp(pid_name,"NetManger")){
		return 0;
	}
	return -1;
}
static void StartNetServer(void){
	system("NetManger -t 5 -wifi on &");
	sleep(1);
}
static void CheckNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
}
//�̰������»�ȡwifi ���ֲ�����
void ShortKeyDown_ForPlayWifiMessage(void){
	if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
		if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
			return;
		}
		char buf[128]={0};
		char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
		if(strlen(wifi)<=0){
			printf("read wifi failed \n");
			return ;
		}
#ifdef DEBUG_SYSTEM_IP
		char IP[20]={0};
		GetNetworkcardIp((char * )"apcli0",IP);
		snprintf(buf,128,"������ wifi %s  IP��ַ�� %s",wifi,IP);
#else
		snprintf(buf,128,"������ wifi %s ",wifi);
#endif	
		Create_PlayQttsEvent(buf,QTTS_GBK);
	}
}

/*******************************************************
��������: ���������������¼�
����: ��
����ֵ: ��
********************************************************/
void LongNetKeyDown_ForConfigWifi(void){
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
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
int __AddNetWork_UrlForPaly(const void *data){
	WritePlayUrl_Log("url start add \n");
	if(checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
		goto exit0;
	}
	//��ֹ��ӹ���
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d \n",getEventNum());
		goto exit0;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){
		WritePlayUrl_Log("add failed ,reocde voices pthread is PLAY_WAV\n");
		goto exit0;
	}else if(GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		goto exit0;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handEvent){
		WritePlayUrl_Log("add url ok\n");
		handEvent->data = data;
		handEvent->event=URL_VOICES_EVENT;
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}
	return ret;
exit0:
	free(data);
	return -1;
}
#ifdef LOCAL_MP3
//�����Զ�����ģʽ�£���¼����ʼʱ��
void setAutoPlayMusicTime(void){
	time_t t;
	sysMes.Playlocaltime=time(&t);
}
/*******************************************************
��������: ��ӱ������ֵ����н��в���
����: localpath ����MP3���ŵ�ַ	
����ֵ: 0��ӳɹ� -1���ʧ��
********************************************************/
int __AddLocalMp3ForPaly(const char *localpath){
	if(getEventNum()>0){	//�¼�������ֱ࣬�Ӷ�������ֹ��ӹ��죬���º�������ʱ�����
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//���ڲ���qtts�ļ�
		DEBUG_EVENT(" PLAY_WAV \n");
		return -1;
	}else if(GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		return -1;
	}
	int ret=-1;
	HandlerText_t *handEvent = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));	
	if(handEvent){
		char *URL= (char *)calloc(1,strlen(localpath)+1);
		if(URL==NULL){
			perror("calloc error !!!");
			goto exit0;
		}
		sprintf(URL,"%s",localpath);		

		handEvent->data = URL;
		handEvent->event=LOCAL_MP3_EVENT;
		ret = AddworkEvent(handEvent,sizeof(HandlerText_t));
	}	
	return ret;
exit0:
	free(handEvent);
}
/*
@ ����Ŀ¼�˵���·����ȡ����sdcatd �������ֽ��в���
@ menu Ŀ¼�˵�(Ӣ��/�Ƽ�/��ѧ/�ղص����ݷ��� ) path sdcardĿ¼·�� playMode����ģʽ ��һ��/��һ��
@ 0��ӳɹ� -1���ʧ��
*/
static int GetSdcardMusicNameforPlay(unsigned char menu,const char *path, unsigned char playMode){	
	char buf[128]={0};
	char filename[64]={0};
	int ret=-1;
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==RECODE_PAUSE)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	int len =snprintf(buf,128,"%s%s",TF_SYS_PATH,path);
	if(menu==xiai){		//��ȡϲ��Ŀ¼�µĸ���·����
		if(PlayxiaiMusic((const char *)TF_SYS_PATH,path,filename, playMode) == -2){
			Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return ret;
		}
		if(!strcmp(filename,"")){//��ȡ��·����Ϊ�գ�ֱ���˳�
			//Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
			return ret;
		}
	}else{				//��ȡ�ͻ��Զ����Ÿ���·����
		if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, playMode)){
			Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
			return ret;
		}
		if(!strcmp(filename,"")){//��ȡ��·����Ϊ�գ�ֱ���˳�
			return ret;
		}
	}
	snprintf(buf+len,128-len,"%s",filename);
	ret=__AddLocalMp3ForPaly((const char *)buf);			//��Ӹ��������в���
	if(ret==0)
		sysMes.localplayname=menu;
	return ret;
}
#endif


/*******************************************************
��������: ����MP3
����: play ����MP3�������� �� URL��ַ
����ֵ: ��
********************************************************/
int Create_playMusicEvent(const void *play,unsigned char Mode){
#ifdef LOCAL_MP3
	int ret=-1;
	if(!strcmp((const char *)play,"mp3")){
		ret=GetSdcardMusicNameforPlay(mp3,TF_MP3_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"story")){
		ret=GetSdcardMusicNameforPlay(story,TF_STORY_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"english")){
		ret=GetSdcardMusicNameforPlay(english,TF_ENGLISH_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"guoxue")){
		ret=GetSdcardMusicNameforPlay(guoxue,TF_GUOXUE_PATH,Mode);
	}
	else if(!strcmp((const char *)play,XIAI_DIR)){
		ret=GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,Mode);
	}
#if defined(HUASHANG_JIAOYU)	
	else if(!strcmp((const char *)play,HUASHANG_GUOXUE_DIR)){
		GetScard_forPlayHuashang_Music(Mode,(const void *)play);
	}
#endif	
	else{
		ret=0;
		__AddNetWork_UrlForPaly(play);
	}
	return ret;
#else
	return __AddNetWork_UrlForPaly(play);
#endif	
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
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//��������ܻỰ���̵��У���Ӳ���ϵͳ����������wifi ���ֵ��µ���������  2017-3-26 
		printf("%s: current is play wav\n",__func__);
		return ;
	}
	if (GetRecordeVoices_PthreadState() == PLAY_DING_VOICES){
		return ;
	}
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������
		return;
	}	
	if (GetRecordeVoices_PthreadState() == PLAY_URL){	//��ǰ���Ÿ���
		Create_CleanUrlEvent();
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		updateCurrentEventNums();
		handtext->EventNums= GetCurrentEventNums();
		handtext->data= (char *)calloc(1,strlen(txt)+1);
		if (handtext->data){
			sprintf(handtext->data,"%s",txt);
			handtext->event =QTTS_PLAY_EVENT;
			handtext->playLocalVoicesIndex=type;
			AddworkEvent((const char *)handtext,sizeof(HandlerText_t));
		}
	}
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
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}	
	else if (GetRecordeVoices_PthreadState() == PLAY_URL){//���ڲ��Ÿ���״̬	
		Create_CleanUrlEvent();
		Write_Speekkeylog((const char *)"PLAY_URL",GetRecordeVoices_PthreadState());
	}else{		
#if defined(HUASHANG_JIAOYU)	//���Ͻ�������������ʶ��ӿڣ���Ҫ�ɼ���Ƶ��������ʶ��
#ifdef XUN_FEI_OK		
#else
	if (checkNetWorkLive(ENABLE_CHECK_VOICES_PLAY)){	//�������,û������ֱ���˳�����
		return;
	}
	#endif
#endif		
		sysMes.localplayname=0;	
		NetStreamExitFile();	//��΢�Ŷ����͸�����û�н��в�����һ�׸�����ʱ��ͻȻ�������ܻỰ��������Ҫ�л������ʣ����ܽ������ܻỰ״̬
		if(SetWm8960Rate(RECODE_RATE)){	//�л�������ʧ�ܣ��˳�(��ֹ���̵߳����л�����Դ��ͻ����)
			return ;
		}
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
static void CreateCloseSystemLock(void){
	FILE *fp = fopen(CLOSE_SYSTEM_LOCK_FILE,"w+");
	if(fp){
		fclose(fp);
	}
}
//�����¼��ص�����
void UartEventcallFuntion(int event){
	if(event==UART_EVENT_CLOSE_SYSTEM){	//�������˳��¼�	
		showFacePicture(CLOSE_SYSTEM_PICTURE);
		CreateCloseSystemLock();
#ifdef PALY_URL_SD
		pool_add_task(Close_Mtk76xxSystem,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
		cleanQuequeEvent();	//�������
		if(GetRecordeVoices_PthreadState()==PLAY_WAV){
		}
		if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
			NetStreamExitFile();
		}
		//����һ������ϵͳ������
		Create_PlaySystemEventVoices(END_SYS_VOICES_PLAY);
	}else if(event==UART_EVENT_LOW_BASTERRY){
		Create_PlaySystemEventVoices(LOW_BATTERY_PLAY);
	}	
} 

/*******************************************************
��������: ����һ������ϵͳ�����¼���
����: sys_voices ϵͳ�����	
����ֵ: ��
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		return;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_URL){
		Create_CleanUrlEvent();
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		updateCurrentEventNums();
		handtext->EventNums =GetCurrentEventNums();
		handtext->playLocalVoicesIndex =sys_voices;
		handtext->event =SYS_VOICES_EVENT;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}	
	
}
//��Ӳ��Ź������¼�
void Create_PlayTulingWaitVoices(int sys_voices){
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->event =SYS_VOICES_EVENT;
		handtext->playLocalVoicesIndex =sys_voices;
		AddworkEvent(handtext,sizeof(HandlerText_t));
	}
}
/*******************************************************
��������: ϵͳ���¼�������
����: sys_voices ϵͳ�����
����ֵ: ��
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices,unsigned int playEventNums){
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//������
			PlaySystemAmrVoices(END_SYS_VOICES,playEventNums);
			Led_vigue_close();
			Led_System_vigue_close();
#if defined(TANGTANG_LUO) || defined(QITUTU_SHI) || defined(HUASHANG_JIAOYU)
			close_sys_led();
#endif
#ifdef DATOU_JIANG
			open_sys_led();
#endif
			led_lr_oc(closeled);
			usleep(500*1000);
			Mute_voices(MUTE);		//�رչ���
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
			//system("sleep 8 && reboot &");
			break;
		case TIMEOUT_PLAY_LOCALFILE:				//�����������ʱ�����ű����Ѿ�¼�ƺõ���Ƶ
			TaiwanToTulingError(playEventNums);
			break;
//----------------------�����й�-----------------------------------------------------
		case CONNET_ING_PLAY:			//�������ӣ����Ե�
			showFacePicture(CONNECT_WIFI_ING_PICTURE);//��������wifi 		
			PlaySystemAmrVoices(CHANGE_NETWORK,playEventNums);
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
			PlaySystemAmrVoices(LINK_SUCCESS,playEventNums);
			Link_NetworkOk();		//���ӳɹ��صƣ����ƣ�״̬����
			enable_gpio();
			showFacePicture(MUSIC_HZ_PICTURE);
			break;
		case NOT_FIND_WIFI_PLAY:			//û��ɨ�赽wifi
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
			PlaySystemAmrVoices(CHECK_INTERNET,playEventNums);
			break;
		case WIFI_CHECK_PLAY:			//�������
			PlaySystemAmrVoices(CHECK_WIFI,playEventNums);
			PlaySystemAmrVoices(CHECK_WIFI_WAIT,playEventNums);
			break;
		case WIFI_NO:					//�������NO
			PlaySystemAmrVoices(CHECK_WIFI_NO,playEventNums);
			break;
		case WIFI_YES:					//�������OK
			PlaySystemAmrVoices(CHECK_WIFI_YES,playEventNums);
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
//==========================================================================
		case NETWORK_ERROT_PLAY:				//��������ʧ��
			Handle_PlayTaiBenToNONetWork(playEventNums);
			break;
		case AI_KEY_TALK_ERROR: 
			TaiBenToTulingNOVoices(playEventNums);
			break;
		case MIN_10_NOT_USER_WARN: 
			PlaySystemAmrVoices(SPEEK_WARNING,playEventNums);
			break;
		case TULING_WAIT_VOICES:
			play_waitVoices(TULING_WINT,playEventNums);
			printf("%s: play wait voices ok\n",__func__);
			break;
		default:
			pause_record_audio();
			break;
	}
}
//-------end--------����ϵͳ�����йصġ��¼��Ĳ��������Ѵ���-----------------------------------------------------
#ifdef SPEEK_VOICES
//����΢�ŷ��͹��������ļ�  filename ���͹�����΢�������ļ�
void CreatePlayWeixinVoicesSpeekEvent(const char *filename){
	if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		goto exit0;
	}
	else if(GetRecordeVoices_PthreadState() == PLAY_WAV){	//��������ܻỰ���̵��У����΢�ŷ��͹������������µ���������  2017-4-26 
		goto exit0;
	}
	else if(GetRecordeVoices_PthreadState() ==PLAY_URL){
		Create_CleanUrlEvent();
	}
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){
		handtext->data= (char *)calloc(1,strlen(filename)+1);
		if(handtext->data==NULL){
			goto exit1;
		}
		sprintf(handtext->data,"%s",filename);
		handtext->event = SPEEK_VOICES_EVENT;
		if(AddworkEvent(handtext,sizeof(HandlerText_t))){
			printf("add play amr voices failed ,and remove file \n");
			goto exit2;
		}
		return;
	}
exit2:
	free(handtext->data);
exit1:
	free(handtext);
exit0:
	remove(filename);
}
#endif

#ifdef SPEEK_VOICES
static Speek_t *speek=NULL;
//�ȹ���¼�����˳�
static void shortVoicesClean(void){
	//lock
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	pthread_mutex_unlock(&speek->mutex);
	//unlock
}
/*******************************************************
��������: ���������͵��ļ��������ֵ����ص���,д��wavͷ
����:	��
����ֵ: 0 �����ɹ� -1 ����ʧ��
********************************************************/
static int CreateRecorderFile(void){
	//lock
	pthread_mutex_lock(&speek->mutex);
	if(speek->savefilefp!=NULL){
		fclose(speek->savefilefp);
		speek->savefilefp=NULL;
	}
	if((speek->savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		pthread_mutex_unlock(&speek->mutex);
		//unlock
		return -1;
	}
	speek->file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,speek->savefilefp);
	DEBUG_EVENT("create save file \n");
	pthread_mutex_unlock(&speek->mutex);
	//unlock
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
		if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
			DEBUG_EVENT("enc_wav_amr_file failed");
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
	if(GetRecordeVoices_PthreadState() ==PLAY_URL){		//��ϲ�������
		Create_CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_DING_VOICES){
		return;
	}
	DEBUG_EVENT("state %d\n",gpioState);
	HandlerText_t *handtext = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(handtext){	
		updateCurrentEventNums();
		handtext->EventNums = GetCurrentEventNums();
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
		}
	}else if(gpioState==VOLKEYUP){			//����
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		voicesTime = endtime - speek->Starttime;
		if(voicesTime<2||voicesTime>10){//ʱ��̫�̻�̫��
			pause_record_audio();
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
#endif			//endif SPEEK_VOICES  :΢�ŶԽ����� �¼��Ĳ����ʹ���

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
		if(GetRecordeVoices_PthreadState()!=PLAY_URL){	//��鵱ǰ�Ƿ��ڲ���url���صĸ���״̬
			printf("%s: this is error save event current not online play music ,cannot save love music to sdcard \n",__func__);
			return ;
		}
		printf("%s: like music add \n",__func__);
		Save_like_music();
	}
}
//ɾ��ϲ������ delete
void Delete_LoveMusic(void){
	if(!checkSdcard_MountState()){
		if(GetRecordeVoices_PthreadState()!=PLAY_URL){	//��鲥��״̬
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
	sleep(10);
	while(++timeout<20){
		if(!access(TF_SYS_PATH, F_OK)){		//���tf��
			break;
		}
		//��⵽�ؼ��ļ�����ֱ���˳�����ִ�������������ֹ�ڹػ����̶�дsdcard
		if(access(CLOSE_SYSTEM_LOCK_FILE, F_OK)==F_OK){
			return ;
		}
		sleep(1);
	}
	snprintf(dirBuf,128,"%s",MP3_LIKEPATH);
	if(access(MP3_LIKEPATH, F_OK)){			//����ϲ������Ŀ¼
		mkdir(MP3_LIKEPATH,0777);	
	}
	SysOnloadMusicList((const char *)TF_SYS_PATH,(const char *)TF_MP3_PATH,(const char *)TF_STORY_PATH,(const char *)TF_ENGLISH_PATH,(const char *)TF_GUOXUE_PATH);
	CheckNetServer();	//����������
	if(sysMes.netstate==NETWORK_UNKOWN){
		sysMes.netstate=NETWORK_ER;
		enable_gpio();
	}
#ifdef HUASHANG_JIAOYU
	openSystemload_huashangData();
#endif
	return;
} 
#endif

#ifdef HUASHANG_JIAOYU
//��ʾ���ܻỰ����
void Show_SmartTalkKey(void){
	int faceNumS=(1+(int) (3.0*rand()/(RAND_MAX+1.0)));	
	switch(faceNumS){
		case 1:
			showFacePicture(FACE_jingya_42);
			break;
		case 2:
			showFacePicture(FACE_qinqin_51);
			break;
		case 3:
			showFacePicture(FACE_eye_show_64);
			break;
		default:
			showFacePicture(FACE_eye_show_64);
			break;
	}
}
#endif

/******************************************************************
��ʼ��8960��ƵоƬ������8K¼���Ͳ���˫��ģʽ,��ʼ��gpio�����ſ���������
*******************************************************************/
void InitMtkPlatfrom76xx(void){
#ifdef SPEEK_VOICES
	speek = (Speek_t *)calloc(1,sizeof(Speek_t));
	if(speek==NULL){
		return ;
	}
	speek->savefilefp=NULL;
	speek->file_len=0;
	pthread_mutex_init(&speek->mutex, NULL);
#endif

	InitMtk76xx_gpio();
	InitWm8960Voices();
#if defined(HUASHANG_JIAOYU)
	PlaySystemAmrVoices(WELCOME_PLAY,0);
#else
	PlaySystemAmrVoices(START_SYS_VOICES,0);//����������
#endif
	initStream(ack_playCtr,WritePcmData,SetWm8960Rate,GetVol);

	InitEventMsgPthread();
	InitRecord_VoicesPthread();
#ifdef TEST_SDK
	enable_gpio();
#endif

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
	DestoryWm8960Voices();
	CleanMtk76xx_gpio();
	CleanEventMsgPthread();
	cleanStream();
}
