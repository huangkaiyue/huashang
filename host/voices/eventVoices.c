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
#include "config.h"

extern void DelSdcardMp3file(char * sdpath);
extern int CheckFileNum(char * sdpath);

SysMessage sysMes;
/*
@ û�������ʱ�򣬲��ű���ϵͳ�̶�¼��̨��
@
*/
static void TaiBenToNONetWork(void){
#if 0			//������������
	static int starttime_net=0;
	int endtime;
	time_t t;
	endtime=time(&t);
	if(((endtime-starttime_net)>1*MIN)&&(!checkconnetState())){
		starttime_net=time(&t);
		startServiceWifi();
	}
#endif
	//srand((unsigned)time(NULL));
	int i=(1+(int)(5.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case NETWORK_ERR_VOICES_1:
			play_sys_tices_voices(NETWORK_ERROR_1);
			break;
		case NETWORK_ERR_VOICES_2:
			play_sys_tices_voices(NETWORK_ERROR_2);
			break;
		case NETWORK_ERR_VOICES_3:
			play_sys_tices_voices(NETWORK_ERROR_3);
			break;
		case NETWORK_ERR_VOICES_4:
			play_sys_tices_voices(NETWORK_ERROR_4);
			break;
		case NETWORK_ERR_VOICES_5:
			play_sys_tices_voices(NETWORK_ERROR_5);
			break;
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
@ 
@
*/
static int checkNetWorkLive(void){
	if(getNetWorkLive()==NETWORK_ER||getNetWorkLive()==NETWORK_UNKOWN){
		//����̨��
		if(getEventNum()>0){	//����Ƿ���ӹ��¼�
			return -1;
		}
		//���ϵͳ����ȥ���ţ���ʾ�û���������
		Create_PlaySystemEventVoices(NETWORK_ERROT_PLAY);
		return -1;
	}else if(getNetWorkLive()==NETWORK_OK){
		return 0;
	}
	return -1;
}
/*******************************************************
��������: �������URL��ַ�����е��н��в���
����: data ���Ÿ�����Ϣ���Ѿ�malloc�����ڴ���	
����ֵ: ��
********************************************************/
static void __AddNetWork_UrlForPaly(const void *data){
	WritePlayUrl_Log("url start add \n");
	if(checkNetWorkLive()){	//�������
		return;
	}
	//��ֹ��ӹ���
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV_E){
		WritePlayUrl_Log("add failed ,reocde voices pthread is PLAY_WAV_E\n");
		//exitqttsPlay();
		return;
	}
	AddworkEvent((const char *)data,0,URL_VOICES_EVENT);
	WritePlayUrl_Log("add url ok\n");
}
#ifdef LOCAL_MP3
/*******************************************************
��������: ��ӱ������ֵ����н��в���
����: localpath ����MP3���ŵ�ַ	
����ֵ: 0��ӳɹ� -1���ʧ��
********************************************************/
static int __AddLocalMp3ForPaly(const char *localpath){
	if(getEventNum()>0){	//�¼�������ֱ࣬�Ӷ�������ֹ��ӹ��죬���º�������ʱ�����
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV_E){	//���ڲ���qtts�ļ�
		DEBUG_EVENT(" PLAY_WAV_E \n");
		exitqttsPlay();
		return -1;
	}
	char *URL= (char *)calloc(1,strlen(localpath)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return -1;
	}
	sprintf(URL,"%s",localpath);
	return AddworkEvent(URL,0,LOCAL_MP3_EVENT);
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
		if(GetRecordeVoices_PthreadState()==PAUSE_E)
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ret;
	}
	if(menu==xiai){		//��ȡϲ��Ŀ¼�µĸ���·����
		PlayxiaiMusic((const char *)TF_SYS_PATH,path,filename, playMode);
	}
	else{				//��ȡ�ͻ��Զ����Ÿ���·����
		if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, playMode)){
			Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
			return ret;
		}
	}
	snprintf(buf,128,"%s%s",TF_SYS_PATH,path);
	if((menu==xiai)&&CheckFileNum(buf)){	 //ϲ��Ŀ¼Ϊ��,��ʾ�û��ղظ���
		Create_PlaySystemEventVoices(LIKE_ERROT_PLAY);
		return ret;
	}
	if(!strcmp(filename,"")){				//��ȡ��·����Ϊ�գ�ֱ���˳�
		//Create_PlaySystemEventVoices(PLAY_ERROT_PLAY);
		return ret;
	}
	snprintf(buf,128,"%s%s",buf,filename);
	printf("filepath = %s\n",buf);
	ret=__AddLocalMp3ForPaly((const char *)buf);			//��Ӹ��������в���
	if(ret==0)
		sysMes.localplayname=menu;
	return ret;
}
#endif

