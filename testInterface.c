#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "base/cJSON.h"
#include "host/StreamPlay/StreamFile.h"
#include "nvram.h"

#include "config.h"

#if 0
#define LEVEL_PLAY_NETWORK_0		0		//播放当前连接名字
#define LEVEL_PLAY_QTTS_1		1
#define LEVEL_PLAY_WEIXIN_SPEEK_1	1
#define LEVEL_PLAY_SYSTEM_VOICES_1	1
#define LEVEL_PLAY_NETWORK_URL_1	1
#define LEVEL_PLAY_SDCARD_MUSIC_1	1
#define LEVEL_RECODER_WEIXIN_2		2	
#define LEVEL_RECODER_AI_SPEEK_2	2
int checkPlayVoicesWorkState(int checkLevel){
	if(checkNetWorkLive()){	//检查网络
		return -1;
	}
	if(getEventNum()>0||getplayEventNum()>0){//防止添加过快
		DEBUG_EVENT("num =%d \n",getEventNum());
		return -1;
	}
	int playState = GetRecordeVoices_PthreadState();
	if(playState>checkLevel){
		return -1;
	}
	switch(playState){
		case PLAY_WAV:	//执行相应的退出，并切换工作状态
			WritePlayUrl_Log("add failed ,reocde voices pthread is PLAY_WAV\n");
			break;
	}
}
#endif

#ifdef TEST_ERROR_TULING
/*
*单独测试图灵的接口
*/
void test_tulingApi_andDownerrorFile(void){
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"code",10);
	cJSON_AddStringToObject(root,"info","testetest");
	cJSON_AddStringToObject(root,"text","testetest");
	cJSON_AddStringToObject(root,"ttsUrl","http://smartdevice.ai.tuling123.com/file2/ace2b7b9-df01-4554-95b2-6a3d5189b0b0.pcm");
	char *text=cJSON_Print(root);
	cJSON_Delete(root);
	AddworkEvent(text,0,STUDY_WAV_EVENT);
}
#endif

#ifdef TEST_MIC
/*******************************************************
函数功能: 测试录音并直接播放出来
参数:
返回值:
********************************************************/
void *TestRecordePlay(void){
	char *pBuf;
	while(GetRecordeVoices_PthreadState()!=RECODE_STOP)
	{
		DEBUG_VOICES("test_recorde_play :recorde now...\n");
		pBuf = I2sGetvoicesData();			//占用的时间有点长
		memcpy(play_buf,pBuf,I2S_PAGE_SIZE);	
		write_pcm(play_buf);
		//usleep(3000*1000);
	}
}
#endif	//end TEST_MIC


#ifdef PCM_TEST
void test_save8kpcm(char *Testbuf,int len){
	char file[128]={0};
	time_t ti;
	time(&ti);
	snprintf(file,128,"./%d.8k",(int)ti);
	FILE *fp=fopen(file,"w+");
	if(fp==NULL)
		return;
	fwrite(Testbuf,1,len,fp);
	fclose(fp);
}
void test_save16kpcm(char *Testbuf,int len){
	char file[128]={0};
	time_t ti;
	time(&ti);
	snprintf(file,128,"./%d.16k",(int)ti);
	FILE *fp=fopen(file,"w+");
	if(fp==NULL)
		return;
	int pos=0;
	for(pos=0;pos<len;pos+=2){
		fwrite(Testbuf+pos,1,2,fp);
		fwrite(Testbuf+pos,1,2,fp);
	}
	fclose(fp);
}
#endif

#ifdef TEST_DOWNFILE
void test_playTuingPcmFile(void){
	//FILE *fp = fopen("8caf0b37-3359-4b85-b06b-aec080ab1d69.pcm","r");
	FILE *fp = fopen("test_down.pcm","r");
	if(fp==NULL){
		perror("fopen read failed ");
		return ;
	}
	int pos=0;
	char buf[2]={0};
	int ret =0;
	while(1){
		ret = fread(buf,2,1,fp);
		if(ret==0){
			break;
		}
		memcpy(play_buf+pos,buf,2);
		pos+=2;
		memcpy(play_buf+pos,buf,2);
		pos+=2;
		if(pos==I2S_PAGE_SIZE){
			write_pcm(play_buf);
			pos=0;
		}
	}
	fclose(fp);
}
#endif

#ifdef TEST_PLAY_EQ_MUSIC
/*******************************************************
函数功能: 测试播放wav 音频文件
参数: play 本地MP3播放内容
返回值: 无
********************************************************/
void createPlay_wavFileEvent(const void *play){
	if(checkNetWorkLive()){	//检查网络
		return;
	}
	if(getEventNum()>0||getplayEventNum()>0){
		DEBUG_EVENT("num =%d \n",getEventNum());
		return;
	}
	if(GetRecordeVoices_PthreadState() == PLAY_WAV){
		return;
	}
	AddworkEvent((const char *)play,0,TEST_PLAY_EQ_WAV);
}
static unsigned char Testmp3sign =0;
void TestPlay_localMp3Music(void){
	char filepath[128]={0};
	if(Testmp3sign>8){
		Testmp3sign=0;
	}
	snprintf(filepath,128,"%stestmp3/test%d.mp3",TF_SYS_PATH,Testmp3sign);
	printf("filepath = %s\n",filepath);
	if(access(filepath, F_OK)==F_OK){
		__AddLocalMp3ForPaly((const char *)filepath);
	}
	Testmp3sign++;
	return;
}
#endif

