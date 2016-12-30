#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "host/voices/wm8960i2s.h"
#include "tlvoice.h"
#include "qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../sdcard/musicList.h"

static char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
//static char *key = "21868a0cd8806ee2ba5eab6181f0add7";//tang : change 2016.4.26 for from chang key 

struct eventMsg{
	int  len:24,type:8;
};
static WorkQueue *evMsg;

#ifdef TEST_SAVE_MP3
static int test_mp3file=0;
static  void test_save_mp3file(char *mp3_data,int size)
{
	FILE * fp;	//tang :change 2016-2-22 add mp3 
	char buf[32]={0};
	sprintf(buf,"%s%d%s","/mnt/test/text",test_mp3file++,".amr");
	fp = fopen(buf,"w+");
	if(fp ==NULL){
		perror("test_save_mp3file: fopen failed ");
		return ;
	}
	fwrite(mp3_data,size,1,fp);
	fclose(fp);
}
#endif

/******************************************
@��������:	�ı�������
@����:	text �ı�����
******************************************/
static void handle_text(char *text)
{
	tolkLog("tolk_start\n");
	int ret=0;
	//���ؼ��ʣ�������Ӧ�Ļش�
	if(check_text_cmd(text)){
		//pause_record_audio();------����check_text_cmd���������
		return ;
	}
	tolkLog("tolk handle qtts start\n");
	ret = PlayQttsText(text,QTTS_UTF8);
	if(ret == 10202){
		//��������������
		playsysvoices(REQUEST_FAILED);
		//startServiceWifi();
	}
	tolkLog("tolk handle qtts end\n");
}
#define J_VOICES_1	1
#define J_TAIBEN_1	2
#define J_TAIBEN_2	3
#define J_TAIBEN_3	4
#define J_TAIBEN_4	5
#define J_TAIBEN_5	6
#define J_TAIBEN_6	7
#define J_TAIBEN_7	8
#define J_TAIBEN_8	9
#define J_TAIBEN_9	10
static void TaiBenToTulingJsonEr(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case J_VOICES_1:
			playsysvoices(ERROR_40002);
			break;
		case J_TAIBEN_1:
			PlayQttsText("С���������˵������һ�׸衣",QTTS_GBK);
			break;
		case J_TAIBEN_2:
			PlayQttsText("С������������ң������˭��",QTTS_GBK);
			break;
		case J_TAIBEN_3:
			PlayQttsText("С������������ң�����������ô����",QTTS_GBK);
			break;
		case J_TAIBEN_4:
			PlayQttsText("С��������Ը��������죬˵һ˵���Ļ���",QTTS_GBK);
			break;
		case J_TAIBEN_5:
			PlayQttsText("С���������˵���������¡�",QTTS_GBK);
			break;
		case J_TAIBEN_6:
			PlayQttsText("С���������˵������Ц����",QTTS_GBK);
			break;
		case J_TAIBEN_7:
			PlayQttsText("С���ѿ��Ը���˵����������",QTTS_GBK);
			break;
		case J_TAIBEN_8:
			PlayQttsText("С������������ң�2+3+4+5���ڼ���",QTTS_GBK);
			break;
		case J_TAIBEN_9:
			PlayQttsText("С��������Զ���˵��������ա�",QTTS_GBK);
			break;
	}
}

/*******************************************
@��������:	json��������������
@����:	pMsg	����������
@		handle_jsion	����������
@		textString	�����������
@����ֵ:	0	�ɹ�	�����������Ǵ�����
***********************************************/
static int parseJson_string(const char * pMsg,void handle_jsion(char *textString))
{
	int err=-1;
	if(NULL == pMsg){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson){
       	return -1;
    }
    cJSON * pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint)
	{
		case 40001:
		case 40003:
		case 40004:
		case 40005:		
		case 40006:	
		case 40007:
		case 305000:
		case 302000:
		case 200000:
		case 40002:
#if 1
			TaiBenToTulingJsonEr();
#else
			playsysvoices(ERROR_40002);
#endif
			goto exit;
	}
#if 0	//�ı�
    pSub = cJSON_GetObjectItem(pJson, "text");		//���ؽ��
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=get text=%s=\n",pSub->valuestring);
	handle_jsion(pSub->valuestring);