#ifdef PALY_URL_SD
void CreateLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==PAUSE_E)	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeVoices_PthreadState()!=PLAY_URL_E){	//��鲥��״̬
		if(GetRecordeVoices_PthreadState()==PAUSE_E){
			//Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//Fix me ��ǰû�в�������
		}
		return ;
	}
	printf("CreateLikeMusic like music add \n");
	Save_like_music();
}
void DelLikeMusic(void){
	if(access(TF_SYS_PATH, F_OK)){	//���tf��
		if(GetRecordeVoices_PthreadState()==PAUSE_E)	
			Create_PlaySystemEventVoices(TF_ERROT_PLAY);
		return ;
	}
	if(GetRecordeVoices_PthreadState()!=PLAY_URL_E){	//��鲥��״̬
		if(GetRecordeVoices_PthreadState()==PAUSE_E){
			//Create_PlaySystemEventVoices(TF_ERROT_PLAY);	//Fix me ��ǰû�в�������
		}
		return ;
	}
	printf("Del_like_music like music sub \n");
	Del_like_music();
}
#endif
#define VOLWAITTIME		300*1000	//�����Ӽ�ʱ����
#define VOLKEY_CHANG	2			//����ʱ��
static int volstart_time=0;
static unsigned char voltype=VOLKEYUP;
static unsigned char KeyNum=0;
static void handlevolandnext(void){
	time_t t;
	int volendtime=0;
	int ret;
	while(1){
		volendtime=time(&t);
		printf("volendtime-volstart_time = %d\n",volendtime-volstart_time);
		if((volendtime-volstart_time)<VOLKEY_CHANG){
			if(voltype==VOLKEYUP){
				//��һ��
				if(KeyNum==ADDVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_NEXT);	//��һ��
				}else if(KeyNum==SUBVOL_KEY){
					createPlayEvent((const void *)"xiai",PLAY_PREV);	//��һ��
				}
				break;
			}
			usleep(10*1000);
			continue;
		}else if((volendtime-volstart_time)<5){
			if(voltype==VOLKEYUP){
				break;
			}
			if(KeyNum==ADDVOL_KEY){
				ret = Setwm8960Vol(VOL_ADD,0);	//������
			}else if(KeyNum==SUBVOL_KEY){
				ret = Setwm8960Vol(VOL_SUB,0);	//������
			}
			if(ret==1){	//����������
				break;
			}
			//������
			usleep(VOLWAITTIME);
		}else{
			break;
		}
	}
	KeyNum=0;
}
/*
@ ���ð����ӿڣ��̰��л���������������������
@ 
@
*/
void VolAndNextKey(unsigned char state,unsigned char dir){
	time_t t;
	if(KeyNum==0||dir==KeyNum){
		if(state==VOLKEYDOWN){	//����
			volstart_time=time(&t);
			voltype=VOLKEYDOWN;
			KeyNum=dir;
			pool_add_task(handlevolandnext,NULL);
		}else{			//����
			voltype=VOLKEYUP;
		}
	}
}
//�����Զ�����ģʽ�£���¼����ʼʱ��
void setAutoPlayMusicTime(void){
	time_t t;
	sysMes.Playlocaltime=time(&t);
}
/*******************************************************
��������: ����MP3
����: play ����MP3�������� �� URL��ַ
����ֵ: ��
********************************************************/
int createPlayEvent(const void *play,unsigned char Mode){
	int ret=-1;
#ifdef LOCAL_MP3
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
	else if(!strcmp((const char *)play,"xiai")){
		ret=GetSdcardMusicNameforPlay(xiai,XIMALA_MUSIC_DIRNAME,Mode);
	}else{
		ret=0;
		__AddNetWork_UrlForPaly(play);
	}
	return ret;
#else
	__AddNetWork_UrlForPaly(play);
#endif	
}

