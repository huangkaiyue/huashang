#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "linux/autoconf.h" //kernel config
#include "host/voices/wm8960i2s.h"
#include "config.h"

#define WM8960_NODE_PATH	"/dev/i2s0"

void *shtxbuf[MAX_I2S_PAGE];
void *shrxbuf[MAX_I2S_PAGE];
char play_buf[I2S_PAGE_SIZE+4];

static int play_index = 0;
I2SST I2S;

//使能发送,先检查当前状态，再操作，防止内核态死锁
static void set_tx_state(int i2s_fd,int enable)
{
	if(I2S.tx_enable==0&&enable==1)
	{
		I2S.tx_enable=1;
		ioctl(i2s_fd, I2S_TX_ENABLE, 0);
	}
	else if(I2S.tx_enable==1&&enable==0)
	{
		I2S.tx_enable=0;
		ioctl(i2s_fd, I2S_TX_DISABLE, 0);
	}
}
//使能接收
static void set_rx_state(int i2s_fd,int enable)
{
	if(I2S.rx_enable==0&&enable==1){
		I2S.rx_enable=1;
		ioctl(i2s_fd, I2S_RX_ENABLE, 0);
	}
	else if(I2S.rx_enable==1&&enable==0){
		I2S.rx_enable=0;
		ioctl(i2s_fd, I2S_RX_DISABLE, 0);
	}
}
static int i2s_txbuffer_mmap(int i2s_fd)
{
	int i;
	for(i = 0; i < MAX_I2S_PAGE; i++)
	{
		shtxbuf[i] = mmap(0, I2S_PAGE_SIZE, PROT_WRITE, MAP_SHARED, i2s_fd, i*I2S_PAGE_SIZE);
		if (shtxbuf[i] == MAP_FAILED)
		{
			printf("i2scmd:failed to mmap..\n");
			return-1;
		}
		memset(shtxbuf[i], 0, I2S_PAGE_SIZE);
	}
	return 0;
}
static int i2s_rxbuffer_mmap(int i2s_fd)
{
	int i;
	for(i = 0; i < MAX_I2S_PAGE; i++)
	{
		shrxbuf[i] = mmap(0, I2S_PAGE_SIZE, PROT_READ, MAP_SHARED, i2s_fd, i*I2S_PAGE_SIZE);
		if (shrxbuf[i] == MAP_FAILED)
		{
			printf("i2scmd:failed to mmap..\n");
			return-1;
		}
	}
	return 0;
}

static int i2s_txbuffer_munmap(void)
{
	int i;
	for(i = 0; i < MAX_I2S_PAGE; i++)
	{
		if(munmap(shtxbuf[i], I2S_PAGE_SIZE)!=0)
		{
			printf("i2scmd : munmap i2s mmap faild\n");
			return-1;
		}
	}
	return 0;
}	

static int i2s_rxbuffer_munmap(void)
{
	int i;
	for(i = 0; i < MAX_I2S_PAGE; i++)
	{
		if(munmap(shrxbuf[i], I2S_PAGE_SIZE)!=0)
		{
			printf("i2scmd : munmap i2s mmap faild\n");
			return-1;
		}
	}
	return 0;
}
/*******************************************************
清除播放的缓存数据
*******************************************************/
static void __clean_play_cache_data(void)
{
	char *pBuf=NULL;
	int i = 0;
	while(1)   //清除音频DMA数据
	{
		ioctl(I2S.i2s_fd, I2S_PUT_AUDIO, &i);
		pBuf = shtxbuf[i];
		memset(pBuf, 0, I2S_PAGE_SIZE); 
		if (i==play_index)
			break;
	}	
}

