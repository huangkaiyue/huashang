#include "comshead.h"
#include "base/pool.h"
#include "config.h"
#include "host/voices/wm8960i2s.h"

extern  void *play_qtts_data(void *arg);
//开始下载, 接口兼容，需要去掉streamLen
static void tulingStartDown(const char *filename,int streamLen){
	initputPcmdata();
	printf("filename =%s streamLen=%d\n",filename,streamLen);
}
//获取到流数据
static void  tulingGetStreamData(const char *data,int size){
	putPcmdata((const void *)data,size);
}
//结束下载
static void  tulingEndDown(int downLen){
	printf("tulingEndDown mp3 \n");
}
#ifdef TEST_DOWNFILE
void test_read_file(void){
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
void downTulingMp3(const char *url){
	printf("start down tuling mp3 \n");
#ifdef TEST_DOWNFILE
	test_read_file();
#else
	setDowning();
	StartPthreadPlay();
	tulingLog("downTulingMp3 start",1);
	demoDownFile(url,15,tulingStartDown,tulingGetStreamData,tulingEndDown);
	SetDownExit();
	tulingLog("downTulingMp3 wait",1);
	WaitPthreadExit();
	tulingLog("downTulingMp3 end",1);
	printf("downTulingMp3 exit mp3 \n");
#endif
}