/*******************************************************
��������: ����URL�¼�
����: ��
����ֵ: ��
********************************************************/
void CleanUrlEvent(void){
	sysMes.localplayname=0;
	AddworkEvent(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
��������: ��������QTTS�����¼� --->�ı�ת����������
����: txt QTTS�ı� type :0---GBK ��ʽ 1----UTF8 ��ʽ
����ֵ: ��
********************************************************/
void Create_PlayQttsEvent(const char *txt,int type){
	if (checkNetWorkLive()){	//�������
		return;
	}
	if (GetRecordeVoices_PthreadState() == PLAY_URL_E){
		CleanUrlEvent();
	}
	if (GetRecordeVoices_PthreadState() == PLAY_TULING_E){
		NetStreamExitFile();
	}
	char *TXT = (char *)calloc(1,strlen(txt)+1);
	if (TXT){
		sprintf(TXT,"%s",txt);
		AddworkEvent((const char *)TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
��������: �Ự���������ź�,����¼�� 
����: ��
����ֵ: ��
********************************************************/
void TulingKeyDownSingal(void){
	Write_Speekkeylog((const char *)"speekstart",0);
	if(GetRecordeVoices_PthreadState()==START_SPEEK_VOICES||GetRecordeVoices_PthreadState()==END_SPEEK_VOICES){		
		Write_Speekkeylog((const char *)"START_SPEEK_VOICES",GetRecordeVoices_PthreadState());
		return;
	}
	if (GetRecordeVoices_PthreadState() == PLAY_WAV_E){
		exitqttsPlay();
		if (GetPlaySystem_VoicesState() == 0){
			SetPlaySystem_VoiceState(EXIT_SYSTEM_PLAY);
		}
		Write_Speekkeylog((const char *)"PLAY_WAV_E",GetRecordeVoices_PthreadState());
	}else if (GetRecordeVoices_PthreadState() == PLAY_URL_E){
		CleanUrlEvent();
		Write_Speekkeylog((const char *)"PLAY_URL_E",GetRecordeVoices_PthreadState());
	}else{	
		if (checkNetWorkLive()){	//�������
			exitqttsPlay();		//��ʱ����Ự�����У����ϵͳ����������
			Write_Speekkeylog((const char *)"exitqttsPlay",GetRecordeVoices_PthreadState());
			return;
		}	
		sysMes.localplayname=0;	
		NetStreamExitFile();
		if(SetWm8960Rate(RECODE_RATE)){
			return ;
		}
		StartTuling_RecordeVoices();
		Write_Speekkeylog((const char *)"StartTuling_RecordeVoices",GetRecordeVoices_PthreadState());
	}
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
void CheckNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove(NET_SERVER_FILE_LOCK);
		remove(INTEN_NETWORK_FILE_LOCK);
		StartNetServer();
	}
}
/*******************************************************
��������: �����¼�
����: ��
����ֵ: ��
********************************************************/
void NetKeyDown_ForConfigWifi(void){
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
	if(GetRecordeVoices_PthreadState()==PAUSE_E){//���ڲ����¼����У�����������
		disable_gpio();
		startSmartConfig(Create_PlaySystemEventVoices,enable_gpio);	
	}else{		
		WiterSmartConifg_Log("startSmartConfig  failed ","is not PAUSE_E");
	}
}
//�ػ������ļ���������
void Close_Mtk76xxSystem(void){
	char token[64]={0};
	GetTokenValue(token);
	Save_TulingToken_toRouteTable((const char *)token);
	SaveVol_toRouteTable(GetVol());		//����������·�ɱ�
#ifdef CLOCKTOALIYUN
	CloseSystemSignToaliyun();			//���͹ػ��źŸ�����
#endif

#ifdef PALY_URL_SD	
	SaveSystemPlayNum();
	DelSdcardMp3file(MP3_SDPATH);
#endif	
}
//���ӳɹ����ù���ָʾ��,����mucʱ��
void Link_NetworkOk(void){
	Led_vigue_close();
#ifdef QITUTU_SHI
	led_lr_oc(openled);
#endif
#ifdef TANGTANG_LUO
	led_lr_oc(openled);
#endif
#ifdef DATOU_JIANG
	led_lr_oc(closeled);
#endif
	SocSendMenu(3,0);			//���ͱ���ʱ���mcu
	usleep(100*1000);
	setNetWorkLive(NETWORK_OK);		//��������״̬
}
void Link_NetworkError(void){
	pool_add_task(Led_vigue_open,NULL);
#ifdef QITUTU_SHI
	led_lr_oc(closeled);
#endif
#ifdef TANGTANG_LUO
	led_lr_oc(closeled);
#endif
#ifdef DATOU_JIANG
	led_lr_oc(openled);
#endif
	setNetWorkLive(NETWORK_ER);
}
#define TLERNUM 34.0
static void TaiwanToTulingError(void){
	char buf[32]={0};
	int i=(1+(int)(TLERNUM*rand()/(RAND_MAX+1.0)));
	snprintf(buf,32,"qtts/TulingError%d_8k.amr",i);
	PlaySystemAmrVoices(buf);
}
/*******************************************************
��������: ����һ������ϵͳ�����¼���
����: sys_voices ϵͳ�����	
����ֵ: ��
********************************************************/
void Create_PlaySystemEventVoices(int sys_voices){
	PlaySystemAmrVoicesLog("playsys_start\n");
	if((sys_voices==END_SYS_VOICES_PLAY)){	//�������˳��¼�
#ifdef PALY_URL_SD
		pool_add_task(Close_Mtk76xxSystem,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
		cleanQuequeEvent();	//�������
		if(GetRecordeVoices_PthreadState()==PLAY_WAV){
			exitqttsPlay();	//�����¼�
		}
		if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
			NetStreamExitFile();
		}
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	AddworkEvent(NULL,sys_voices,SYS_VOICES_EVENT);
	PlaySystemAmrVoicesLog("playsys voices end \n");
}
/*******************************************************
��������: ϵͳ���¼�������
����: sys_voices ϵͳ�����
����ֵ: ��
********************************************************/
void Handle_PlaySystemEventVoices(int sys_voices){
	PlaySystemAmrVoicesLog("playsys voices handle \n");
//----------------------ϵͳ�й�-----------------------------------------------------
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//������
			play_sys_tices_voices(END_SYS_VOICES);
			Led_vigue_close();
			Led_System_vigue_close();
			Mute_voices(MUTE);		//�رչ���
#ifdef QITUTU_SHI
			close_sys_led();
#endif
#ifdef DATOU_JIANG
			open_sys_led();
#endif
#ifdef TANGTANG_LUO
			close_sys_led();
#endif
			led_lr_oc(closeled);
			break;
		case TULING_WINT_PLAY:						//���Ե�	
#ifdef QITUTU_SHI
			pool_add_task(Led_System_vigue_open,NULL);
#endif
#ifdef TANGTANG_LUO
			pool_add_task(Led_System_vigue_open,NULL);
#endif
			play_sys_tices_voices(TULING_WINT);
			break;
		case LOW_BATTERY_PLAY:						//�͵�ػ�
#ifdef PALY_URL_SD
			pool_add_task(Close_Mtk76xxSystem,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
			play_sys_tices_voices(LOW_BATTERY);			
			break;
		case RESET_HOST_V_PLAY:						//�ָ���������
			play_sys_tices_voices(RESET_HOST_V);
			break;
//----------------------�����й�-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//�������������������ʧ��
			PlaySystemAmrVoices(REQUEST_FAILED);
			break;
		case UPDATA_END_PLAY:						//���¹̼�����
			play_sys_tices_voices(UPDATA_END);
			//system("sleep 8 && reboot &");
			break;
		case TIMEOUT_PLAY_LOCALFILE:				//�����������ʱ�����ű����Ѿ�¼�ƺõ���Ƶ
			TaiwanToTulingError();
			break;
		case 8:
			play_sys_tices_voices(REQUEST_FAILED);
			break;
//----------------------�����й�-----------------------------------------------------
		case CONNET_ING_PLAY:			//�������ӣ����Ե�
			play_sys_tices_voices(CHANGE_NETWORK);
			play_sys_tices_voices(CONNET_TIME);
			break;
		case START_SMARTCONFIG_PLAY:		//��������
			pool_add_task(Led_vigue_open,NULL);
			led_lr_oc(closeled);
			setNetWorkLive(NETWORK_ER);
			play_sys_tices_voices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//��������ɹ�
			play_sys_tices_voices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//���ӳɹ�
			play_sys_tices_voices(LINK_SUCCESS);
			Link_NetworkOk();		//���ӳɹ��صƣ����ƣ�״̬����
#if 0
			if(!access(TF_SYS_PATH, F_OK)){
				play_sys_tices_voices(WELCOME_PLAY);
				//PlayTuLingTaibenQtts("С�������ǽ����ϴ����ݼ������ɣ���ʼ���š�",QTTS_GBK);
				createPlayEvent((const void *)"xiai",PLAY_NEXT);
			}
#endif
			enable_gpio();
			break;
		case NOT_FIND_WIFI_PLAY:			//û��ɨ�赽wifi
			play_sys_tices_voices(NO_WIFI);
			enable_gpio();
			break;
		case SMART_CONFIG_FAILED_PLAY:	//û���յ��û����͵�wifi
			play_sys_tices_voices(NOT_REAVWIFI);
			break;
		case NOT_NETWORK_PLAY:			//û�����ӳɹ�
			play_sys_tices_voices(NO_NETWORK_VOICES);
			Link_NetworkError();
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//��������Ƿ����
			play_sys_tices_voices(CHECK_INTERNET);
			break;
		case WIFI_CHECK_PLAY:			//�������
			play_sys_tices_voices(CHECK_WIFI);
			play_sys_tices_voices(CHECK_WIFI_WAIT);
			break;
		case WIFI_NO:			//�������NO
			play_sys_tices_voices(CHECK_WIFI_NO);
			break;
		case WIFI_YES:			//�������OK
			play_sys_tices_voices(CHECK_WIFI_YES);
			break;
//----------------------�Խ��й�-----------------------------------------------------
		case SEND_OK_PLAY:			//���ͳɹ�
			play_sys_tices_voices(SEND_OK);
			break;
		case SEND_ERROR_PLAY:			//����ʧ��
			play_sys_tices_voices(SEND_ERROR);
			break;
		case SEND_LINK_PLAY:			//���ڷ���
			play_sys_tices_voices(SEND_LINK);
			break;
		case KEY_DOWN_PLAY:			//��������	=---���ڷ���
			play_sys_tices_voices(KEY_VOICE_DOWN);
			break;
		case PLAY_ERROT_PLAY:			//����ʧ��
			play_sys_tices_voices(PLAY_ERROR);
			break;
		case LIKE_ERROT_PLAY:			//
			play_sys_tices_voices(LIKE_ERROR);
			break;
		case TF_ERROT_PLAY:				//TF����ʧ��
			play_sys_tices_voices(TF_ERROR);
			break;
//=====================================================================
		case BIND_SSID_PLAY:			//
			play_sys_tices_voices(BIND_SSID);
			break;
		case BIND_OK_PLAY:			//
			play_sys_tices_voices(BIND_OK);
			break;
		case SEND_LINK_ER_PLAY:			//
			play_sys_tices_voices(SEND_LINK_ER);
			break;
		case TALK_CONFIRM_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM);
			break;
		case TALK_CONFIRM_OK_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM_OK);
			break;
		case TALK_CONFIRM_ER_PLAY:			//
			play_sys_tices_voices(TALK_CONFIRM_ER);
			break;
		case DOWNLOAD_ING_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_ING);
			break;
		case DOWNLOAD_ERROE_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_ERROE);
			break;
		case DOWNLOAD_END_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_END);
			break;
		case DOWNLOAD_25_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_25);
			break;
		case DOWNLOAD_50_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_50);
			break;
		case DOWNLOAD_75_PLAY:			//
			play_sys_tices_voices(DOWNLOAD_75);
			break;
		case UPDATA_NEW_PLAY:			//
			play_sys_tices_voices(UPDATA_NEW);
			break;
		case UPDATA_START_PLAY:			//
			play_sys_tices_voices(UPDATA_START);
			break;
		case UPDATA_ERROR_PLAY:			//
			play_sys_tices_voices(UPDATA_ERROR);
			break;
