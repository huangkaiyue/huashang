#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/message_wav.h"
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

#define XIMALA_MUSIC_E	"ximalaya/"	//ϲ�������ղصĸ���		

#define START_SPEEK_E	START_SPEEK_VOICES 		  	//¼��������ʶ��
#define PAUSE_E			RECODE_PAUSE 				//¼������
#define PLAY_WAV_E		PLAY_WAV					//����wavԭʼ������
#define END_SPEEK_E		END_SPEEK_VOICES			//����¼��
#define PLAY_URL_E		PLAY_URL					//����url����

#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("event : " fmt, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

extern void DelSdcardMp3file(char * sdpath);

SysMessage sysMes;
/*******************************************************
��������: ��������URL��ַ
����: url URL��ַ	
����ֵ: ��
********************************************************/
static void CreateUrlEvent(const void *data){
	playurlLog("url_start\n");
	playurlLog(VERSION);	//�汾ʱ��
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		exitqttsPlay();
		return;
	}
	add_event_msg(data,0,URL_VOICES_EVENT);
	playurlLog("add ok\n");
}
#ifdef LOCAL_MP3
/*******************************************************
��������: ���ű��ص�ַ
����: localpath ����MP3���ŵ�ַ	
����ֵ: ��
********************************************************/
static void CreateLocalMp3(char *localpath){
	if(getEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		exitqttsPlay();
		return;
	}
	char *URL= (char *)calloc(1,strlen(localpath)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return;
	}
	sprintf(URL,"%s",localpath);
	add_event_msg(URL,0,LOCAL_MP3_EVENT);
}
static void PlayLocal(unsigned char menu,const char *path, unsigned char Mode){	
	char buf[128]={0};
	char filename[64]={0};
	if(GetSdcardMusic((const char *)TF_SYS_PATH,path,filename, Mode)){
		return;
	}
	snprintf(buf,128,"%s%s%s",TF_SYS_PATH,path,filename);
	printf("filepath = %s\n",buf);
	CreateLocalMp3(buf);
	sysMes.localplayname=menu;
}
#endif
/*******************************************************
��������: ����MP3
����: play ����MP3�������� �� URL��ַ
����ֵ: ��
********************************************************/
void createPlayEvent(const void *play,unsigned char Mode){
#ifdef LOCAL_MP3
	if(Mode==PLAY_NEXT_AUTO){
		Mode=PLAY_NEXT;
	}else{
		time_t t;
		sysMes.Playlocaltime=time(&t);
	}
	if(!strcmp((const char *)play,"mp3")){
		PlayLocal(mp3,TF_MP3_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"story")){
		PlayLocal(story,TF_STORY_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"english")){
		PlayLocal(english,TF_ENGLISH_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"guoxue")){
		PlayLocal(guoxue,TF_GUOXUE_PATH,Mode);
	}
	else if(!strcmp((const char *)play,"xiai")){
		PlayLocal(xiai,XIMALA_MUSIC_E,Mode);
	}else{
		CreateUrlEvent(play);
	}
#else
	CreateUrlEvent(play);
#endif	
}
/*******************************************************
��������: ����URL�¼�
����: ��
����ֵ: ��
********************************************************/
void CleanUrlEvent(void){
	sysMes.localplayname=0;
	add_event_msg(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
��������: QTTS�¼�
����: txt QTTS�ı� type :0---GBK 1----UTF8
����ֵ: ��
********************************************************/
void QttsPlayEvent(const char *txt,int type){
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(txt)+1);
	if(TXT){
		sprintf(TXT,"%s",txt);
		add_event_msg((const char *)TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
��������: �Ự�¼�
����: ��
����ֵ: ��
********************************************************/
void down_voices_sign(void){
	if(GetRecordeLive() ==START_SPEEK_VOICES||GetRecordeLive() ==END_SPEEK_VOICES){
		DEBUG_EVENT("check_recorde_state ...\n");
		return;
	}
	if(GetRecordeLive()==PLAY_WAV_E){
		exitqttsPlay();
	}else if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}else{
		sysMes.localplayname=0;	
		NetStreamExitFile();
		start_event_std();
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
void CacheNetServer(void){
	sleep(5);
	if(judge_pid_exist(get_pid_name)){
		remove("/var/server.lock");
		remove("/var/internet.lock");
		StartNetServer();
	}
}
/*******************************************************
��������: �����¼�
����: ��
����ֵ: ��
********************************************************/
void Net_work(void){
	if(judge_pid_exist(get_pid_name)){
		remove("/var/server.lock");
		remove("/var/internet.lock");
		StartNetServer();
	}
	smartConifgLog("smart_start\n");
	if(!checkInternetFile()){
		smartConifgLog("startSmartConfig checkInternetFile failed\n");
		return;
	}
	if(GetRecordeLive()==PAUSE_E){
		disable_gpio();
		startSmartConfig(create_event_system_voices,enable_gpio);
	}
	smartConifgLog("startSmartConfig Net_work ok\n");
}
/*******************************************************
��������: ����һ��ϵͳ�����¼�������ϵͳ����
����: sys_voices ϵͳ�����	
����ֵ: ��
********************************************************/
void create_event_system_voices(int sys_voices)
{
	playsysvoicesLog("playsys_start\n");
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	add_event_msg(NULL,sys_voices,SYS_VOICES_EVENT);
	playsysvoicesLog("playsys voices end \n");
}
//�ػ������ļ���������
void CloseSystemWork(void){
	SaveSystemPlayNum();
	DelSdcardMp3file(MP3_SDPATH);
}
/*******************************************************
��������: ϵͳ���¼�������
����: sys_voices ϵͳ�����
����ֵ: ��
********************************************************/
void handle_event_system_voices(int sys_voices){
	playsysvoicesLog("playsys voices handle \n");
//----------------------ϵͳ�й�-----------------------------------------------------
	switch(sys_voices){
		case END_SYS_VOICES_PLAY:					//������
#ifdef PALY_URL_SD
			pool_add_task(CloseSystemWork,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
			CloseSystemSignToaliyun();			//���͹ػ��źŸ�����
			play_sys_tices_voices(END_SYS_VOICES);			
			break;
		case TULING_WINT_PLAY:						//���Ե�	
#ifdef QITUTU_SHI
			pool_add_task(Led_System_vigue_open,NULL);
#endif
			play_sys_tices_voices(TULING_WINT);
			break;
		case LOW_BATTERY_PLAY:						//�͵�ػ�
#ifdef PALY_URL_SD
			pool_add_task(CloseSystemWork,NULL);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
			play_sys_tices_voices(LOW_BATTERY);			
			break;
		case RESET_HOST_V_PLAY:						//�ָ���������
			play_sys_tices_voices(RESET_HOST_V);
			break;
//----------------------�����й�-----------------------------------------------------
		case REQUEST_FAILED_PLAY:					//�������������������ʧ��
			play_sys_tices_voices(REQUEST_FAILED);
			break;
		case UPDATA_END_PLAY:						//���¹̼�����
			play_sys_tices_voices(UPDATA_END);
			//system("sleep 8 && reboot &");
			//clean_resources();
			break;
		case 7:
			play_sys_tices_voices(REQUEST_FAILED);
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
			led_left_right(left,closeled);
			led_left_right(right,closeled);
			play_sys_tices_voices(START_INTERNET);
			break;
		case SMART_CONFIG_OK_PLAY:		//��������ɹ�
			play_sys_tices_voices(YES_REAVWIFI);
			break;
		case CONNECT_OK_PLAY:			//���ӳɹ�
			play_sys_tices_voices(LINK_SUCCESS);
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
			enable_gpio();
			break;
		case CONNET_CHECK_PLAY:			//��������Ƿ����
			play_sys_tices_voices(CHECK_INTERNET);
			break;
//----------------------�Խ��й�-----------------------------------------------------
		case SEND_OK_PLAY:			//��������Ƿ����
			play_sys_tices_voices(SEND_OK);
			break;
		case SEND_ERROR_PLAY:			//��������Ƿ����
			play_sys_tices_voices(SEND_ERROR);
			break;
	}
	playsysvoicesLog("playsys voices end \n");
}
#ifdef SPEEK_VOICES
void CreateSpeekEvent(const char *filename){
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(filename)+1);
	if(TXT){
		sprintf(TXT,"%s",filename);
		add_event_msg(TXT,0,SPEEK_VOICES_EVENT);
	}
}
#endif

#ifdef SPEEK_VOICES
static int file_len=0;
static FILE *savefilefp=NULL;
static void shortVoicesClean(void)
{
	if(savefilefp!=NULL){
		fclose(savefilefp);
		savefilefp=NULL;
	}
}
/*******************************************************
��������: ���������͵��ļ��������ֵ����ص���
����:	��
����ֵ: 0 �����ɹ� -1 ����ʧ��
********************************************************/
static int create_recorder_file(void)
{
	if((savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL){
		perror("open send file failed \n");
		return -1;
	}
	file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,savefilefp);
	DEBUG_EVENT("create save file \n");
	return 0;
}
/*******************************************************
��������: ֹͣ¼�����������ݷ��͸�app�û�
����:
����ֵ:
********************************************************/
static void stop_recorder_tosend_file(void)
{
	char filepath[64];
	pcmwavhdr.size_8 = (file_len+36);
	pcmwavhdr.data_size = file_len;
	if(savefilefp==NULL){
		return;
	}
	DEBUG_EVENT("stop_recorder_tosend_file : fseek file \n");
	fseek(savefilefp,0,SEEK_SET);
	fwrite(&pcmwavhdr,1,WAV_HEAD,savefilefp);
	DEBUG_EVENT("stop_recorder_tosend_file : shortVoicesClean \n");
	shortVoicesClean();
	memset(filepath,0,64);
	time_t t;
	t = time(NULL);
	DEBUG_EVENT("stop_recorder_tosend_file : sprintf \n");
#ifdef CACHE_SDCARD
	sprintf(filepath,"%s%s%d%s",sysMes.sd_path,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif
	DEBUG_EVENT("stop_recorder_tosend_file : WavToAmr8kFile \n");
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath))
	{
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("stop save file \n");
	uploadVoicesToaliyun(filepath);
	create_event_system_voices(SEND_OK_PLAY);
	//remove(SAVE_WAV_VOICES_DATA);
}
/*******************************************************
��������:�����Խ��������º͵����¼�
����:state 0 ����  1 ����
����ֵ:
********************************************************/
void create_event_voices_key(unsigned int state)
{	
	int endtime;
	time_t t;
	if(state==0){	//����
		sysMes.Starttime=time(&t);
	}else{			//����
		endtime=time(&t);
		if((endtime - sysMes.Starttime)<2){		//ʱ��̫��
			pause_record_audio();
			shortVoicesClean();
			return ;
		}else if((endtime - sysMes.Starttime)>10){		//ʱ��̫��
			pause_record_audio();
			shortVoicesClean();
			return ;
		}
	}
	if(GetRecordeLive() ==PLAY_URL_E){		//��ϲ�������
		CleanUrlEvent();
		return;
	}
	DEBUG_EVENT("create_event_voices_key ...\n");
	add_event_msg(NULL,state,TALK_EVENT_EVENT);
}
/*******************************************************
��������:�������¼�
����:
����ֵ:
********************************************************/
void handle_voices_key_event(unsigned int state)
{
	int i;
	if(state==0&&GetRecordeLive() ==PAUSE_E){	//����
		DEBUG_EVENT("handle_voices_key_event : state(%d)...\n",state);
		start_event_play_wav();
		if(create_recorder_file())		//������Ƶ�ļ��ڵ㣬�����뵽������
		{
			pause_record_audio();
		}
		start_event_talk_message();
	}else if(state==1){			//����
		DEBUG_EVENT("handle_voices_key_event : state(%d)\n",state);
		pause_record_audio();
		usleep(100);
		/********************************************
		ע :��Ҫд�ļ�ͷ��Ϣ 
		*********************************************/
		stop_recorder_tosend_file();
	}
}
/*******************************************************
��������:������������ (��Ҫ����Ƶ����ѹ������)
����:  voices_data ԭʼ��Ƶ����  size ԭʼ��Ƶ���ݴ�С
����ֵ:
********************************************************/
void save_recorder_voices(const char *voices_data,int size)
{
	int i=0;
	int endtime;
	time_t t;
	if(savefilefp!=NULL)
	{
		endtime=time(&t);
		if((endtime - sysMes.Starttime)>60)//
		{
			pause_record_audio();
		}
#if 0	//������
		for(i=2; i<size; i+=4)		//˫��������ת�ɵ���������
		{
			fwrite(voices_data+i,2,1,savefilefp);
		}
#else	//������
		for(i=0; i<size; i+=4){
			fwrite(voices_data+i,2,1,savefilefp);
		}
#endif
		i /=2; 
		file_len +=i;
	}
	else{
		pause_record_audio();
	}
}
#endif

#ifdef LOCAL_MP3
static void *waitLoadMusicList(void *arg){
	sleep(15);
	SysOnloadMusicList((const char *)TF_SYS_PATH,(const char *)TF_MP3_PATH,(const char *)TF_STORY_PATH,(const char *)TF_ENGLISH_PATH,(const char *)TF_GUOXUE_PATH);
	CacheNetServer();	//����������
	return;
} 
#endif
/******************************************************************
��ʼ��8960��ƵоƬ������8K¼���Ͳ���˫��ģʽ
*******************************************************************/
void init_wm8960_voices(void)
{
	init_7620_gpio();
	__init_wm8960_voices();
	//disable_gpio();
#ifndef TEST_SDK
	playsysvoices(START_SYS_VOICES);//����������
#endif
	initStream(ack_playCtr,WritePcmData,i2s_start_play,GetVol);
	init_stdvoices_pthread();
	init_record_pthread();
#ifdef TEST_SDK
	enable_gpio();
#endif

#ifdef LOCAL_MP3
	InitMusicList();
	pool_add_task(waitLoadMusicList, NULL);	//��ֹT��������
#endif
}
void clean_main_voices(void)
{
	exit_record_pthread();
	i2s_destory_voices();
	clean_7620_gpio();
	clean_stdvoices_pthread();
	cleanStream();
}

