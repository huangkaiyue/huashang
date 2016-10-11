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

//默认音频头部数据
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
#define VOICES_MIN	13200	//是否是大于0.5秒的音频，采样率16000、量化位16位
#define VOICES_ERR	1000	//误触发

#define KB		1024

static char buf_voices[STD_RECODE_SIZE];
static char amr_data[12*KB];
static int len_voices = 0;

/*****************************************************
*开启语音识别状态
*****************************************************/
void start_event_std(void)
{	
	sysMes.recorde_live=START_SPEEK_VOICES;
}
/*****************************************************
*结束语音识别状态
*****************************************************/
void end_event_std(void)
{
	sysMes.recorde_live=END_SPEEK_VOICES;
}
/*****************************************************
*进入播放wav原始数据状态
*****************************************************/
void start_event_play_wav(void)
{
	sysMes.recorde_live =PLAY_WAV;
}
/*****************************************************
*进入播放URL状态
*****************************************************/
void start_event_play_url(void)
{
	sysMes.recorde_live =PLAY_URL;
}
/*****************************************************
*暂停录音状态
*****************************************************/
void pause_record_audio(void)
{	
	sysMes.recorde_live =RECODE_PAUSE ;
}
/*****************************************************
*获取状态
*****************************************************/
int GetRecordeLive(void)
{
	return sysMes.recorde_live;
}
/*****************************************************
*录制退出状态
******************************************************/
void exit_handle_event(void)
{
	sysMes.recorde_live=RECODE_STOP;
}
/*****************************************************
*录制短消息状态
******************************************************/
void start_event_talk_message(void)
{
	sysMes.recorde_live =START_TAIK_MESSAGE;
}
#ifdef TIMEOUT_CHECK
/*****************************************************
*保存上一次切换状态和恢复状态
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
@函数功能:	处理采集数据函数
@参数:	data 采集到的数据  size 数据大小
*****************************************/
static void voices_packt(const char *data,int size)
{
	int amr_size=0;
	int i;
	if(data != NULL)
	{
		//大于5秒的音频丢掉
		if((len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){
			goto exit0;
		}
#if 0		
		//只有有声道有数据，左声道没有，需要拷贝2、3两个数据
		for(i=2; i<size; i+=4){
			//双声道数据转成单声道数据
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
	else if(len_voices > VOICES_MIN)//音频上传
	{
		pcmwavhdr.size_8 = (len_voices+36);
		pcmwavhdr.data_size = len_voices;
		memcpy(buf_voices,&pcmwavhdr,WAV_HEAD);				//写音频头
		if(WavToAmr8k((const char *)buf_voices,amr_data,&amr_size)){		//转换成amr格式
			DEBUG_VOICES("enc failed \n");
		}
		send_voices_server((const char *)amr_data,amr_size,"amr");
		goto exit0;
	}
	else if(len_voices < VOICES_ERR)
	{
		goto exit1;//误触发
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
函数功能: 录音线程 ，对当前环境检测，筛选有效音频
参数:   	无
返回值: 
********************************************************/
static void *pthread_record_voices(void *arg)
{
	char *pBuf;
	sysMes.error_400002=3;//保证第一次就能报出400002的语音
	sysMes.recorde_live=RECODE_PAUSE;
	//sysMes.oldrecorde_live=sysMes.recorde_live;
	while(sysMes.recorde_live!=RECODE_STOP){
		switch(sysMes.recorde_live){
			case START_SPEEK_VOICES:
				pBuf = i2s_get_data();
				voices_packt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:
				voices_packt(NULL, 0);//发送空音频作为一段音频的结束标志
				break;
#ifdef SPEEK_VOICES
			case START_TAIK_MESSAGE:
				pBuf = i2s_get_data();
				save_recorder_voices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);//不会有快进的感觉
				break;
#endif
			default:
				//printf("pthread_record_voices: recorde_live (%d)\n",sysMes.recorde_live);
				i2s_get_data();	//默认状态清除音频
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
函数功能: 测试录音并直接播放出来
参数:
返回值:
********************************************************/
static void *test_recorde_play(void)
{
	char *pBuf;
	while(sysMes.recorde_live!=RECODE_STOP)
	{	
		DEBUG_VOICES("test_recorde_play :recorde now...\n");
		pBuf = i2s_get_data();			//占用的时间有点长
		memcpy(play_buf,pBuf,I2S_PAGE_SIZE);	
		write_pcm(play_buf);
		//usleep(3000*1000);
	}
}
#endif	//end TEST_MIC
/*****************************************************
开启录音工作线程
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
退出录音(退出系统)
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
