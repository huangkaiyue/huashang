#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "base/pool.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "config.h"

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
#define VOICES_MIN	13200	//�Ƿ��Ǵ���0.5�����Ƶ��������16000������λ16λ
#define VOICES_ERR	1000	//�󴥷�

#define KB		1024

static char buf_voices[STD_RECODE_SIZE];
static char amr_data[12*KB];
static int len_voices = 0;

/*****************************************************
*��������ʶ��״̬
*****************************************************/
void start_event_std(void)
{	
	sysMes.recorde_live=START_SPEEK_VOICES;
}
/*****************************************************
*��������ʶ��״̬
*****************************************************/
void end_event_std(void)
{
	sysMes.recorde_live=END_SPEEK_VOICES;
}
/*****************************************************
*���벥��wavԭʼ����״̬
*****************************************************/
void start_event_play_wav(void)
{
	sysMes.recorde_live =PLAY_WAV;
}
/*****************************************************
*���벥��URL״̬
*****************************************************/
void start_event_play_url(void)
{
	sysMes.recorde_live =PLAY_URL;
}
/*****************************************************
*��ͣ¼��״̬
*****************************************************/
void pause_record_audio(void)
{	
	sysMes.recorde_live =RECODE_PAUSE ;
}
/*****************************************************
*��ȡ״̬
*****************************************************/
int GetRecordeLive(void)
{
	return sysMes.recorde_live;
}
/*****************************************************
*¼���˳�״̬
******************************************************/
void exit_handle_event(void)
{
	sysMes.recorde_live=RECODE_STOP;
}
/*****************************************************
*¼�ƶ���Ϣ״̬
******************************************************/
void start_event_talk_message(void)
{
	sysMes.recorde_live =START_TAIK_MESSAGE;
}
#ifdef TIMEOUT_CHECK
/*****************************************************
*������һ���л�״̬�ͻָ�״̬
*****************************************************/
void keep_recorde_live(int change)
{
	if(change==1)
		sysMes.oldrecorde_live=sysMes.recorde_live;
	else
		sysMes.recorde_live=sysMes.oldrecorde_live;
}
#endif	//end TIMEOUT_CHECK

/****************************************
@��������:	����ɼ����ݺ���
@����:	data �ɼ���������  size ���ݴ�С
*****************************************/
static void voices_packt(const char *data,int size)
{
	int amr_size=0;
	int i;
	if(data != NULL)
	{
		//����5�����Ƶ����
		if((len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){
			goto exit0;
		}
#if 0		
		//ֻ�������������ݣ�������û�У���Ҫ����2��3��������
		for(i=2; i<size; i+=4){
			//˫��������ת�ɵ���������
			memcpy(buf_voices+WAV_HEAD+len_voices,data+i,2);
			len_voices += 2;
		}
#else
		for(i=0; i<size; i+=4){
			memcpy(buf_voices+WAV_HEAD+len_voices,data+i,2);
			len_voices += 2;
		}
#endif	//end 0
	}
	else if(len_voices > VOICES_MIN)//��Ƶ�ϴ�
	{
		pcmwavhdr.size_8 = (len_voices+36);
		pcmwavhdr.data_size = len_voices;
		memcpy(buf_voices,&pcmwavhdr,WAV_HEAD);				//д��Ƶͷ
		if(WavToAmr8k((const char *)buf_voices,amr_data,&amr_size)){		//ת����amr��ʽ
			DEBUG_VOICES("enc failed \n");
		}
		send_voices_server((const char *)amr_data,amr_size,"amr");
		goto exit0;
	}
	else if(len_voices < VOICES_ERR)
	{
		goto exit1;//�󴥷�
	}
	else
	{
		if(sysMes.recorde_live !=PLAY_WAV){
			play_sys_tices_voices(NO_VOICES);
		}
		goto exit1;
	}
	return ;
exit1:
	pause_record_audio();
exit0:
	memset(buf_voices,0,len_voices);
	len_voices = 0;
}

/*******************************************************
��������: ¼���߳� ���Ե�ǰ������⣬ɸѡ��Ч��Ƶ
����:   	��
����ֵ: 
********************************************************/
static void *pthread_record_voices(void *arg)
{
	char *pBuf;
	sysMes.error_400002=3;//��֤��һ�ξ��ܱ���400002������
	sysMes.recorde_live=RECODE_PAUSE;
	//sysMes.oldrecorde_live=sysMes.recorde_live;
	while(sysMes.recorde_live!=RECODE_STOP){
		switch(sysMes.recorde_live){
			case START_SPEEK_VOICES:
				pBuf = i2s_get_data();
				voices_packt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:
				voices_packt(NULL, 0);//���Ϳ���Ƶ��Ϊһ����Ƶ�Ľ�����־
				break;
#ifdef SPEEK_VOICES
			case START_TAIK_MESSAGE:
				pBuf = i2s_get_data();
				save_recorder_voices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);//�����п���ĸо�
				break;
#endif
			default:
				//printf("pthread_record_voices: recorde_live (%d)\n",sysMes.recorde_live);
				i2s_get_data();	//Ĭ��״̬�����Ƶ
				usleep(30000);
				break;
		}
	}
	sysMes.recorde_live =RECODE_EXIT_FINNISH;
	DEBUG_VOICES("handle record voices exit\n");
	return NULL;
}
#ifdef TEST_MIC
/*******************************************************
��������: ����¼����ֱ�Ӳ��ų���
����:
����ֵ:
********************************************************/
static void *test_recorde_play(void)
{
	char *pBuf;
	while(sysMes.recorde_live!=RECODE_STOP)
	{	
		DEBUG_VOICES("test_recorde_play :recorde now...\n");
		pBuf = i2s_get_data();			//ռ�õ�ʱ���е㳤
		memcpy(play_buf,pBuf,I2S_PAGE_SIZE);	
		write_pcm(play_buf);
		//usleep(3000*1000);
	}
}
#endif	//end TEST_MIC
/*****************************************************
����¼�������߳�
*****************************************************/
void init_record_pthread(void)
{	
#ifdef TEST_MIC
	if(pthread_create_attr(test_recorde_play,NULL)){
		perror("create test recorde play failed!");
		exit(-1);
	}
#else
	if(pthread_create_attr(pthread_record_voices,NULL)){
    	perror("create handle record voices failed!");
        exit(-1);
	}
#endif	//end TEST_MIC
	usleep(300);
}

/*****************************************************
�˳�¼��(�˳�ϵͳ)
*****************************************************/
void exit_record_pthread(void)
{
	unsigned char timeout=0;
	sysMes.recorde_live = RECODE_STOP;
	while(sysMes.recorde_live!=RECODE_EXIT_FINNISH){
		DEBUG_VOICES("wait record audio exit recorde_live(%d)\n",sysMes.recorde_live);
		usleep(100000);
		if(sysMes.recorde_live!=RECODE_EXIT_FINNISH)
			sysMes.recorde_live = RECODE_STOP;
		if(timeout++>50)
			break;
	}
	DEBUG_VOICES("exit record pthread success recorde_live(%d)\n",sysMes.recorde_live);
}
