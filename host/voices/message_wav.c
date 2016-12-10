#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/callvoices.h"
#include "host/studyvoices/prompt_tone.h"
#include "host/voices/WavAmrCon.h"
#include "config.h"

#if 0
void playWavVoices(char *path)
{
	int size=0;
	FILE *fp= fopen(path,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return;
	}
#ifdef TULIN_WINT_MUSIC
	if(strstr(path,"TuLin_Wint_8K")){
		size=fread(play_buf,1,I2S_PAGE_SIZE,fp);
		memset(play_buf,0,size);
	}
#endif
	start_event_play_wav();
	while(1)
	{
		size= fread(play_buf,1,I2S_PAGE_SIZE,fp);
		if(size==0){
			write_pcm(play_buf);
			usleep(1000);
			break;
		}
		write_pcm(play_buf);
		memset(play_buf,0,size);
	}
	fclose(fp);
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
}
#else
void playAmrVoices(const char *filename)
{
	char *outfile ="speek.wav";
	AmrToWav8k(filename,(const char *)outfile);
	int r_size=0,pos=0;
	char readbuf[2];
	FILE *fp= fopen(outfile,"r");
	if(fp==NULL){
		printf("open sys failed \n");
		return;
	}
	start_event_play_wav();
	fseek(fp,WAV_HEAD,SEEK_SET);//����wavͷ��	

#ifdef TULIN_WINT_MUSIC
	if(strstr(filename,"TuLin_Wint_8K")){
		r_size=fread(play_buf,1,I2S_PAGE_SIZE/2,fp);
		memset(play_buf,0,r_size);
	}
#endif
	playsysvoicesLog("playsys amr start \n");
	while(1){
		r_size= fread(readbuf,1,2,fp);
		if(r_size==0){
			if(pos>0){
				memset(play_buf+pos,0,I2S_PAGE_SIZE-pos);//�����һ��β������
				write_pcm(play_buf);
			}
			break;
		}
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		memcpy(play_buf+pos,readbuf,2);
		pos+=2;
		if(pos==I2S_PAGE_SIZE){
			playsysvoicesLog("playsys amr while \n");
			write_pcm(play_buf);
			pos=0;
		}
	}
	playsysvoicesLog("playsys amr end \n");
	fclose(fp);
	memset(play_buf,0,pos);
	stait_qtts_cache();
	if(strstr(filename,"TuLin_Wint_8K")){
		return ;
	}
	clean_play_cache();
	remove(outfile);
}
#endif
#ifdef SPEEK_VOICES
/********************************************************
@ ���Ž��յ��ֻ����͵ĶԽ���Ϣ
@ filename:���浽���ص�wav���ݵ��ļ�·�� 	
@
*********************************************************/
void playspeekVoices(const char *filename){
	i2s_start_play(8000);
	playAmrVoices(filename);
#ifdef CLOSE_VOICE
	sleep(2);
	Mute_voices(MUTE);
#endif	//end CLOSE_VOICE
}
#endif
void playsysvoices(char *filePath){
	if(strstr(filePath,"no_voices_8K")){
	}else{
		while(get_qtts_cache());	//fix me
		sleep(1);
	}
	play_sys_tices_voices(filePath);
	pause_record_audio();
}

/********************************************************
@ ��������:	����ϵͳ��
@ filePath:	·��
@ ����ֵ: ��
*********************************************************/
void play_sys_tices_voices(char *filePath)
{
	char path[128];
	snprintf(path,128,"%s%s",sysMes.sd_path,filePath);
	i2s_start_play(8000);
#ifdef CLOSE_VOICE
#ifdef DATOU_JIANG
	if(strstr(path,"TuLin_Wint_8K")){
		mute_recorde_vol(107);
	}else{
		mute_recorde_vol(UNMUTE);
	}
#endif
#endif	//end CLOSE_VOICE
#if 1
	playsysvoicesLog("playsys voices start \n");
	playAmrVoices(path);
	playsysvoicesLog("playsys voices end \n");
#else
	playWavVoices(path);
#endif
	if(strstr(filePath,"TuLin_Wint_8K")){
		return;
	}
	usleep(800*1000);
	if(strstr(filePath,"request_failed_8k")||strstr(filePath,"no_voices_8K")||strstr(filePath,"40002_8k")||strstr(filePath,"no_music_8K")||strstr(filePath,"TuLin_Hahaxiong_8K")){
		pause_record_audio();	//�˳�����״̬
	}
#ifdef CLOSE_VOICE
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
void exitqttsPlay(void){
	clean_qtts_cache();
	__exitqttsPlay();
}
/********************************************************
@ ��������:	����QTTS����
@ text:�ı�		type:�ı�����
@ ����ֵ: ��
*********************************************************/
void PlayQttsText(char *text,unsigned char type)
{
	i2s_start_play(8000);
#if 0
	char *textbuf= (char *)calloc(1,strlen(text)+2);
	if(textbuf==NULL){
		perror("calloc error !!!");
		return;
	}
	sprintf(textbuf,"%s%s",text,",");	//�ı�β�����",",��֤�ı���������
	tolkLog("tolk qtts start\n");
	stait_qtts_cache();
	Qtts_voices_text(textbuf,type);
	free(textbuf);
#else
	Qtts_voices_text(text,type);
#endif
	tolkLog("tolk qtts end\n");
	//printf("qttspos = %d qttsend = %d\n",I2S.qttspos,I2S.qttsend);
	if(I2S.qttspos!=0&&I2S.qttsend==0)
	{
		memset(play_buf+I2S.qttspos,0,I2S_PAGE_SIZE-I2S.qttspos);
		write_pcm(play_buf);
		I2S.qttspos =0;
	}
	tolkLog("tolk qtts clean\n");
	clean_play_cache();
	usleep(800*1000);
	tolkLog("tolk qtts pause\n");
	pause_record_audio();	//�˳�����״̬
#ifdef CLOSE_VOICE
	if(I2S.qttsend==1){
		Mute_voices(MUTE);
		return;
	}
	usleep(1000*1000);
	Mute_voices(MUTE);
#endif
}
