#ifndef _INIT_I2S_H
#define _INIT_I2S_H

#include "i2s_ctrl.h"
#include "config.h"

#define RECODE_RATE 			8000 
#define AUDIO_RX_VOICE 			118  	//录音大小
#ifdef TEST_SDK
#define AUDIO_TX_VOICE 			100
#else
#define AUDIO_TX_VOICE 			115		//音乐播放音量大小
#endif
#define PLAY_MODE				0 		//播放模式
#define RECORD_MODE				1  		//录音模式
#define NONE_MODE				9		//普通模式
#define	EXTERNAL_LBK2			14		//双工模式

#define THRESHOLD 				600		//音频信号阈值
#define RSIZE    				8    	//


#define VOL_SUB 0
#define VOL_ADD 1
#define VOL_SET 2


extern void *shrxbuf[MAX_I2S_PAGE];
extern char play_buf[I2S_PAGE_SIZE+4];


typedef struct {
	unsigned char rx_enable:1,		//接收使能
	tx_enable:1,					//发送使能
	execute_mode:6;				//播放模式
#ifdef VOICS_CH
	unsigned char vol_ch;			//播音人的选择
#endif //end VOICS_CH
	unsigned char tx_vol;			//播放音量大小
	short i2s_fd;					//打开音频设备节点描述符
	unsigned short tx_rate;			//当前播放采样率
	unsigned short play_size;
	unsigned short qttspos;
	unsigned char qttsend;
	unsigned char old_vol:7,cache_vol:1;//tang : change 2015-12-2 for save vol
}I2SST;

extern I2SST I2S;

#define MUTE 	0
#define UNMUTE 	1

#define SET_RATE(i2s_fd,rate) 	ioctl(i2s_fd, I2S_SRATE, rate)//采样率设置
#define SET_RX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_RX_VOL, vol)//录音音量
#define SET_TX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_TX_VOL, vol)//播放音量
#define SET_TX_ENABLE(i2s_fd) 	ioctl(i2s_fd, I2S_TX_ENABLE, 0)
#define SET_TX_DISABLE(i2s_fd) 	ioctl(i2s_fd, I2S_TX_DISABLE, 0)
#define SET_RX_ENABLE(i2s_fd) 	ioctl(i2s_fd, I2S_RX_ENABLE, 0)
#define SET_RX_DISABLE(i2s_fd) 	ioctl(i2s_fd, I2S_RX_DISABLE, 0)

extern void write_pcm(char *buf);
extern void WritePcmData(char *data,int size);
extern void WriteqttsPcmData(char *data,int len);
extern int SetVol(int dir,int vol);
extern int GetVol(void);
extern void Mute_voices(unsigned char stat);
extern int i2s_start_play(unsigned short rate);
extern char *i2s_get_data(void);
extern void i2s_destory_voices(void);
extern void __init_wm8960_voices(void);
#ifdef VOICS_CH
extern int get_volch(void);
extern void set_volch(unsigned char ch);
#endif //end VOICS_CH


#endif