void betaServer_OkUserId_And_token(void){
#if 0	//id和token测试ok
		char token[64]={"772f32e9-1a8a-46fe-95ed-76b405e71fca"};
		char *user_id  = "ai22334455667780";
#endif
#if 0
		char token[64]={"b3346661-ee10-4d46-ad1e-5b35bf2a824e"};
		char *user_id  = "ai00000000000015";
#endif
#if 0
		char token[64]={"de693565-b4ab-49d2-8376-7bb01d799d6f"};
		char *user_id  = "ai00000000000016";
#endif
#if 0
		char token[64]={"2f3f51f9-0488-454a-aff8-5063860e8c5c"};
		char *user_id  = "ai00000000000017";
#endif
#if 0
		char token[64]={"954689a2-1203-4eca-bd59-7ce690c9f0c2"};
		char *user_id  = "ai2000000000000";
#endif
#if 0	//id和token测试ok //beta
		char token[64]={"8589b8c9-c1a8-4492-8c26-f5f2586493ab"};
		char *user_id  = "aia0000000000002";
#endif
		
#if 0	//smart  id和token 测试ok smart
		char token[64]={"63c5dc4b-ccff-48cd-b112-eff5426ca3e8"};
		char *user_id  = "ai123456789ababab";
#endif
#if 0   //idoítoken2aê?ok //smart
		char token[64]={"b825f483-d5be-4a6f-a1bb-80be3de187e5"};
		char *user_id  = "ai123456789abab12";
#endif
#if 0	//beta ok
char token[64]={"fa750125-cb6d-4259-b667-278c32f2eece"};	
char *user_id  = "airhwrhwrhw00104";
#endif
}

#ifdef TEST_PLAY_EQ_MUSIC
#include "host/voices/WavAmrCon.h"
#include "host/voices/wm8960i2s.h"

void SetplayWavEixt(void){
	st->player.playState=MAD_EXIT;
}
void TestPlay_EqWavFile(const void *data){
	Player_t *play =(Player_t *)data;
	st->player.progress=0;
	st->streamLen=0;
	st->playSize=0;
	
	unsigned short rate_one=0,rate_two=0;
	char filepath[256]={0};
	snprintf(filepath,256,"/media/mmcblk0p1/testmp3/%s",play->playfilename);
	st->rfp = fopen(filepath,"r");
	if(st->rfp==NULL){
		perror("fopen read failed ");
		return ;
	}
	struct wave_pcm_hdr wav;
	fread(&wav,sizeof(struct wave_pcm_hdr),1,st->rfp);
	writeLog((const char * )"/log/test_play_wav.txt",(const char *) play->playfilename);
	char logBuf[128]={0};
	sprintf(logBuf,"rate %d channel %d",wav.samples_per_sec,wav.channels);
	if(wav.samples_per_sec>44100){
		wav.samples_per_sec=44100;
	}
	if(wav.channels>2){
		wav.channels=2;
	}
	writeLog((const char * )"/log/test_play_wav.txt",(const char *) logBuf);
	st->SetI2SRate(wav.samples_per_sec);
	Mute_voices(UNMUTE);
	int i=0,pos=0,ret=0,Numbers=0;
	char cachebuf[2]={0};
	st->player.playState=MAD_PLAY;
	while(1){
		if(st->player.playState==MAD_EXIT)
			break;
		pos =0;
		if(wav.channels==1){
			for(i=0;i<I2S_PAGE_SIZE;i+=2){
				ret = fread(cachebuf,1,2,st->rfp);
				if(ret==0){
					break;
				}
				memcpy(play_buf+pos,cachebuf,2);
				pos+=2;
				memcpy(play_buf+pos,cachebuf,2);
				pos+=2;
				write_pcm(play_buf);
			}				
		}else if(wav.channels==2){
			ret = fread(play_buf,1,I2S_PAGE_SIZE,st->rfp);
			if(ret==0){
				break;
			}
			sprintf(logBuf,"play Numbers %d ret=%d ",Numbers++,ret);
			//writeLog((const char * )"/log/test_play_wav.txt",(const char *) logBuf);
			write_pcm(play_buf);
		}
	}
	fclose(st->rfp);
}
#endif

#ifdef WORK_INTER
void test_quikSeekTo(void){
	st->player.progress +=10;
	if(st->player.progress<=0)
		st->player.progress=1;
	else if(st->player.progress >=100)
		st->player.progress =100;
	seekToStream(st->player.progress);
}
void test_backSeekTo(void){
	st->player.progress -=10;
	if(st->player.progress<=0)
		st->player.progress=1;
	else if(st->player.progress >=100)
		st->player.progress =100;
	seekToStream( st->player.progress);
}
#endif


