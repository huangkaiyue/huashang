#ifndef _TULING_VOICE_
#define _TULING_VOICE_

#ifdef __cplusplus
extern "C"{
#endif

	/**
	audio����Ƶ���ݣ�����base64����
	len��ԭʼ���ݳ��ȣ���λ�ֽڣ����Ȳ��ܳ���20*1024
	rate�������ʣ�֧��8000��16000��Ĭ��8000
	format����Ƶ��ʽ��֧��pcm��wav��amr��opus��speex��x-flac��Ĭ��wav
	key��ͼ��ӿ�keyֵ
	result�����ص���Ƶ���ݣ�Ĭ��mp3
	resSize��������Ƶ���ݳ���
	**/
	//int tl_req_voice(char *audio, int len, int rate, char *format, char *key, char **result, int *resSize);
	int tl_req_voice(char *audio, int len, int rate, char *format, char *key, char **result, int *resSize,char **szText, int *iTextSize );
	int tuling_voices(char *srcdata,int srclen,char **result,int *resSize,char **text,int *textSize);


#ifdef __cplusplus
}
#endif
#endif // !_TULING_VOICE_
