#include "comshead.h"
#include "mad.h"
#include "madplay.h"

static MadDecode_t *Mad=NULL;
 
void DecodeStart(void)
{
	Mad->playstate=MAD_PLAY;
}
void DecodePause(void)
{
	Mad->playstate=MAD_PAUSE;
}
void DecodeExit(void)
{
	Mad->playstate=MAD_EXIT;
}
int GetDecodeState(void)
{
	return (int )Mad->playstate;
}

void SetDecodeSize(int fileLen)
{
	Mad->flen =fileLen;
}
//同一个位置解码出错超过30次。表明为无效音频
#define DECODE_ERROR_NUM	60 
static enum mad_flow NewInput(void *data,struct mad_stream *stream)
{
    MadDecode_t *Mad =(MadDecode_t *)data;
    int ret_code;
    int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
	switch(Mad->playstate){
		case MAD_EXIT:
			ret_code = MAD_FLOW_STOP;
			break;
		case MAD_PAUSE:
			usleep(10000);
			break;
		default:
			/*mp3根据每一完整的帧才能解码出来*/
			if( Mad->flen>0)	//已经拿到数据流的大小
			{	
				//printf("Mad->flen =fileLen =%d\n",Mad->flen );
				if(Mad->fpos >= Mad->flen)
				{
					ret_code = MAD_FLOW_STOP;
					break;
				}
				unproc_data_size = stream->bufend - stream->next_frame;
				memcpy(Mad->fbuf, Mad->fbuf+Mad->fbsize-unproc_data_size, unproc_data_size);
				copy_size = BUFSIZE - unproc_data_size;
				if(Mad->fpos + copy_size > Mad->flen)
				{
					copy_size = Mad->flen - Mad->fpos;
				}
			}
			else{
				unproc_data_size = stream->bufend - stream->next_frame;
				memcpy(Mad->fbuf, Mad->fbuf+Mad->fbsize-unproc_data_size, unproc_data_size);
				copy_size = BUFSIZE - unproc_data_size;
			}
			Mad->InputMusicStream((char *)(Mad->fbuf+unproc_data_size),copy_size);
			Mad->fbsize = unproc_data_size + copy_size;
			Mad->fpos += copy_size;
			/*Hand off the buffer to the mp3 input stream*/
			mad_stream_buffer(stream, Mad->fbuf,Mad->fbsize);
			ret_code = MAD_FLOW_CONTINUE;
			break;
	}
    return ret_code;
}

static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L <= MAD_F_FRACBITS - 16);
    if(sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if(sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow output(void *data,struct mad_header const *header,struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples;
//    unsigned int rate;
	signed int sample;
    mad_fixed_t const *left_ch, *right_ch;
	char buf[4];
    /* pcm->samplerate contains the sampling frequency */
//    rate = pcm->samplerate;
    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];

    while (nsamples--)
    {
        /* output sample(s) in 16-bit signed little-endian PCM */
		sample = scale(*left_ch++);
		buf[0]=(sample >> 0) & 0xff;
		buf[1]=(sample >> 8) & 0xff;
		if(nchannels==1)
		{
			//将左声道数据写到右声道当中
			buf[2]=buf[0];
			buf[3]=buf[1];
			Mad->WritePcmVoices(buf,4);
		}else{
            sample = scale(*right_ch++);
			buf[2]=(sample >> 0) & 0xff;
			buf[3]=(sample >> 8) & 0xff;
			Mad->WritePcmVoices(buf,4);
		}
    }
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
   MadDecode_t *mp3 = (MadDecode_t *)data;
    DEBUG_MADPLAY_ERR("decoding error 0x%04x (%s) at byte offset %u\n",stream->error, mad_stream_errorstr(stream),stream->this_frame - mp3->fbuf);
	if(Mad->err_tices++>DECODE_ERROR_NUM)
	{
		Mad->err_tices=0;
		//DecodeExit();
	}
    return MAD_FLOW_CONTINUE;
}
//-------------------------------------
int get_playstate(void){
	return Mad->playstate;
}

void DecodePlayMusic(void InputMusicStream(char *msg,int size))
{
    struct mad_decoder decoder;
    int result;

	Mad->InputMusicStream = InputMusicStream;
	Mad->playstate = MAD_PLAY;
	//开始播放需要读入8k数据
	usleep(200);
	if(Mad->playstate == MAD_EXIT){
		return;
	}
	Mad->InputMusicStream((char *)Mad->fbuf,BUFSIZE);
	/* configure input, output, and error functions */
    mad_decoder_init(&decoder, Mad,NewInput, 0 /* header */, 0 /* filter */, output,error, 0 /* message */);
    /* start decoding */
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    /* release the decoder */
    mad_decoder_finish(&decoder);
	Mad->playstate = MAD_EXIT;
	Mad->fpos=0;
	Mad->flen=0;
    return result;
}
void InitDecode(void WritePcmData(char *data,int size))
{
	Mad = (MadDecode_t *)calloc(1,sizeof(MadDecode_t));
	if(Mad==NULL)
	{
		DEBUG_MADPLAY_ERR("calloc Mad failed \n");
	}
	Mad->WritePcmVoices=WritePcmData;
}
void CleanDecode(void)
{
	if(Mad)
	{
		Mad->playstate =MAD_EXIT;
		free(Mad);
		Mad=NULL;
	}
}
