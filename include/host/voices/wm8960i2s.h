#ifndef _INIT_I2S_H
#define _INIT_I2S_H

#include "i2s_ctrl.h"
#include "config.h"

#define RECODE_RATE 			8000 
#define AUDIO_RX_VOICE 			118  	//¼����С
#ifdef TEST_SDK
#define AUDIO_TX_VOICE 			100
#else
#define AUDIO_TX_VOICE 			115		//���ֲ���������С
#endif
#define PLAY_MODE				0 		//����ģʽ
#define RECORD_MODE				1  		//¼��ģʽ
#define NONE_MODE				9		//��ͨģʽ
#define	EXTERNAL_LBK2			14		//˫��ģʽ

#define THRESHOLD 				600		//��Ƶ�ź���ֵ
#define RSIZE    				8    	//


#define VOL_SUB 0
#define VOL_ADD 1
#define VOL_SET 2


extern void *shrxbuf[MAX_I2S_PAGE];
extern char play_buf[I2S_PAGE_SIZE+4];


typedef struct {
	unsigned char rx_enable:1,		//����ʹ��
	tx_enable:1,					//����ʹ��
	execute_mode:6;				//����ģʽ
#ifdef VOICS_CH
	unsigned char vol_ch;			//�����˵�ѡ��
#endif //end VOICS_CH
	unsigned char tx_vol;			//����������С
	short i2s_fd;					//����Ƶ�豸�ڵ�������
	unsigned short tx_rate;			//��ǰ���Ų�����
	unsigned short play_size;
	unsigned short qttspos;
	unsigned char qttsend;
	unsigned char old_vol:7,cache_vol:1;//tang : change 2015-12-2 for save vol
}I2SST;

extern I2SST I2S;

#define MUTE 	0
#define UNMUTE 	1

#define SET_RATE(i2s_fd,rate) 	ioctl(i2s_fd, I2S_SRATE, rate)//����������
#define SET_RX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_RX_VOL, vol)//¼������
#define SET_TX_VOL(i2s_fd,vol) 	ioctl(i2s_fd, I2S_TX_VOL, vol)//��������
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

