#ifndef _MADPLAY_H
#define _MADPLAY_H

#define MAD_EXIT			0	//Í£Ö¹	
#define MAD_PLAY 			1	//²¥·Å
#define MAD_PAUSE			2 	//ÔÝÍ£
#define MAD_NEXT			3	//ÏÂÒ»Çú

//#define DEBUG_MAD
#ifdef DEBUG_MAD  
#define DEBUG_MADPLAY(fmt, args...)	printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_MADPLAY(fmt, args...) { }  
#endif  

#define DEBUG_MAD_ERR
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

extern int DecodePlayMusic(void GetMusicMessage(int rate,int channels),void InputMusicStream(const void *msg,int size));
extern void InitDecode(void WritePcmData(char *data,int size));
extern void CleanDecode(void);

#endif