void test_clock_Interfaces(const char *str){
	FILE *fp =fopen("/media/mmcblk0p1/testclock.txt","a+");
	char buf[128]={0};
	sprintf(buf,"test clock %s \n",str);
	if(fp){
		fwrite(buf,"123",3,fp);
		fwrite(buf,strlen(buf),1,fp);
		fflush(fp);
		fclose(fp);
	}
}
void test_Clock_saveLoute(const char *clockTime){
#if 0	
	char buf_s[128]={0};
	sprintf(buf_s,"nvram_set 2860 testclock %s", clockTime);
	system(buf_s);
#endif	
}

static unsigned char playIndex=0;

#define TEST_START_ADD	1
#define TEST_STOP_ADD	2
#define TEST_SINGLE_ADD	3

static unsigned char play_state=0;
static unsigned char test_pthread=0;
static char play_index_buf[5][128]={"http://smartdevice.ai.tuling123.com/file1/85de323e-8c3b-4daa-b0d7-87ec47600c5a.pcm",
	"http://smartdevice.ai.tuling123.com/file1/c8a5652c-1d1d-4bf1-8ef7-5748312147ea.pcm",
	"http://smartdevice.ai.tuling123.com/file1/e3b8e324-d93e-44a9-86e5-92ba4d6cb5c5.pcm",
	"http://smartdevice.ai.tuling123.com/file1/3f4a98d5-63e8-4547-b70d-96c9f558db48.pcm",
	"http://smartdevice.ai.tuling123.com/file1/3baf3631-1182-486d-960a-3dbfef9cfc44.pcm",
	"http://smartdevice.ai.tuling123.com/file1/83ba4cd1-4ce8-4816-92ce-20821e9b2c6a.pcm"};

static void addplay_urlFile(void){
	char *url = play_index_buf[playIndex];
	char *ttsURL= (char *)calloc(1,strlen(url)+1);
	sprintf(ttsURL,"%s",url);
	SetMainQueueLock(MAIN_QUEUE_UNLOCK);
	SetplayNetwork_Lock();	
	AddDownEvent((const char *)ttsURL,TULING_URL_MAIN);
	if(++playIndex==5){
		playIndex=0;
	}
}
static void *test_play_tulUrl(void){
	while(test_pthread){	
		switch(play_state){
			case TEST_START_ADD:
				addplay_urlFile();
				break;
			case TEST_SINGLE_ADD:
				play_state=TEST_STOP_ADD;
				addplay_urlFile();
				break;
			case TEST_STOP_ADD:
				break;
		}
		sleep(8);
	}
}
void test_start_playurl(void){
	play_state=TEST_START_ADD;
	if(test_pthread)
		return ;
	test_pthread=1;
	pool_add_task(test_play_tulUrl,NULL);
}
void test_stop_playurl(void){
	play_state=TEST_STOP_ADD;
}

void test_single_playurl(void){
	play_state=TEST_SINGLE_ADD;
}

#if 0
#define KB 1024
#define QTTS_PLAY_SIZE	12*KB
#define LEN_BUF 1*KB
#define LEN_TAR 2*KB
/*******************************************************
函数功能: 单声道转双声道
参数:	dest_files 输出文件 src_files输入文件
返回值: 0 转换成功 -1 转换失败
********************************************************/
int voices_single_to_stereo(char *dest_files,char *src_files){
	FILE  *fd_q,*fd_w;
	size_t read_size;
	int size = 0,pos= 0;
	char buf[LEN_BUF],tar[LEN_TAR];
	fd_q = fopen(src_files,"r");
	fseek(fd_q, WAV_HEAD, SEEK_SET);
	if (NULL == fd_q)
	{
		printf("open file src_voices error\n");
		return -1;
	}
	fd_w = fopen(dest_files,"w+");
	if (NULL == fd_w)
	{
		printf("open file dest_voices error\n");
		return -1;
	}
	while(1)
	{
		read_size=fread(buf, 1,LEN_BUF, fd_q);
		if(read_size==0)
		{
			break;
		}
		while(read_size != size)
		{
			memcpy(tar+pos,buf+size,2);
			pos += 2;
			memcpy(tar+pos,buf+size,2);
			size += 2;
			pos += 2;
		}
		//printf("voices size =%d,len =%d\n",size,len);
      	fwrite(tar, 2*read_size, 1,fd_w);
		pos= 0;
		size = 0;
	}
	fclose(fd_q);
	fclose(fd_w);
	return 0;
}

static void single_to_stereo(char *src,int srclen,char *tar,int *tarlen)
{
	int i=0;
	int pos=0;
	for(i=0; i<srclen; i+=2)
	{
		memcpy(tar+pos,src+i,2);
		pos += 2;
		memcpy(tar+pos,src+i,2);
		pos += 2;
	}
	*tarlen = pos;
}
#endif

