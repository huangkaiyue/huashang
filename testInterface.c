#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "base/cJSON.h"
#include "config.h"

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
	if(GetRecordeVoices_PthreadState() == PLAY_WAV_E){
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


