#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "base/pool.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "gpio_7620.h"
#include "../studyvoices/qtts_qisc.h"
#include "config.h"
#if defined(HUASHANG_JIAOYU)
#include "huashangMusic.h"
#endif
static RecoderVoices_t *RV=NULL;

//Ĭ����Ƶͷ������
struct wave_pcm_hdr pcmwavhdr = {
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	RECODE_RATE,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

#if defined(MY_HTTP_REQ)	//��¼�Ƶ�8k����ת����16k����
static void pcmVoice8kTo16k(const char *inputdata,char *outputdata,int inputLen){
	int pos=0,npos=0;
	for(pos=0;pos<inputLen;pos+=2){
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
	}
}
#else					//��8k����ת����16k��������д�뵽�ļ�����
static int PcmVoice8kTo16k_File(const char *inputdata,const char *outfilename,int inputLen){
	FILE *fp=NULL;
	fp = fopen(outfilename,"w+");
	if(fp==NULL){
		return -1;
	}
	int pos=0,npos=0;
	for(pos=0;pos<inputLen;pos+=2){
		fwrite(inputdata+pos,1,2,fp);
		fwrite(inputdata+pos,1,2,fp);
	}
	fclose(fp);
	return 0;
}	
#endif
unsigned int GetCurrentEventNums(void){
	return RV->CurrentuploadEventNums;
}
unsigned int updateCurrentEventNums(void){
	struct timeval starttime;
    gettimeofday(&starttime,0); 
	RV->CurrentuploadEventNums=(unsigned int)starttime.tv_sec/2+starttime.tv_usec;
	return RV->CurrentuploadEventNums;
}

/*****************************************************
*��ȡ״̬
*****************************************************/
int GetRecordeVoices_PthreadState(void){
	return RV->recorde_live;
}

/*****************************************************
*����״̬
*****************************************************/
static void SetRecordeVoices_PthreadState(unsigned char state){
	printf("%s: recorde_live %d  %d \n",__func__,RV->recorde_live,state);
	RV->recorde_live=state;
}

/*****************************************************
*��������ʶ��״̬
*****************************************************/
void StartTuling_RecordeVoices(void){	
	SetRecordeVoices_PthreadState(START_SPEEK_VOICES);
}
/*****************************************************
*��������ʶ��״̬
*****************************************************/
void StopTuling_RecordeVoices(void){
	if(GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES)
		SetRecordeVoices_PthreadState(END_SPEEK_VOICES);
}
/*****************************************************
*���벥��wavԭʼ����״̬
*****************************************************/
void start_event_play_wav(void){
	SetRecordeVoices_PthreadState(PLAY_WAV);
}
void start_speek_wait(void){
	SetRecordeVoices_PthreadState(SPEEK_WAIT);
}
void start_play_tuling(void){
	SetRecordeVoices_PthreadState(PLAY_DING_VOICES);
}
/*****************************************************
*���벥��Mp3music״̬
*****************************************************/
void start_event_play_Mp3music(void){
	SetRecordeVoices_PthreadState(PLAY_MP3_MUSIC);
}
//��������
void start_event_play_soundMix(void){
	SetRecordeVoices_PthreadState(SOUND_MIX_PLAY);
}

/*****************************************************
*��ͣ¼��״̬
*****************************************************/
void pause_record_audio(void){
	Show_waitCtrlPicture();
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
}

/*****************************************************
*¼�ƶ���Ϣ״̬
******************************************************/
void start_event_talk_message(void){
	SetRecordeVoices_PthreadState(START_TAIK_MESSAGE);
}

static void *uploadPcmPthread(void *arg){
	HandlerText_t *handText = (HandlerText_t *)arg;
#if defined(MY_HTTP_REQ)
	ReqTulingServer(handText,"pcm","0",RECODE_RATE*2);
#else
	PcmVoice8kTo16k_File(RV->buf_voices,"pcm16k.cache",RV->len_voices);
	ReqTulingServer((const char *)"pcm16k.cache",RV->len_voices*2,"pcm","0",RECODE_RATE*2);
#endif
	RV->uploadState = END_UPLOAD;
	return NULL;
}
/****************************************
@��������:	��ʼ�ϴ�������������
@����:	��
*****************************************/
static void Start_uploadVoicesData(void){
	if(RV->uploadState==START_UPLOAD){
		return ;
	}
	RV->uploadState = START_UPLOAD;
	start_play_tuling();	//���õ�ǰ����״̬Ϊ : �����ϴ�����
#if defined(HUASHANG_JIAOYU)

#else
	Create_PlayImportVoices(TULING_WAIT_VOICES);
	usleep(200);
#endif	
	HandlerText_t *up = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(up){
		up->dataSize =RV->len_voices*2;
		up->data= (char *)calloc(1,up->dataSize+2);
		if(up->data){
			up->EventNums = GetCurrentEventNums();
			pcmVoice8kTo16k(RV->buf_voices+WAV_HEAD,up->data,RV->len_voices);
			pthread_create_attr(uploadPcmPthread,(void * )up);
		}
	}
}
/****************************************
@��������:	����ɼ����ݺ���
@����:	data �ɼ���������  size ���ݴ�С
*****************************************/
static void Save_VoicesPackt(const char *data,int size){
	int i;
	if(data != NULL){
		if((RV->len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){//����5�����Ƶ����
			test_Save_VoicesPackt_function_log((const char *)"STD_RECODE_SIZE is >5s",RV->len_voices);
			goto exit1;
		}
#if 0		
		//ֻ�������������ݣ�������û�У���Ҫ����2��3��������
		for(i=2; i<size; i+=4){
			//˫��������ת�ɵ���������
			memcpy(RV->buf_voices+WAV_HEAD+RV->len_voices,data+i,2);
			RV->len_voices += 2;
		}
#else
		for(i=0; i<size; i+=4){
			memcpy(RV->buf_voices+WAV_HEAD+RV->len_voices,data+i,2);
			RV->len_voices += 2;
		}
#endif	//end 0
	}else{
		if(RV->len_voices > VOICES_MIN)	//����0.5s ��Ƶ�����ϴ������������п�ʼʶ��  13200
		{
#ifdef DATOU_JIANG	//���ϴ����̵�����˸��
			led_lr_oc(openled);
#endif
			test_Save_VoicesPackt_function_log((const char *)"start upload",GetRecordeVoices_PthreadState());
			Start_uploadVoicesData();		//��ʼ�ϴ�����
			goto exit0;
		}
		else if(RV->len_voices < VOICES_ERR){			//
			test_Save_VoicesPackt_function_log((const char *)"< VOICES_ERR",GetRecordeVoices_PthreadState());
			goto exit1;	//�󴥷�
		}
		else{	//VOICES_ERR --->VOICES_MIN �������Ƶ���϶�Ϊ��Ч��Ƶ
			//Create_PlaySystemEventVoices(AI_KEY_TALK_ERROR);
			test_Save_VoicesPackt_function_log((const char *)"< VOICES_MIN",GetRecordeVoices_PthreadState());
			goto exit1;
		}
	}
	return ;
exit1:
	pause_record_audio();
exit0:
	memset(RV->buf_voices,0,RV->len_voices);
	RV->len_voices = 0;
	return ;
}
//�ػ�������ǰϵͳʱ�䷢�͸�MCU
int SetMucClose_Time(unsigned char closeTime){
	time_t timep;
	struct tm *p;
	time(&timep);
	char syscloseTime[64]={0};
	SocSendMenu(3,0);
	usleep(100*1000);
	p=localtime(&timep); /*ȡ�õ���ʱ��*/
	if((p->tm_hour)+8>=24){
		p->tm_hour=(p->tm_hour)+8-24;
	}else{
		p->tm_hour=(p->tm_hour)+8;
	}
	sprintf(syscloseTime,"%d:%d",(p->tm_hour),(p->tm_min)+closeTime);
	printf("SetMucClose_Time : %s\n",syscloseTime);
	SocSendMenu(2,syscloseTime);
	return 0;
}
/*******************************************************
��������: ¼���߳� ���Ե�ǰ������⣬ɸѡ��Ч��Ƶ
����:   	��
����ֵ: 
********************************************************/
static void *PthreadRecordVoices(void *arg){
	char *pBuf;
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
	time_t t;
	int endtime,starttime;
	starttime=time(&t);
	endtime=time(&t);
	while(GetRecordeVoices_PthreadState()!=RECODE_STOP){
#ifdef CLOCESYSTEM
		endtime=time(&t);
		if((endtime-starttime)>ERRORTIME){	//����ʱ��û�л�ȡ����ʱ�䣬����ʱ������
			starttime=time(&t);
		}else{
			if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
				
				if((endtime-starttime)==LONG_TIME_NOT_USER_MUTE_VOICES){	//10s ֮�󣬲��ùر���Ƶ
					printf("%s: MUTE wm8960====%d===========\n",__func__,endtime-starttime);
					Mute_voices(MUTE);
				}
				if((endtime-starttime)>SYSTEMOUTSIGN){		//��һ�γ�ʱ�䲻�����¼�����ر�
					SetRecordeVoices_PthreadState(TIME_SIGN);
				}
			}else{
				starttime=time(&t);
			}
#if 1		
			if(sysMes.enableCountStarttime==ENABLE_auto_count_time&&(endtime-sysMes.auto_count_starttime)>TIME_OUT_NOT_USER_FOR_CLOSE){
				SetRecordeVoices_PthreadState(PLAY_OUT);
			}
#endif			
		}
#endif
		switch(GetRecordeVoices_PthreadState()){
			case START_SPEEK_VOICES:	//�Ự¼��
				pBuf = I2sGetvoicesData();
				Save_VoicesPackt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:		//�Ự¼������
				Save_VoicesPackt(NULL, 0);	//���Ϳ���Ƶ��Ϊһ����Ƶ�Ľ�����־
				break;
			case START_TAIK_MESSAGE:	//�Խ�¼��
				pBuf = I2sGetvoicesData();
				SaveRecorderVoices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);			//�����п���ĸо�
				break;
#ifdef CLOCESYSTEM
			case TIME_SIGN:				//��ʾ��Ϣ�ܾ���
				systemTimeLog("time out for play music");
				Create_PlaySystemEventVoices(MIN_10_NOT_USER_WARN);
				sleep(1);
				break;
				
			case PLAY_OUT:				//���ų�ʱ�˳�
				printf("-----------------------\n close system --------------------\n");
				systemTimeLog("close system");
				SetMucClose_Time(1);	//����һ���Ӻ�ػ�
				sysMes.enableCountStarttime=DISABLE_count_time;
				sysMes.auto_count_starttime=time(&t);
				SetRecordeVoices_PthreadState(PLAY_MP3_MUSIC);
				break;
#endif
			case PLAY_DING_VOICES:
				usleep(50000);
				break;
			default:
				I2sGetvoicesData();		//Ĭ��״̬�����Ƶ
				usleep(50000);
				break;
		}
	}
	SetRecordeVoices_PthreadState(RECODE_EXIT_FINNISH);
	DEBUG_VOICES("handle record voices exit\n");
	test_Save_VoicesPackt_function_log("recorde pthread exit",0);
	return NULL;
}

