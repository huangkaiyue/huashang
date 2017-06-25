#include <stdio.h>  
#include <string.h>
#include <bits/types.h>
#include "DemoMp3head.h"
typedef __uint32_t uint32_t;
  
enum {  
    BITRATE_MPEG1,  
    BITRATE_MPEG2,  
    BITRATE_NUM  
};  
  
  
enum {  
    SAMPLERATE_MPEG1,  
    SAMPLERATE_MPEG2,  
    SAMPLERATE_MPEG25,  
    SAMPLERATE_NUM  
};  
  
#define MP3_FRAME_SYNC          0xFFE00000  
  
#define MASK_VERSION            0x00180000  
#define SHIFT_VERSION           19  
  
#define MPEG_1              3  
#define MPEG_2              2  
#define MPEG_25             0  
  
#define SHIFT_LAYER         17  
#define LAYER_3             1  
#define LAYER_2             2  
#define LAYER_1             3  
  
#define PROTECTION_BIT          0x00010000  
  
#define MASK_BITRATE            0x0000f000  
#define SHIFT_BITRATE           12  
  
  
#define MASK_SAMPLERATE         0x00000C00  
#define SHIFT_SAMPLERATE        10  
  
#define MASK_PADDING            0x00000200  
  
#define MASK_CHANNEL                0x000000C0  
#define CHANNEL_STEREO              0x00000000  
#define CHANNEL_JOINT               0x00000040  
#define CHANNEL_DUAL                0x00000080  

#define CHANNEL_MONO                0x000000C0  
  
struct mp3_frame {  
    uint32_t version;  
    uint32_t channel;  //通道
    int bitrate;  
    int samplingRate;	//采样率  
    int samples;  
    int size;  
    const unsigned char *data;  
};  
// bit率   128K bit/s    
static int bitrateTbl[16][BITRATE_NUM] =  
{  
    { -1,  -1},  
    { 32,   8},  
    { 40,  16},  
    { 48,  24},  
    { 56,  32},  
    { 64,  40},  
    { 80,  48},  
    { 96,  56},  
    {112,  64},  
    {128,  80},  
    {160,  96},  
    {192, 112},  
    {224, 128},  
    {256, 144},  
    {320, 160},  
    { -1,  -1}  
};  
  
// 采样率  
static unsigned short samplingRateTbl[3][SAMPLERATE_NUM] =  
{  
    { 44100, 22050, 11025 },  
    { 48000, 24000, 12000 },  
    { 32000, 16000,  8000 }  
};  
  
// 检查帧头信息  
static int checkHdr(uint32_t hdr)  
{  
    if( (hdr & MP3_FRAME_SYNC) != MP3_FRAME_SYNC  
        || ((hdr>>SHIFT_VERSION) & 0x3) == 1  
        || ((hdr>>SHIFT_LAYER) & 0x3) == 0  
        || ((hdr>>SHIFT_BITRATE) & 0xf) == 0xf   
        || ((hdr>>SHIFT_BITRATE) & 0xf) == 0  
        || ((hdr>>SHIFT_SAMPLERATE)&0x3) == 0x3  
        || (hdr & 0xffff0000) == 0xfffe0000)  
        return 0;  
    return 1;  
}  
  
#define READ_ERROR(ret) if((ret) < 0) return (ret);  
  
static int READ_CHAR(FILE * fp, uint32_t * v)  
{  
    char buf[1];  
  
    if(fread(buf, 1, 1, fp) <= 0)  
    {
        return -1; 
        }
  
    *v = 0xff & buf[0];  
    return 0;  
}  
  
static int READ_UINT16(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
  
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}  
  
static int READ_UINT24(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_UINT16(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
      
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}  
  
static int READ_UINT32(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_UINT24(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
      
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}                                     

static int getMp3Frame(struct mp3_frame * frame,uint32_t hdr)
{
    uint32_t  br_idx, sr_idx;     
    int ret;  
    int factor;   

    frame->version = (hdr >> SHIFT_VERSION) & 0x3;  
    br_idx = (hdr >> SHIFT_BITRATE) & 0xf;  
    sr_idx = (hdr >> SHIFT_SAMPLERATE) & 0x3;  
  
    switch(frame->version)  
    {  
    case MPEG_25:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG2];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG25];  
        factor = 72;  
        break;  
    case MPEG_2:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG2];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG2];  
        factor = 72;  
        break;  
    case MPEG_1:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG1];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG1];  
        factor = 144;  
        break;  
    default:  
        printf("invalid header %x\n", hdr);  
        return -1;  
    }  
  
    frame->size = (factor * frame->bitrate * 1000) / frame->samplingRate;    
    if(frame->size == 0)  
    {  
        printf("size == 0\n");  
        return -1;  
    }  
  
    if((hdr & MASK_PADDING) != 0)  
        frame->size++;  
  
    if(frame->samplingRate > 32000)  
        frame->samples = 1152;  
    else  
        frame->samples = 576;  
  
    frame->channel = hdr & MASK_CHANNEL;  
    frame->channel = frame->channel>>6;
	return 0;
}