#else	//����
	pSub = cJSON_GetObjectItem(pJson, "fileUrl");		//���ؽ��
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=get url=%s=\n",pSub->valuestring);
	//http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3
	//AddDownEvent("http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3",TULING_URL_MAIN);
	char *URL= (char *)calloc(1,strlen(pSub->valuestring)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	sprintf(URL,"%s",pSub->valuestring);
	AddDownEvent(URL,TULING_URL_MAIN);
#endif
	pSub = cJSON_GetObjectItem(pJson, "info");		//����ʶ��
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=upload=%s=\n",pSub->valuestring);
	err=0;
exit:
	cJSON_Delete(pJson);
	return err;
}

/*******************************************************
@��������:	�ϴ����ݵ�����������ȡ���ظ�
@����:	voicesdata �ϴ�����
@		len	�ϴ����ݴ�С
@		voices_type	��������
********************************************************/
#if 0
jmp_buf env; 

void recvSignal(int sig) {
	tulingLog("recvSignal signal 11",0);
    printf("received signal %d !!!\n",sig);
	siglongjmp(env,1); 
} 

void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	time_t t;
	int endtime=0,starttime=0;
	int resSize = 0, textSize=0, err=0;
	char *result=NULL, *text=NULL, *audio=NULL;
	starttime=time(&t);
	start_event_play_wav();//��ͣ¼��
	audio = base64_encode(voicesdata, len);
	if(!audio){
		create_event_system_voices(5);
		goto exit1;
	}
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	
	tulingLog("tuling_start",err);
	int r = sigsetjmp(env,1);  //����һ��������
	tulingLog("tuling env start...",err);
	if(r== 0){
		signal(SIGSEGV, recvSignal);  //���ֶδ���ص�����
		DEBUG_STD_MSG("set signal ok \n");
		///ͼ��������ϣ��ڲ����ܻᷢ������Ĵ��� 
		if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize)))
		{
			tulingLog("tl_req_voice error",err);
			if(err==5||err==6){
				endtime=time(&t);
				if((endtime-starttime)<15)//��������������
				{
					if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize))){
						DEBUG_STD_MSG("up voices data failed err =%d\n",err);
						if(err==5||err==6){
							create_event_system_voices(5);
							startServiceWifi();
							free(audio);
							goto exit1;
						}
					}
				}else{
					create_event_system_voices(5);
					startServiceWifi();
					free(audio);
					goto exit1;
				}
			}
		}
	}
	else{
		DEBUG_STD_MSG("jump this code bug!!\n");
		tulingLog("save up down",err);
		create_event_system_voices(5);
		free(audio);
		goto exit1;
	}
	tulingLog("tuling_end",err);
	free(audio);
	audio=NULL;
	if(result){
		free(result);
		result=NULL;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#else
#define SPEEK_1 	1
#define SPEEK_2 	2
#define SPEEK_3 	3
#define SPEEK_4 	4
#define JOKE_1		5
#define JOKE_2		6
#define JOKE_3		7
#define MUSIC_1		8
#define MUSIC_2		9
#define SONG_1		10
#define EDUCARE_1	11
#define SPEEK_5		12
#define EDUCARE_2	13
#define EDUCARE_3	14
#define EDUCARE_4	15
#define EDUCARE_5	16
#define EDUCARE_6	17
#define MUSIC_3		18
#define MUSIC_4		19
#define SONG_2		20
#define SONG_3		21
#define SONG_4		22
#define SONG_5		23
#define SONG_6		24
#define SONG_7		25
#define SONG_8		26
#define SONG_9		27
#define SONG_10		28
#define MUSIC_5		29
#define MUSIC_6		30
#define EDUCARE_7	31
#define SPEEK_6		32

static void TaiwanToTulingError(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (30.0*rand()/(RAND_MAX+1.0)));
	switch(i){
//---------------------------------�Դ�-------------------------------------------------------
	case SPEEK_1:
		QttsPlayEvent("�������룬�ٻش��������",QTTS_GBK);
		break;
	case SPEEK_2:
		QttsPlayEvent("��Ҳ��֪��Ӵ���㲻����Ұɡ�",QTTS_GBK);
		break;
	case SPEEK_3:
		QttsPlayEvent("�����������������治������Ҳ�ǰ����ա�",QTTS_GBK);
		break;
	case SPEEK_4:
		QttsPlayEvent("��Ҫ�����������������⡣",QTTS_GBK);
		break;
	case SPEEK_5:
		QttsPlayEvent("��Ҫ���ң�����˼���ء�",QTTS_GBK);
		break;
	case SPEEK_6:
		QttsPlayEvent("�������Ļ�����Ҫ���������߲��ܸ����������",QTTS_GBK);
		break;
//---------------------------------Ц��-------------------------------------------------------
	case JOKE_1:
		QttsPlayEvent("�����ʣ�үү��ˮţ��ɶ����?үү˵��ˮţ����ͨţ���Ĳ�࣬��ͬ������ϲ����ˮ���������˵���ޣ��Ҷ�������һ����ϲ������ɡ�",QTTS_GBK);
		break;
	case JOKE_2:
		QttsPlayEvent("���׷Ը�����˵���㵽������ȥȡΪ�ְֶ������·�������ϰ�����ҪǮ����͸���������Ϊ��̫С���ְֲ������Ǯ���š������뿪�󲻾ã��ֿ����ֻ����ˣ������߸���˵���ְ֣��������ϰ�˵�����ҳ�������ȥ�á�",QTTS_GBK);
		break;
	case JOKE_3:
		QttsPlayEvent("����˵�����裬�ҵ���һ�ٷ֣���������ʲôѽ������˵��ʮ��Ǯ������˵���Ǿ��Ƚ���һ��ɣ��ҵ�����ʮ�֡�",QTTS_GBK);
		break;
//---------------------------------����-------------------------------------------------------
	case MUSIC_1:
		QttsPlayEvent("����һ,����һ,һ��С�����ɻ���",QTTS_GBK);
		break;
	case MUSIC_2:
		QttsPlayEvent("��ѽ��ѽ�����ѣ��ҵ�һ�������ѣ�������ѽ�����֣������ҵĺ����ѡ�",QTTS_GBK);
		break;
	case MUSIC_3:
		QttsPlayEvent("�޹������޹��������ǵ��������졣",QTTS_GBK);
		break;
	case MUSIC_4:
		QttsPlayEvent("СС���꣬���ٷ��գ��������������ա�",QTTS_GBK);
		break;
	case MUSIC_5:
		QttsPlayEvent("�����������־����ط���С���ѵĺ��棬��Ҳ�Ҫ�������������׽ס���������׽ס���������׽ס����",QTTS_GBK);
		break;
	case MUSIC_6:
		QttsPlayEvent("ҹҹ��������Ļ������������³������",QTTS_GBK);
		break;
//---------------------------------ʫ��-------------------------------------------------------
	case SONG_1:
		QttsPlayEvent("�Ҹ������ʫ�ɣ���ҹ˼�����ߣ���ף���ǰ���¹⣬���ǵ���˪����ͷ�����£���ͷ˼���硣",QTTS_GBK);
		break;
	case SONG_2:
		QttsPlayEvent("�Ҹ������ʫ�ɣ�ӽ�죬�ơ���������죬�죬�죬��������衣��ë����ˮ�����Ʋ��岨��",QTTS_GBK);
		break;
	case SONG_3:
		QttsPlayEvent("�Ҹ������ʫ�ɣ�����ȸ¥���ơ���֮����������ɽ�����ƺ��뺣��������ǧ��Ŀ������һ��¥��",QTTS_GBK);
		break;
	case SONG_4:
		QttsPlayEvent("�Ҹ������ʫ�ɣ����������ơ��Ϻ�Ȼ�����߲�����������������ҹ��������������֪���١�",QTTS_GBK);
		break;
	case SONG_5:
		QttsPlayEvent("�Ҹ������ʫ�ɣ������¾�����ɽ���ֵܡ��ơ���ά����������Ϊ��ͣ�ÿ��ѽڱ�˼�ס�ң֪�ֵܵǸߴ������������һ�ˡ�",QTTS_GBK);
		break;
	case SONG_6:
		QttsPlayEvent("�Ҹ������ʫ�ɣ�����®ɽ�ٲ����ơ���ס�������¯�����̣�ң���ٲ���ǰ��������ֱ����ǧ�ߣ�������������졣",QTTS_GBK);
		break;
	case SONG_7:
		QttsPlayEvent("�Ҹ������ʫ�ɣ��������ס��ơ���ס���׳��۽����У����Ű���̤�������һ�̶ˮ��ǧ�ߣ��������������顣",QTTS_GBK);
		break;
	case SONG_8:
		QttsPlayEvent("�Ҹ������ʫ�ɣ����ƺ�¥���Ϻ�Ȼ֮���꡷�ơ���ס��������ǻƺ�¥���̻����������ݡ��·�ԶӰ�̿վ���Ψ�������������",QTTS_GBK);
		break;
	case SONG_9:
		QttsPlayEvent("�Ҹ������ʫ�ɣ����緢�׵۳ǡ��ơ���ס����ǰ׵۲��Ƽ䣬ǧ�ｭ��һ�ջ�������Գ���䲻ס�������ѹ�����ɽ��",QTTS_GBK);
		break;
	case SONG_10:
		QttsPlayEvent("�Ҹ������ʫ�ɣ�������ż�顷�ơ���֪�¡���С����ϴ�أ������޸���ë˥����ͯ�������ʶ��Ц�ʿʹӺδ�����",QTTS_GBK);
		break;
//---------------------------------��ȫ������-------------------------------------------------------
	case EDUCARE_1:
		QttsPlayEvent("��ȫС֪ʶ������������ż��������Ͳ�Ч���á�",QTTS_GBK);
		break;
	case EDUCARE_2:
		QttsPlayEvent("��ȫС֪ʶ����·Ҫ�����е�����׼׷��ʹ��֡�һ��������ͨ�����ᴩ��·�����ˡ�",QTTS_GBK);
		break;
	case EDUCARE_3:
		QttsPlayEvent("��ȫС֪ʶ����ʳһ����������������ˮ��ʳ�١�",QTTS_GBK);
		break;
	case EDUCARE_4:
		QttsPlayEvent("��ȫС֪ʶ������¥�ݿ����У���׼ӵ����������",QTTS_GBK);
		break;
	case EDUCARE_5:
		QttsPlayEvent("��ȫС֪ʶ���˳�Ҫ�ȳ�ͣ�ȣ����º��ϲ����ꡣ��ͷ���ܳ��������������ֺ���Ҫ��",QTTS_GBK);
		break;
	case EDUCARE_6:
		QttsPlayEvent("��ȫС֪ʶ��Сѧ����Ҫ�μǣ����Ǹߣ����µ͡���׷�𣬲�Ͷ������ȫ��ʶ�ŵ�һ��",QTTS_GBK);
		break;
	case EDUCARE_7:
		QttsPlayEvent("��ȫС֪ʶ�����ͣ���̵��У����̵�������У���ͣͣ�п�������",QTTS_GBK);
		break;
	}
}
#define Q_VOICES_1	1
#define Q_TAIBEN_1	2
#define Q_TAIBEN_2	3
#define Q_TAIBEN_3	4
#define Q_TAIBEN_4	5
#define Q_TAIBEN_5	6
#define Q_TAIBEN_6	7
#define Q_TAIBEN_7	8
#define Q_TAIBEN_8	9
#define Q_TAIBEN_9	10
static void TaiBenToTulingQuestEr(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (5.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case Q_VOICES_1:
			create_event_system_voices(5);
			break;
#ifdef QITUTU_SHI
		case Q_TAIBEN_1:
			createPlayEvent((const void *)"xiai",PLAY_NEXT);
			break;
#else
		case Q_TAIBEN_2:
			createPlayEvent((const void *)"mp3",PLAY_NEXT);
			break;
		case Q_TAIBEN_3:
			createPlayEvent((const void *)"story",PLAY_NEXT);
			break;
		case Q_TAIBEN_4:
			createPlayEvent((const void *)"guoxue",PLAY_NEXT);
			break;
		case Q_TAIBEN_5:
			createPlayEvent((const void *)"english",PLAY_NEXT);
			break;
#endif
#if 0
		case Q_TAIBEN_6:
			QttsPlayEvent("С���������˵������Ц����",QTTS_GBK);
			break;
		case Q_TAIBEN_7:
			QttsPlayEvent("С���Ѹ���˵����������",QTTS_GBK);
			break;
		case Q_TAIBEN_8:
			QttsPlayEvent("С���������ң�2+3+4+5���ڼ���",QTTS_GBK);
			break;
		case Q_TAIBEN_9:
			QttsPlayEvent("С���������ң�������ա�",QTTS_GBK);
			break;
#endif
	}
}
void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	int textSize=0, err=0;
	char *text=NULL;
	start_event_play_wav();//��ͣ¼��
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
#if 0
	char *URL= (char *)calloc(1,strlen("http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3")+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return -1;
	}
	sprintf(URL,"%s","http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3");
	AddDownEvent(URL,TULING_URL_MAIN);