void SetVol(int dir,int vol)
{
	if(I2S.tx_vol<=77){
		I2S.tx_vol=77;
	}
	switch(dir)
	{
		case VOL_SUB:
			I2S.tx_vol-=5;
			break;
		case VOL_ADD:
			I2S.tx_vol+=5;
			break;
		case VOL_SET:
			if(vol==0)
				I2S.tx_vol=0;
			else
				I2S.tx_vol=vol/2+77;
			break;
		default:
			return;
	}
		
	if(I2S.tx_vol>=127){
		I2S.tx_vol=127;
	}
	else if(I2S.tx_vol<=77){
		I2S.tx_vol=0;
	}
	set_vol_size(I2S.tx_vol);
	SET_TX_VOL(I2S.i2s_fd,I2S.tx_vol);
	printf("SetVol :vol = %d\n",I2S.tx_vol);
}

int GetVol(void){
	return (int)I2S.tx_vol;
}

void mute_recorde_vol(int change)
{
	if(change==UNMUTE){
		SET_TX_VOL(I2S.i2s_fd,I2S.tx_vol);
	}else{
		SET_TX_VOL(I2S.i2s_fd,change);
	}
	usleep(1000);
}

/********************************************
写入pcm数据给音频接口
********************************************/
void write_pcm(char *buf)
{
	char *pBuf;
	int play_index = 0;
#if defined(CONFIG_I2S_MMAP)	
	ioctl(I2S.i2s_fd, I2S_PUT_AUDIO, &play_index);
	pBuf = shtxbuf[play_index];  //指向DMA发送区
	memcpy(pBuf, buf, I2S_PAGE_SIZE); 
#else
	pBuf = txbuffer;
	memcpy(pBuf, buf, I2S_PAGE_SIZE);    
	ioctl(I2S.i2s_fd, I2S_PUT_AUDIO, pBuf);	
#endif	//end defined(CONFIG_I2S_MMAP)
}

void WritePcmData(char *data,int size)
{
	if(I2S.play_size==I2S_PAGE_SIZE)//fix me end is < do?
	{
		I2S.play_size=0;
		write_pcm(play_buf);
	}
	memcpy(play_buf+I2S.play_size,data,size);
	I2S.play_size +=size;
}
void WriteqttsPcmData(char *data,int len)
{
	int i=0;
	for(i=0;i<len;i+=2){
		memcpy(play_buf+I2S.qttspos,data+i,2);
		I2S.qttspos += 2;
		memcpy(play_buf+I2S.qttspos,data+i,2);
		I2S.qttspos += 2;
		if(I2S.qttspos==I2S_PAGE_SIZE){
			write_pcm(play_buf);
			I2S.qttspos=0;
		}
	}
}

