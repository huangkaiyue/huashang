#ifndef _DEMOMP3HEAD_H_  
#define _DEMOMP3HEAD_H_  
    
typedef struct{
	unsigned short rate;
	unsigned char channel;
	int samplingRate;
	int bitrate;
}Mp3Demo_t;
  
extern int DemoGetMp3head(FILE *fp,Mp3Demo_t *mp);
extern int DemoGetmp3Steamer(char *mp3data,int length,Mp3Demo_t *mp);
extern int DemoGetmp3Toalltime(int bitrate,int fileLen);

#endif // _MP3_H_  