/*****************************************************
����¼�������߳�
*****************************************************/
void InitRecord_VoicesPthread(void){	
	RV =(RecoderVoices_t *)calloc(1,sizeof(RecoderVoices_t));
	if(RV==NULL){
		perror("calloc RecoderVoices_t memory failed");
        exit(-1);
	}
#if defined(HUASHANG_JIAOYU)
	char playFile[24]={0};
	srand((unsigned)time(NULL));
	int i=(rand()%4)+1;
	if(i>=4)
		i=4;
	snprintf(playFile,24,"qtts/start_%d.amr",i);
	PlayImportVoices(playFile,0);
#else
	PlayImportVoices(START_SYS_VOICES,0);//����������
#endif

#ifdef TEST_MIC
	if(pthread_create_attr(TestRecordePlay,NULL)){
		perror("create test recorde play failed!");
		exit(-1);
	}
#else
	if(pthread_create_attr(PthreadRecordVoices,NULL)){
  	  	perror("create handle record voices failed!");
        	exit(-1);
	}
#endif	//end TEST_MIC
	usleep(300);
}

/*****************************************************
�˳�¼��(�˳�ϵͳ)
*****************************************************/
void ExitRecord_Voicespthread(void){
	unsigned char timeout=0;
	SetRecordeVoices_PthreadState(RECODE_STOP);
	while(GetRecordeVoices_PthreadState()!=RECODE_EXIT_FINNISH){
		usleep(100000);
		if(GetRecordeVoices_PthreadState()!=RECODE_EXIT_FINNISH)
			SetRecordeVoices_PthreadState(RECODE_STOP);
		if(timeout++>50)
			break;
	}
	DEBUG_VOICES("exit record pthread success (%d)\n",GetRecordeVoices_PthreadState());

	//���̡߳��������硢¼���������̡߳��¼������̡߳������̡߳������߳�
}
