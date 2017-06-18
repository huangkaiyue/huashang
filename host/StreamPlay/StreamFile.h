#ifndef _STREAMFILE_H
#define _STREAMFILE_H

#include <pthread.h>
#include <stdio.h>
#include "base/queWorkCond.h"

//#define TEST_SAVE

#define SAFE_READ_WRITER		//安全的读写文件

#define SEEK_TO					//进度条拖动实现快进
//#define SHOW_progressBar		//显示播放进度

#define KB	1024
#define CACHE_PLAY_SIZE		8*KB
#define LIST_CACHE_SIZE		4*KB
#define BUF_TANG			16*KB

#define UDP_ACK		1
#define TCP_ACK		0

#define MUSIC_PLAY_LIST			0	//列表播放
#define	MUSIC_SINGLE_LIST		1 	//单曲循环

#define MP3STEAM_SIZE	sizeof(Mp3Stream)

typedef struct{
	unsigned char playListState;//播放列表状态		0:顺序播放 1:单曲播放		
	unsigned char playState;	//播放状态
	unsigned char vol;			//音量大小	
	short progress;				//播放进度
	short proflag;				//播放进度标记
	unsigned short musicTime;	//音乐总时长
	char playfilename[128];		//当前播放的地址
	char musicname[48];			//当前播放的歌曲名字		
}Player_t;

typedef struct{
	char channel;					//采样通道
	unsigned char wait;
	unsigned short rate;			//采样率	
	int bitrate;					//播放比特率
	int playSize;					//当前播放大小
	int cacheSize;					//当前下载音频数据大小
	unsigned int streamLen;			//数据流大小
	Player_t player; 
	pthread_mutex_t mutex;			//资源锁
	void (*SetI2SRate)(int rate,const char *function);	//切换音频采样率
	int (*GetVol)(void);			//获取音量大小	
	void (*ack_playCtr)(int nettype,Player_t *player,unsigned char playState);//回复给app状态
	void (*GetWm8960Rate)(void);
	char mp3name[128];
#ifdef MEM_PLAY	
	char *streamdata;				//音频数据流	
#endif	
	FILE *wfp;						//缓存文件指针				
	FILE *rfp;						//播放文件指针
}Mp3Stream;
extern Mp3Stream *st;

//#define STREAM_DEBUG(fmt, args...)	printf("%s: "fmt,__func__, ## args)

//#define DBG_STREAM
#ifdef 	DBG_STREAM
#define DEBUG_STREAM(fmt, args...) printf("%s: " fmt,__func__, ## args)
#else   
#define DEBUG_STREAM(fmt, args...) { }
#endif

extern void NetStreamExitFile(void);	//退出播放,耗时退出
extern int Mad_PlayMusic(Player_t *play);	//播放URL接口，本地存在就不下载了
extern void StreamPause(void);
extern void StreamPlay(void);
extern void SetStreamPlayState(unsigned char playliststate);
extern int GetStreamPlayState(void);
extern void keyStreamPlay(void);

extern void getStreamState(void *data,void StreamState(void *data,Player_t *player));	//获取播放流状态
//初始化 ，WritePcmData写入音频数据流回调函数  SetI2SRate 设置采样率回调函数
extern void initStream(void ack_playCtr(int nettype,Player_t *player,unsigned char playState),void WritePcmData(char *data,int size),void SetI2SRate(int rate,const char *function),int GetVol(void),void GetWm8960Rate(void));
extern void cleanStream(void);//清除
#endif
