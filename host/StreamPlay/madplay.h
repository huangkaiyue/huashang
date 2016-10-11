#ifndef _MADPLAY_H
#define _MADPLAY_H

#define MAD_EXIT			0	//停止	
#define MAD_PLAY 			1	//播放
#define MAD_PAUSE			2 	//暂停
#define MAD_NEXT			3

#define STREAM_START	0
#define STREAM_END		1
#define ENABLE_STREAM	1
#define DISABLE_STREAM	0


#define BUFSIZE 			8192//mp3一帧最大值
typedef struct buffer {
	unsigned char err_tices;		//解码出错次数
	unsigned char playstate:4,
		streamState:4;		//播放状态
	unsigned short fbsize; 			/*indeed size of buffer*/
    unsigned int flen; 				/*file length*/
    unsigned int fpos; 				/*current position*/
    unsigned char fbuf[BUFSIZE+1]; 	/*buffer*/
	void (*WritePcmVoices)(char *data,int size);
	//读入音频数据大小，msg 存放读入数据， size 读入数据大小
	void (*InputMusicStream)(char *msg,int size);		
}MadDecode_t;


//#define DEBUG_MAD
#ifdef DEBUG_MAD  
#define DEBUG_MADPLAY(fmt, args...)	printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_MADPLAY(fmt, args...) { }  
#endif  

//#define DEBUG_MAD_ERR
#ifdef DEBUG_MAD_ERR  
#define DEBUG_MADPLAY_ERR(fmt, args...)	printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_MADPLAY_ERR(fmt, args...) { }  
#endif 

extern void DecodeStart(void);
extern void DecodePause(void);
extern void DecodeExit(void);
extern int GetDecodeState(void);
extern void SetDecodeSize(int fileLen);

extern void DecodePlayMusic(void InputMusicStream(char *msg,int size));
extern void InitDecode(void WritePcmData(char *data,int size));
extern void CleanDecode(void);

#endif