/***********************************************************
解析mp3帧，获取mp3头信息 
***********************************************************/ 
static int getNextFrame(FILE * mp3fp, struct mp3_frame * frame)  
{  
    	uint32_t hdr;     
    	int ret;  
    	while (1){  
        	ret = READ_UINT32(mp3fp, &hdr);  
        	if (ret < 0)  
            		break;  
        	if (checkHdr(hdr))  
            		break;  
        	fseek(mp3fp, -3, 1);  
    	} 
      
	if(getMp3Frame(frame,hdr))
	{
		ret=-1;
	}
    	fseek(mp3fp, 0, SEEK_SET);  
  
    	return ret;  
}  

/*****************************************************
获取采样率和通道数rate:采样率channel:通道
*****************************************************/
int DemoGetMp3head(FILE *fp,Mp3Demo_t *mp)
{
	if(fp ==NULL){
		return -1;
	}
	struct mp3_frame pframe;
    	getNextFrame(fp,&pframe);    
	mp->rate = pframe.samplingRate;
	mp->bitrate = pframe.bitrate*1000;
    	if(pframe.channel == 0x03)
		mp->channel = 1;
    	else
		mp->channel = 2;
	return 0;
}

static int READ_8(char * data, uint32_t * v)  
{   
    *v = 0xff & data[0];  
    return 0;  
}  
  
static int READ_16(char *data, uint32_t * v)  
{  
    uint32_t tmp;  

	READ_8(data, &tmp);  
    *v = (tmp << 8);  
  
    READ_8(data+1, &tmp);  
    *v |= (tmp);  
  
    return 0;  
}  
  
static int READ_24(char *data, uint32_t * v)  
{  
    uint32_t tmp;  
    READ_16(data, &tmp);  
    *v = (tmp << 8);  
      
    READ_8(data+2, &tmp);  
    *v |= (tmp);  

    return 0;  
}  
  
static int READ_32(char *data, uint32_t * v)  
{  
    uint32_t tmp;  
  
    READ_24(data, &tmp);  
    *v = (tmp << 8);  
      
    READ_8(data+3, &tmp);  
    *v |= (tmp);  
  
    return 0;  
}                                     


/***********************************************************
解析mp3帧，获取mp3头信息 
***********************************************************/ 
static int getFramedata(char *data, int length,struct mp3_frame * frame)  
{  
    uint32_t hdr ;     
    int pos=0;   
    while (1) {  
    	READ_32(data+pos,&hdr);
        if (checkHdr(hdr)){
            break;  
        }
		if(++pos>length)
		{
			printf("not hdr \n");
			return -1;
		}
    }  
    return getMp3Frame(frame, hdr);  
}  

int DemoGetmp3Steamer(char *mp3data,int length,Mp3Demo_t *mp){
	struct mp3_frame pframe;
	memset(&pframe,0,sizeof(struct mp3_frame));
   	if(getFramedata(mp3data,length,&pframe)){
		return -1;
	}
	mp->rate = pframe.samplingRate;
   	mp->bitrate = pframe.bitrate*1000;
	
	if(pframe.channel == 0x03)
		mp->channel = 1;
    else
		mp->channel = 2;
	
	return 0;
}

int DemoGetmp3Toalltime(int bitrate,int fileLen)
{
	return fileLen/bitrate*8;
}

#if 0
int main(int argc,char **argv)
{
	if(argc<2)
	{
		printf("please input music file\n");
		return -1;
	}
	FILE *fp = fopen(argv[1],"r");
	if(fp==NULL)
	{
		perror("open failed\n");
		return -1;
	}
	Mp3_t *mp3 = DemoGetMp3head(fp);
	printf("This song samplingRate is %dHz channel %d bitrate =%d\n",mp3->rate,mp3->channel,mp3->bitrate);
	fclose(fp);
	char mp3data[3000]={0};
	fp = fopen(argv[1],"r");
	fread(mp3data,3000,1,fp);
	mp3 = Getmp3Steamer(mp3data,3000);
	printf("\nThis song samplingRate is %dHz channel %d bitrate =%d\n",mp3->rate,mp3->channel,mp3->bitrate);
	fclose(fp);
	return 0;
}
#endif
