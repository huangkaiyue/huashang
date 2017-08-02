#ifndef _INIT_I2S_H
#define _INIT_I2S_H

#include "i2s_ctrl.h"
#include "config.h"


#define WM8960_NODE_PATH	"/dev/i2s0"

#define RECODE_RATE 			8000 			//录音采样率
#define AUDIO_RX_VOICE 			108  			//录音大小

#define AUDIO_TX_VOICE 			115				//音乐播放音量大小
#define PLAY_MODE				0 				//播放模式
#define RECORD_MODE				1  				//录音模式
#define NONE_MODE				9				//普通模式
#define	EXTERNAL_LBK2			14				//双工模式

#define VOL_UP					125				//音量上限
#define VOL_DWON				100				//音量下限
#define VOL_NUM					3				//每次增加
#define VOL_SET_DATA(x) 		(x/5)+VOL_DWON	//APP设置值算法

#define	SYSTEM_DEFALUT_VOL		110				//设置开机默认初始值
#define PLAY_PASUSE_VOICES_VOL	40				//播放智能会话过渡音大小		

#define VOL_SUB 				0
#define VOL_ADD 				1
#define VOL_APP_SET 			2
#define VOL_SET_VAULE			3

extern char play_buf[I2S_PAGE_SIZE+4];


typedef struct {
	unsigned char lockSetRate;		//对切换采样率进行上锁
	unsigned char rx_enable:1,		//接收使能
	tx_enable:1,					//发送使能
	execute_mode:6;					//播放模式
	unsigned char tx_vol;			//播放音量大小
	short i2s_fd;					//打开音频设备节点描述符
	unsigned short tx_rate;			//当前播放采样率
	unsigned short play_size;		//原始数据播放大小，当达到I2S_PAGE_SIZE ,才能往内核里面写
}I2SST;

extern I2SST I2S;

#define MUTE 	0
#define UNMUTE 	1

#define SET_RATE(i2s_fd,rate) 	ioctl(i2s_fd, I2S_SRATE, rate)	//采样率设置
#define SET_RX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_RX_VOL, vol)	//录音音量
#define SET_TX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_TX_VOL, vol)	//播放音量
#define SET_TX_ENABLE(i2s_fd) 	ioctl(i2s_fd, I2S_TX_ENABLE, 0)	//使能发送音频数据
#define SET_TX_DISABLE(i2s_fd) 	ioctl(i2s_fd, I2S_TX_DISABLE, 0)//关闭使能
#define SET_RX_ENABLE(i2s_fd) 	ioctl(i2s_fd, I2S_RX_ENABLE, 0) //使能接收
#define SET_RX_DISABLE(i2s_fd) 	ioctl(i2s_fd, I2S_RX_DISABLE, 0)//关闭使能

extern void write_pcm(char *buf);
extern void WritePcmData(char *data,int size);
extern void CleanI2S_PlayCachedata(void);
extern int Setwm8960Vol(int dir,int vol);
extern int GetVol(void);
extern void Mute_voices(unsigned char stat);
extern void mute_recorde_vol(int change);
extern int GetLockRate(void);
extern int GetWm8960Rate(void);
extern int SetWm8960Rate(unsigned short rate,const char *function);
extern char *I2sGetvoicesData(void);
extern void DestoryWm8960Voices(void);
extern void InitWm8960Voices(void);

#endif

