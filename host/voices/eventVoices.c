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

#include "config.h"

#define START_SPEEK_E	START_SPEEK_VOICES 		  	//¼��������ʶ��
#define PAUSE_E			RECODE_PAUSE 				//¼������
#define PLAY_WAV_E		PLAY_WAV					//����wavԭʼ������
#define END_SPEEK_E		END_SPEEK_VOICES			//����¼��
#define PLAY_URL_E		PLAY_URL					//����url����

//#define DBG_EVENT
#ifdef DBG_EVENT  
#define DEBUG_EVENT(fmt, args...) printf("event : " fmt, ## args)  
#else   
#define DEBUG_EVENT(fmt, args...) { }  
#endif //end DBG_EVENT

extern void enable_gpio(void);
extern void DelSdcardMp3file(char * sdpath);

SysMessage sysMes;
static int playTextNum=0;
/*******************************************************
��������: ��������URL��ַ
����: url URL��ַ	
����ֵ: ��
********************************************************/
static void CreateUrlEvent(const void *data){
#ifdef LOG_MP3PLAY
	playurlLog("url_start\n");
	playurlLog(VERSION);	//�汾ʱ��
#endif
	if(getEventNum()>0){
		DEBUG_EVENT("CreateUrlEvent num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeLive() == PLAY_WAV_E){
		exitqttsPlay();
	}
	add_event_msg(data,0,URL_VOICES_EVENT);
#ifdef LOG_MP3PLAY
	playurlLog("add ok\n");
#endif
}
/*******************************************************
��������: ���ű��ص�ַ
����: localpath ����MP3���ŵ�ַ	
����ֵ: ��
********************************************************/
static void CreateLocalMp3(char *localpath)
{
	if(getEventNum()>0)
	{
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
#ifdef LOCAL_MP3
static int playMp3LastNum;
static void PlayLocal(unsigned char str,char *path, unsigned char Mute)
{
	static int playMp3Num=0;
	char buf[64]={0};
	char filename[64]={0};
	int ret=0;
	get_paly_num(&playMp3Num,str);
#if 1
	if(++playMp3Num>127)
		playMp3Num=1;
	if(Mute==PLAY_LAST){
		playMp3Num -=2;
		if(playMp3Num<=0){
			playMp3Num=playMp3LastNum;
		}
	}
#endif
	snprintf(buf,64,"%s%s",TF_SYS_PATH,path);
	ret=get_mp3filenmae(buf,filename,playMp3Num);
	if(ret == -1){
		playMp3LastNum=playMp3Num-1;
		playMp3Num=1;
	}else if(ret == -2){
		return;
	}
	snprintf(buf,64,"%s%s%s",TF_SYS_PATH,path,filename);
	CreateLocalMp3(buf);
	set_paly_num(playMp3Num,str);
	sysMes.localplayname=str;
}
#endif
/*******************************************************
��������: ����MP3
����: play ����MP3�������� �� URL��ַ
����ֵ: ��
********************************************************/
void createPlayEvent(const void *play,unsigned char Mute)
{
	char buf[64]={0};
	char filename[64]={0};
	int ret=0;
	if(!strcmp((const char *)play,"testmp3")){
		if(++playTextNum>127)
			playTextNum=0;
		snprintf(buf,64,"%s%s",TF_SYS_PATH,TF_TEST_PATH);
		ret=get_mp3filenmae(buf,filename,playTextNum);
		if(ret == -1){
			playTextNum=1;
		}else if(ret == -2){
			return;
		}
		snprintf(buf,64,"%s%s%s",TF_SYS_PATH,TF_TEST_PATH,filename);
		CreateLocalMp3(buf);
		sysMes.localplayname=testmp3;
	}
#ifdef LOCAL_MP3
	else if(!strcmp((const char *)play,"mp3")){
		PlayLocal(mp3,TF_MP3_PATH,Mute);
	}
	else if(!strcmp((const char *)play,"story")){
		PlayLocal(story,TF_STORY_PATH,Mute);
	}
	else if(!strcmp((const char *)play,"english")){
		PlayLocal(english,TF_ENGLISH_PATH,Mute);
	}
#endif
	else{
		CreateUrlEvent(play);
	}
}
/*******************************************************
��������: ����URL�¼�
����: ��
����ֵ: ��
********************************************************/
void CleanUrlEvent(void)
{
	sysMes.localplayname=0;
	add_event_msg(NULL,0,SET_RATE_EVENT);
}
/*******************************************************
��������: QTTS�¼�
����: txt QTTS�ı� type :0---GBK 1----UTF8
����ֵ: ��
********************************************************/
void QttsPlayEvent(char *txt,int type)
{
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}
	char *TXT= (char *)calloc(1,strlen(txt)+1);
	if(TXT){
		sprintf(TXT,"%s",txt);
		add_event_msg(TXT,type,QTTS_PLAY_EVENT);
	}
}

/*******************************************************
��������: �Ự�¼�
����: ��
����ֵ: ��
********************************************************/
static int staittime=0;
void down_voices_sign(void)
{
	time_t t;
	if(time(&t)-staittime<1)
		return;
	if(GetRecordeLive() ==START_SPEEK_VOICES||GetRecordeLive() ==END_SPEEK_VOICES){
		DEBUG_EVENT("check_recorde_state ...\n");
		return;
	}
	if(GetRecordeLive()==PLAY_WAV_E){
		exitqttsPlay();
	}else if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
	}else{
		start_event_std();
	}
	staittime=time(&t);
}
/*******************************************************
��������: �鿴NetManger�����Ƿ����pid��
����: pid_name	��������
����ֵ: ��
********************************************************/
static int get_pid_name(char *pid_name)
{
	if(!strcmp(pid_name,"NetManger")){
		return 0;
	}
	return -1;
}
/*******************************************************
��������: �����¼�
����: ��
����ֵ: ��
********************************************************/
void Net_work(void)
{
	if(judge_pid_exist(get_pid_name)){
		remove("/var/server.lock");
		remove("/var/internet.lock");
		system("NetManger &");
		sleep(1);
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
/*******************************************************
��������: ϵͳ���¼�������
����: sys_voices ϵͳ�����
����ֵ: ��
********************************************************/
void handle_event_system_voices(int sys_voices)
{
	playsysvoicesLog("playsys voices handle \n");
//----------------------ϵͳ�й�-----------------------------------------------------
	if(sys_voices==1)							//������
	{
#ifdef PALY_URL_SD
		pool_add_task(DelSdcardMp3file,MP3_SDPATH);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
		play_sys_tices_voices(END_SYS_VOICES);
		set_paly_sys_num();
	}
	else if(sys_voices==2)						//���Ե�
	{
		play_sys_tices_voices(TULING_WINT);
	}
	else if(sys_voices==3)						//�͵�ػ�
	{
#ifdef PALY_URL_SD
		pool_add_task(DelSdcardMp3file,MP3_SDPATH);//�ػ�ɾ������ʱ�䲻�õ��ļ�
#endif
		play_sys_tices_voices(LOW_BATTERY);
		set_paly_sys_num();
	}
	else if(sys_voices==4)						//�ָ���������
	{
		play_sys_tices_voices(RESET_HOST_V);
	}
//----------------------�����й�-----------------------------------------------------
	else if(sys_voices==5)						//�������������������ʧ��
	{
		play_sys_tices_voices(REQUEST_FAILED);
	}
	else if(sys_voices==6){
		play_sys_tices_voices(UPDATA_END);		//���¹̼�����
		//system("sleep 8 && reboot &");
		//clean_resources();
	}
	else if(sys_voices==7){
		play_sys_tices_voices(REQUEST_FAILED);
	}
	else if(sys_voices==8)
	{
		play_sys_tices_voices(REQUEST_FAILED);
	}
//----------------------�����й�-----------------------------------------------------
	else if(sys_voices==CONNET_ING)				//�������ӣ����Ե�
	{
		play_sys_tices_voices(CHANGE_NETWORK);
	}
	else if(sys_voices==START_SMARTCONFIG)		//��������
	{
		pool_add_task(Led_vigue_open,NULL);
		play_sys_tices_voices(START_INTERNET);
	}
	else if(sys_voices==SMART_CONFIG_OK)		//��������ɹ�
	{
		play_sys_tices_voices(YES_REAVWIFI);
	}
	else if(sys_voices==CONNECT_OK)			//���ӳɹ�
	{
		char buf[128]={0};
		play_sys_tices_voices(LINK_SUCCESS);
		char *wifi = nvram_bufget(RT2860_NVRAM, "ApCliSsid");
		if(strlen(wifi)>0){
			snprintf(buf,128,"%s%s","������ wifi ",wifi);
			QttsPlayEvent(buf,0);
		}
		Led_vigue_close();
		enable_gpio();
	}
	else if(sys_voices==NOT_FIND_WIFI)			//û��ɨ�赽wifi
	{
		play_sys_tices_voices(NO_WIFI);
		enable_gpio();
	}
	else if(sys_voices==SMART_CONFIG_FAILED)	//û���յ��û����͵�wifi
	{	
		play_sys_tices_voices(NOT_REAVWIFI);
	}
	else if(sys_voices==NOT_NETWORK)			//û�����ӳɹ�
	{
		play_sys_tices_voices(NO_NETWORK_VOICES);
		pool_add_task(Led_vigue_open,NULL);
		enable_gpio();
	}
	else if(sys_voices==CONNET_CHECK)			//��������Ƿ����
	{
		play_sys_tices_voices(CHECK_INTERNET);
	}
	usleep(1000);
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
��������:�����Խ��������º͵����¼�
����:state 0 ����  1 ����
����ֵ:
********************************************************/
void create_event_voices_key(unsigned int state)
{	
	int endtime;
	time_t t;
	if(state==0)
	{
		sysMes.Starttime=time(&t);		
	}else{
		endtime=time(&t);
		if((endtime - sysMes.Starttime)<2)
		{
			pause_record_audio();
			shortVoicesClean();
			return ;
		}else if((endtime - sysMes.Starttime)>10){
			pause_record_audio();
			shortVoicesClean();
			return ;
		}
	}
	if(GetRecordeLive() ==PLAY_URL_E){
		CleanUrlEvent();
		return;
	}
	printf("create_event_voices_key ...\n");
	add_event_msg(NULL,state,TALK_EVENT_EVENT);
}

/*******************************************************
��������: ���������͵��ļ��������ֵ����ص���
����:	��
����ֵ: 0 �����ɹ� -1 ����ʧ��
********************************************************/
static int create_recorder_file(void)
{
	printf("create_recorder_file ...\n");
	if((savefilefp= fopen(SAVE_WAV_VOICES_DATA,"w+"))==NULL)
	{
		perror("open send file failed \n");
		return -1;
	}
	file_len=0;
	fwrite((char *)&pcmwavhdr,WAV_HEAD,1,savefilefp);
	printf("create save file \n");

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
	printf("stop_recorder_tosend_file : file_len(%d)\n",file_len);
	if(savefilefp==NULL){
		return;
	}
	fseek(savefilefp,0,SEEK_SET);
	fwrite(&pcmwavhdr,1,WAV_HEAD,savefilefp);
	
	shortVoicesClean();
	memset(filepath,0,64);
	time_t t;
	t = time(NULL);
	
#ifdef CACHE_SDCARD	
	sprintf(filepath,"%s%s%d%s",sysMes.sd_path,CACHE_WAV_PATH,(unsigned int)t,".amr");
#else
	sprintf(filepath,"%s%d%s",CACHE_WAV_PATH,(unsigned int)t,".amr");
#endif	
	
	DEBUG_EVENT("stop save file \n");
		
	if(WavToAmr8kFile(SAVE_WAV_VOICES_DATA,filepath))
	{
		DEBUG_EVENT("enc_wav_amr_file failed");
		return;
	}
	DEBUG_EVENT("start send file \n");
	uploadVoicesToaliyun(filepath);
	QttsPlayEvent("���ͳɹ���",QTTS_SYS);
	//remove(SAVE_WAV_VOICES_DATA);
}

/*******************************************************
��������:�������¼�
����:
����ֵ:
********************************************************/
void handle_voices_key_event(unsigned int state)
{
	int i;
	if(state==0)
	{
		DEBUG_EVENT("handle_voices_key_event : state(%d)...\n",state);
		if(create_recorder_file())		//������Ƶ�ļ��ڵ㣬�����뵽������
		{
			pause_record_audio();
		}
		start_event_talk_message();
		/********************************************
		ע :��Ҫд�ļ�ͷ��Ϣ 
		*********************************************/	
	}else {
		DEBUG_EVENT("handle_voices_key_event : state(%d)\n",state);
		pause_record_audio();
		//usleep(500000);		//tang : 2015-12-3 for yan chang shi jian
		usleep(100);
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
			DEBUG_EVENT("=============================================\n");
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

/******************************************************************
��ʼ��8960��ƵоƬ������8K¼���Ͳ���˫��ģʽ
*******************************************************************/
void init_wm8960_voices(void)
{
	init_7620_gpio();
	__init_wm8960_voices();
	//disable_gpio();
#ifndef TEST_SDK
	play_sys_tices_voices(START_SYS_VOICES);//����������
#endif
	initStream(ack_playCtr,WritePcmData,i2s_start_play,GetVol);
	init_stdvoices_pthread();
	init_record_pthread();
#ifdef TEST_SDK
	enable_gpio();
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

