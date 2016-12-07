#ifndef _STREAMFILE_H
#define _STREAMFILE_H

#include <pthread.h>
#include <stdio.h>
#include "base/queWorkCond.h"

//#define TEST_SAVE

#define SAFE_READ_WRITER		//��ȫ�Ķ�д�ļ�

#define SEEK_TO					//�������϶�ʵ�ֿ��
//#define SHOW_progressBar		//��ʾ���Ž���

#define KB	1024
#define CACHE_PLAY_SIZE		8*KB
#define LIST_CACHE_SIZE		4*KB
#define BUF_TANG			16*KB

#define UDP_ACK		1
#define TCP_ACK		0

#define MP3STEAM_SIZE	sizeof(Mp3Stream)

typedef struct{
	unsigned char playState;
	unsigned char vol;			//������С	
	short progress;				//���Ž���
	short proflag;				//���Ž��ȱ��
	unsigned short musicTime;	//������ʱ��
	char playfilename[128];		//��ǰ���ŵ�url��ַ
	char musicname[48];			//��ǰ���ŵĸ�������		
}Player_t;

typedef struct{
	char channel;					//����ͨ��
	unsigned char wait;
	unsigned short rate;			//������	
	int bitrate;					//���ű�����
	int playSize;					//��ǰ���Ŵ�С
	int cacheSize;					//��ǰ������Ƶ���ݴ�С
	unsigned int streamLen;			//��������С
	Player_t player; 
	pthread_mutex_t mutex;			//��Դ��
	void (*SetI2SRate)(int rate);	//�л���Ƶ������
	int (*GetVol)(void);			//��ȡ������С	
	void (*ack_playCtr)(int nettype,Player_t *player,unsigned char playState);//�ظ���app״̬
	char mp3name[128];
#ifdef MEM_PLAY	
	char *streamdata;				//��Ƶ������	
#endif	
	FILE *wfp;						//�����ļ�ָ��				
	FILE *rfp;						//�����ļ�ָ��
}Mp3Stream;

//#define STREAM_DEBUG(fmt, args...)	printf("%s: "fmt,__func__, ## args)

//#define DBG_STREAM
#ifdef 	DBG_STREAM
#define DEBUG_STREAM(fmt, args...) printf("%s: " fmt,__func__, ## args)
#else   
#define DEBUG_STREAM(fmt, args...) { }
#endif

extern void NetStreamExitFile(void);	//�˳�����,��ʱ�˳�
extern void PlayUrl(const void *data);	//����URL�ӿڣ����ش��ھͲ�������
extern int NetStreamDownFilePlay(const void *data);//��ʼ���±߲��� ���� ��ʱ���أ���̨�̲߳���
extern void StreamPause(void);
extern void StreamPlay(void);
extern void getStreamState(void *data,void StreamState(void *data,Player_t *player));	//��ȡ������״̬

extern void playLocalMp3(const char *mp3file);	//���ű�����Ƶ�� 

//��ʼ�� ��WritePcmDataд����Ƶ�������ص�����  SetI2SRate ���ò����ʻص�����
extern void initStream(void ack_playCtr(int nettype,Player_t *player,unsigned char playState),void WritePcmData(char *data,int size),void SetI2SRate(int rate),int GetVol(void));
extern void cleanStream(void);//���
#endif
