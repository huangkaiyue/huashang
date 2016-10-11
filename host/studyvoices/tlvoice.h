#ifndef _TULING_VOICE_
#define _TULING_VOICE_

#ifdef __cplusplus
extern "C"{
#endif

	/**
	audio：音频数据，经过base64编码
	len：原始数据长度，单位字节，长度不能超过20*1024
	rate：采样率，支持8000、16000，默认8000
	format：音频格式，支持pcm、wav、amr、opus、speex、x-flac，默认wav
	key：图灵接口key值
	result：返回的音频数据，默认mp3
	resSize：返回音频数据长度
	**/
	//int tl_req_voice(char *audio, int len, int rate, char *format, char *key, char **result, int *resSize);
	int tl_req_voice(char *audio, int len, int rate, char *format, char *key, char **result, int *resSize,char **szText, int *iTextSize );
	int tuling_voices(char *srcdata,int srclen,char **result,int *resSize,char **text,int *textSize);


#ifdef __cplusplus
}
#endif
#endif // !_TULING_VOICE_