#else
	err=reqTlVoices(10,key,(const void *)voicesdata,len,RECODE_RATE,voices_type,&text,&textSize);
	if(err==-1){
#if 1
		create_event_system_voices(5);	
#else
		TaiBenToTulingQuestEr();
#endif
		goto exit1;
	}else if(err==1){
		TaiwanToTulingError();
		goto exit1;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
#endif
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#endif
/******************************************************
@��������:	ѧϰ���¼�������
@����:	data ���� len ���ݴ�С
*******************************************************/
static void runJsonEvent(const char *data)
{
	parseJson_string(data,handle_text);
	free(data);
}
int event_lock=0;
/*******************************************************
@��������:	����¼�������
@����:	databuf ����	len ���ݳ��� type�¼�����
@����ֵ:	-1 ���ʧ�� ������ӳɹ�
********************************************************/
int add_event_msg(const char *databuf,int  len,int  type)
{
	int msgSize=0;
	if(event_lock){
		DEBUG_STD_MSG("add_event_msg event_lock =%d\n",event_lock); // д�� type event_lock a+
		eventlockLog("event_lock add error\n",event_lock);
		return ;
	}
	if(type!=LOCAL_MP3_EVENT)	//��Ϊ���ز���������������
		sysMes.localplayname=0;
	eventlockLog("event_lock add ok\n",event_lock);
	struct eventMsg *msg =(struct eventMsg *)(&msgSize);
	msg->len = len;
	msg->type = type;
	printf("add end ..\n");
	return putMsgQueue(evMsg,databuf,msgSize);
}
int getEventNum(void)
{
	return getWorkMsgNum(evMsg);
}
void cleanEvent(void){
char *msg;
int msgSize;
event_lock=1;	//�ܱ���״̬�¼�
while(getWorkMsgNum(evMsg)){
	getMsgQueue(evMsg,&msg,&msgSize);
	if(msg!=NULL){
		free(msg);
		usleep(100);
	}
}
event_lock=0;
}
/*******************************************************
@��������:	�¼�������
@����:	data ����	msgSize�¼������Լ����ݴ�С�ṹ��
********************************************************/
static void handle_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize); 
	DEBUG_STD_MSG("=====================================================================================\n");
	DEBUG_STD_MSG("handle_event_msg cur->type = %d\n",cur->type);
	DEBUG_STD_MSG("=====================================================================================\n");
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//�Ự�¼�
#ifdef QITUTU_SHI
			Led_System_vigue_close();