/**********************************************
获取音频数据
**********************************************/
char *i2s_get_data(void)
{
	static int index_1 = 0;
	ioctl(I2S.i2s_fd, I2S_GET_AUDIO, &index_1);
	return shrxbuf[index_1];
}
#ifdef VOICS_CH
int get_volch(void){
	return I2S.vol_ch; 
}
void set_volch(unsigned char ch){
	I2S.vol_ch=ch;
}
#endif //end VOICS_CH
/********************************************
初始化wm8960音频接口
********************************************/
void __init_wm8960_voices(void)
{
	memset(&I2S,0,sizeof(I2SST));
	get_vol_size(&(I2S.tx_vol));//获取路由音量和播音人
#ifdef VOICS_CH
	get_vol_ch(&(I2S.vol_ch));
#endif //end VOICS_CH
	I2S.tx_rate =RECODE_RATE;
	I2S.i2s_fd = open(WM8960_NODE_PATH, O_RDWR|O_SYNC); 
	if(I2S.i2s_fd<0)
	{
		perror("i2scmd init");
		return -1;
	}
#ifdef MUTE_TX
	mute_recorde_vol(MUTE);
#endif	
	close_wm8960_voices();//----------------------
	SET_RATE(I2S.i2s_fd, I2S.tx_rate);
	SET_RX_VOL(I2S.i2s_fd, AUDIO_RX_VOICE);
	
#if defined(CONFIG_RALINK_MT7628) /*(CONFIG_I2S_WORD_LEN)*/
	ioctl(I2S.i2s_fd, I2S_WORD_LEN, 16);
	ioctl(I2S.i2s_fd, I2S_ENDIAN_FMT, 1);
#endif	//end defined(CONFIG_RALINK_MT7628)

#if defined(CONFIG_I2S_MMAP)
	i2s_txbuffer_mmap(I2S.i2s_fd);
#endif

#if defined(CONFIG_I2S_MMAP)
	i2s_rxbuffer_mmap(I2S.i2s_fd);
#endif	//end  defined(CONFIG_I2S_MMAP)

	set_rx_state(I2S.i2s_fd,  1);
	set_tx_state(I2S.i2s_fd, 1);
	I2S.execute_mode = EXTERNAL_LBK2;
	SET_TX_VOL(I2S.i2s_fd, I2S.tx_vol);
	open_wm8960_voices();//----------------------
#ifdef MUTE_TX
	mute_recorde_vol(UNMUTE);
#endif
}
#if 0
void SET_MUTE_DISABLE(void){
	ioctl(I2S.i2s_fd, I2S_MUTE_DISABLE, 0);
}
void SET_MUTE_ENABLE(void){
	ioctl(I2S.i2s_fd, I2S_MUTE_ENABLE, 0);
}
#endif
static void clean_i2s_play(void){
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
	usleep(100*1000);
	
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
	usleep(100*1000);
	
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
	usleep(100*1000);
	
	memset(play_buf,0,I2S_PAGE_SIZE);
	write_pcm(play_buf);
	usleep(100*1000);
}
int i2s_start_play(unsigned short rate)
{
#ifdef MUTE_TX
	mute_recorde_vol(MUTE);
#endif
	close_wm8960_voices();
	clean_i2s_play();
	I2S.play_size=0;
	if(rate==I2S.tx_rate)  //播放的采样率等于录音采样率，不需要切换
	{
		printf("start play rate = %d\n",rate);
		//SET_MUTE_ENABLE();
		open_wm8960_voices();
#ifdef MUTE_TX
		mute_recorde_vol(UNMUTE);
#endif
		return -1;
	}
	set_rx_state(I2S.i2s_fd,0);		//先关闭发送和接收，切换采样率
	set_tx_state(I2S.i2s_fd,0);
	SET_RATE(I2S.i2s_fd,rate);	//设置采样率
	I2S.tx_rate=rate;	//生效采样率
	I2S.tx_enable=0;
	set_rx_state(I2S.i2s_fd,1);
	set_tx_state(I2S.i2s_fd,1);
	I2S.execute_mode = PLAY_MODE;
	
	//SET_MUTE_ENABLE();
	open_wm8960_voices();
#ifdef MUTE_TX
	mute_recorde_vol(UNMUTE);
#endif
	return 0;
}
void i2s_destory_voices(void)
{
	char* pBuf;
	if(I2S.execute_mode == EXTERNAL_LBK2 || I2S.execute_mode == PLAY_MODE)
	{
		int i = 0;
		while(1)   //清除音频DMA数据
		{
			ioctl(I2S.i2s_fd, I2S_PUT_AUDIO, &i);
			pBuf = shtxbuf[i];
			if(*pBuf==0&&*(pBuf+1)==0&&*(pBuf+2)==0)
			{
				printf("__clean_voices : shtxbuf is empty\n");
				break;
			}
			memset(pBuf, 0, I2S_PAGE_SIZE); 
			if (i==play_index)
				break;
		}	
	}
	set_tx_state(I2S.i2s_fd,0);
	set_rx_state(I2S.i2s_fd,0);
	i2s_txbuffer_munmap();
	i2s_rxbuffer_munmap();
	I2S.execute_mode = NONE_MODE;
	close(I2S.i2s_fd);
}
