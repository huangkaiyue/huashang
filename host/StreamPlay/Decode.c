#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mad.h>
#include "mplay.h"

struct mad_stream stream;
struct mad_frame frame;
struct mad_synth synth;

#define MAD_BLOCK_SIZE 8192
#define MAD_INBUF_SIZE MAD_BLOCK_SIZE + MAD_BUFFER_GUARD


#define END_NUM				15
typedef struct{
	unsigned char playstate:4,
		streamState:4;		//播放状态
	unsigned char EndFile;	
	unsigned char Inbuf[MAD_INBUF_SIZE];
	void (*outPcm)(char *msg,int size);
	int fpos;
	int flen;
}Mplayer_t;
Mplayer_t *Mad=NULL;
/*
@ 
@ 初始化解码器
@
*/
static void Mad_Decode_Init(void){
	mad_stream_init(&stream);
 	mad_frame_init(&frame);
 	mad_synth_init(&synth);
}
/*
@ 
@ 释放解码器
@
*/
static void Mad_Decode_Free(void){
	mad_synth_finish(&synth);
 	mad_frame_finish(&frame);
 	mad_stream_finish(&stream);
}
/*
@ 解码音频文件
@ InputMusicStream 函数指针,回调读入mp3文件数据 ，msg 存入的音频数据 size 需要读入大小 
@ 0 解码成功 -1 解码失败
*/
int DecodePlayMusic(void InputMusicStream(const void *msg,int size)){
	int i = 0;
 	unsigned char *in_buf;
 	int remaining, rlen;
 	unsigned int nchannels, nsamples;
	int ret=-1;
	int sampleofs=0,samplecnt=0;
	Mad_Decode_Init();
	Mad->playstate=MAD_PLAY;
 	while(1)
	{
		//printf("Mad->EndFile = %d\n",Mad->EndFile);
		if(Mad->playstate==MAD_EXIT)
			break;
		else if(Mad->playstate==MAD_PAUSE){
			printf("is pause music\n");
			usleep(10000);
			continue;
		}
  		if(samplecnt == 0)
  		{
   			if((stream.buffer == NULL)||(stream.error = MAD_ERROR_BUFLEN))
   			{
    				if(stream.next_frame != NULL)
    				{
     					remaining = stream.bufend - stream.next_frame;
     					memcpy(Mad->Inbuf, stream.next_frame, remaining);
	     				in_buf = Mad->Inbuf + remaining;
     					rlen = MAD_BLOCK_SIZE - remaining;
    				}
    				else
    				{ 
     					rlen = MAD_BLOCK_SIZE;
     					in_buf = Mad->Inbuf;
     					remaining = 0;
    				}
				if(Mad->EndFile==0){
						if(Mad->flen!=0&&Mad->fpos+rlen>=Mad->flen){
							printf("----------------Mad->fpos=%d Mad->flen=%d ------------\n",Mad->fpos+rlen,Mad->flen);
							printf("Mad->fpos = %d remaining=%d Mad->flen=%d\n",Mad->fpos,remaining,Mad->flen);
							Mad->EndFile=END_NUM;
							InputMusicStream((const void  *)in_buf,Mad->flen-Mad->fpos);
							continue;
						}
						InputMusicStream((const void  *)in_buf,rlen);
						Mad->fpos+=rlen;
   				}else{
					if(--Mad->EndFile==0){
						printf("remaining=%d rlen=%d\n",remaining,rlen);
						break;
					}
				}
    				mad_stream_buffer(&stream, Mad->Inbuf, rlen + remaining);
    				stream.error = MAD_ERROR_NONE;
   			}
	   		if(mad_frame_decode(&frame, &stream) != 0)
   			{
    				if(MAD_RECOVERABLE(stream.error)||(stream.error == MAD_ERROR_BUFLEN)) 
    					continue;
	    			ret=-1;
				goto exit;
   			}  
   	
   			mad_synth_frame(&synth, &frame);
   
	   		nchannels = synth.pcm.channels;
   			nsamples  = synth.pcm.length;
			
   			if(nchannels == 2)
   			{
    				for(i = 0; i < nsamples; i++)
    				{
			     		if(synth.pcm.samples[0][i] >= MAD_F_ONE)synth.pcm.samples[0][i] = MAD_F_ONE - 1;
					if(synth.pcm.samples[0][i] < -MAD_F_ONE)synth.pcm.samples[0][i] = -MAD_F_ONE;
			     		synth.pcm.samples[0][i] = synth.pcm.samples[0][i] >> (MAD_F_FRACBITS + 1 - 16 + 1);
     
			     		if(synth.pcm.samples[1][i] >= MAD_F_ONE)synth.pcm.samples[1][i] = MAD_F_ONE - 1;
			     		if(synth.pcm.samples[1][i] < -MAD_F_ONE)synth.pcm.samples[1][i] = -MAD_F_ONE;
			     		synth.pcm.samples[1][i] = synth.pcm.samples[1][i] >> (MAD_F_FRACBITS + 1 - 16 + 1);     
    				}
   			}
   			else
   			{
				for(i = 0; i < nsamples; i++)
    				{
			     		if(synth.pcm.samples[0][i] >= MAD_F_ONE)synth.pcm.samples[0][i] = MAD_F_ONE - 1;
			     		if(synth.pcm.samples[0][i] < -MAD_F_ONE)synth.pcm.samples[0][i] = -MAD_F_ONE;
					synth.pcm.samples[0][i] = synth.pcm.samples[0][i] >> (MAD_F_FRACBITS + 1 - 16 + 1);
			     		synth.pcm.samples[1][i] = synth.pcm.samples[0][i];
    				}
   			}
   			sampleofs = 0;
			samplecnt = nsamples;
  		}
		rlen = samplecnt;
		//printf("rlen = %d\n",rlen);
		char buf[4];
  		for(i = 0; i < rlen; i++)
  		{	
			*(short *)(buf  + 0) = synth.pcm.samples[0][sampleofs];
   			*(short *)(buf  + 2) = synth.pcm.samples[1][sampleofs];	
			Mad->outPcm(buf,4);
   			sampleofs++;
   			samplecnt--;
  		}
 	}
exit:	
	Mad->flen=0;
	Mad->EndFile=0;
	Mad->fpos=0;
	Mad->playstate=MAD_EXIT;
	Mad_Decode_Free();	
	printf("exit play .................\n");
 	return ret;

}
void DecodeStart(void){
	Mad->playstate=MAD_PLAY;
}
void DecodePause(void){
	Mad->playstate=MAD_PAUSE;
}
void DecodeExit(void){
	Mad->playstate=MAD_EXIT;
}
int GetDecodeState(void){
	return (int )Mad->playstate;
}
int get_playstate(void){
	return Mad->playstate;
}

void SetDecodeSize(int fileLen){
	Mad->flen =fileLen;
}
void InitDecode(void WritePcmData(char *data,int size)){
	Mad = (Mplayer_t*)calloc(1,sizeof(Mplayer_t));
	if(Mad==NULL){
		return -1;
	}
	Mad->outPcm = WritePcmData;
 	Mad_Decode_Init();
}
void CleanDecode(void){
	if(Mad){
		Mad->playstate =MAD_EXIT;
		free(Mad);
		Mad=NULL;
	}
}