#endif
			runJsonEvent(data);
			//pause_record_audio();
			break;
			
		case SYS_VOICES_EVENT:		//ϵͳ���¼�
			start_event_play_wav();
			handle_event_system_voices(cur->len);
			if(cur->len==2)
				break;
			pause_record_audio();
			break;
			
		case SET_RATE_EVENT:		//URL�����¼�
			event_lock=1;	//�ܱ���״̬�¼�
			eventlockLog("eventlock_start\n",event_lock);
			cleanplayEvent(1);
			//cleanEvent();
			NetStreamExitFile();
			i2s_start_play(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio();
			eventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL���粥���¼�
			playurlLog("url play\n");
			cleanplayEvent(0);		
			NetStreamExitFile();
			start_event_play_url();
			playurlLog("NetStreamExitFile\n");
			AddDownEvent(data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//�������ֲ����¼�
			cleanplayEvent(0);		//ȥ��������
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent(data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif
			
		case QTTS_PLAY_EVENT:		//QTTS�¼�
			PlayTuLingTaibenQtts(data,cur->len);
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//���յ�������Ϣ	
			playspeekVoices(data);
			pause_record_audio();
			remove(data);
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//�Խ��¼�
			handle_voices_key_event(cur->len);
			break;
#endif
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
	handleeventLog("handleevent end\n",cur->type);
}

static void clean_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize);
	if(cur->type==STUDY_WAV_EVENT){
		free(data);
	}else if(cur->type==SYS_VOICES_EVENT){

	}
}


void init_stdvoices_pthread(void)
{
	evMsg = InitCondWorkPthread(handle_event_msg);
	init_iat_MSPLogin(WriteqttsPcmData);
}
void clean_stdvoices_pthread(void)
{
	CleanCondWorkPthread(evMsg,clean_event_msg);
	iat_MSPLogout();
}