//==========================================================================
		case NETWORK_ERROT_PLAY:				//��������ʧ��
			TaiBenToNONetWork();
			break;
	}
	PlaySystemAmrVoicesLog("playsys voices end \n");
}
#ifdef SPEEK_VOICES
void CreateSpeekEvent(const char *filename){
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
		NetStreamExitFile();
	}
	char *TXT= (char *)calloc(1,strlen(filename)+1);
	if(TXT){
		sprintf(TXT,"%s",filename);
		AddworkEvent(TXT,0,SPEEK_VOICES_EVENT);
	}
}
#endif

#ifdef SPEEK_VOICES

typedef struct{
	int file_len;
	FILE *savefilefp;
	int Starttime;	//¼��΢�ŶԽ���ʼʱ�䣬��������ļ�¼�Ƴ��ȣ���ֹ¼��̫�̵���Ƶ
	pthread_mutex_t mutex;
}Speek_t;
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
static void StopRecorder_AndSendFile(void){
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
	}else{
		pthread_mutex_unlock(&speek->mutex);
		return;
	}
	pthread_mutex_unlock(&speek->mutex);
	time_t t;
	t = time(NULL);
#ifdef CACHE_SDCARD
	sprintf(filepath,"%s%s%d%s",sysMes.localVoicesPath,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath)){
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("stop save file \n");
	Create_PlaySystemEventVoices(KEY_DOWN_PLAY);	//����΢����ʾ������ʾ��Ƶ���ڷ���
	uploadVoicesToaliyun(filepath,pcmwavhdr.data_size/10+6);
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
��������:΢�������Խ��������º͵����¼�
����:gpioState 0 ����  1 ����
********************************************************/
void Create_WeixinSpeekEvent(unsigned int gpioState){	
	if(checkNetWorkLive()){	//�������
		return;
	}
	if(gpioState==VOLKEYDOWN){	//����
		
	}else{			//����
	
	}
	if(GetRecordeVoices_PthreadState() ==PLAY_URL_E){		//��ϲ�������
		CleanUrlEvent();
		return;
	}else if(GetRecordeVoices_PthreadState()==PLAY_WAV_E){
		exitqttsPlay();
		return;
	}else if(GetRecordeVoices_PthreadState() ==PLAY_TULING_E){
		NetStreamExitFile();
		return;
	}
	DEBUG_EVENT("state %d\n",gpioState);
	AddworkEvent(NULL,gpioState,TALK_EVENT_EVENT);
}
/*******************************************************
��������:����΢�����������¼� 
����: gpioState 0 ����  1 ����
********************************************************/
void Handle_WeixinSpeekEvent(unsigned int gpioState){
	int endtime;
	time_t t;
	if(gpioState==VOLKEYDOWN&&GetRecordeVoices_PthreadState()==RECODE_PAUSE){	//����
		DEBUG_EVENT("gpioState(%d)...\n",gpioState);
		start_speek_wait();
		if(CreateRecorderFile()){		//������Ƶ�ļ��ڵ㣬�����뵽������
			pause_record_audio(6);
		}
		speek->Starttime=time(&t);
		start_event_talk_message();
	}else if(gpioState==VOLKEYUP){			//����
		DEBUG_EVENT("state(%d)\n",gpioState);
		start_speek_wait();
		endtime=time(&t);
		if((endtime - speek->Starttime)<2){		//ʱ��̫��
			pause_record_audio(7);
			shortVoicesClean();
			Create_PlaySystemEventVoices(SEND_ERROR_PLAY);
			return ;
		}else if((endtime - speek->Starttime)>10){		//ʱ��̫��
			pause_record_audio(7);
			shortVoicesClean();
			Create_PlaySystemEventVoices(SEND_ERROR_PLAY);
			return ;
		}
		/********************************************
		ע :��Ҫд�ļ�ͷ��Ϣ 
		*********************************************/
		StopRecorder_AndSendFile();
		pause_record_audio(8);
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
			pause_record_audio(9);
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
		pause_record_audio(10);
	}
	pthread_mutex_unlock(&speek->mutex);
}
#endif

#ifdef LOCAL_MP3
static void *waitLoadMusicList(void *arg){
	int timeout=0;
	char dirBuf[128]={0};
	sleep(10);
	while(++timeout<20){
		if(!access(TF_SYS_PATH, F_OK)){		//���tf��
			break;
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
	return;
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
#ifndef TEST_SDK
	PlaySystemAmrVoices(START_SYS_VOICES);//����������
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

