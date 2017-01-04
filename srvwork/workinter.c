#include "config.h"

#ifdef WORK_INTER
#include "comshead.h"
#include "base/pool.h"
#include "base/tools.h"
#include "workinter.h"
#include "host/voices/callvoices.h"
#include "../host/voices/gpio_7620.h"

#define IMHELP    \
  "Commands: poolnum(pn)\n"\
  "Commands: lsmem(mem) devlist(show devices list )\n"\
  "Commands: susr(look usrdb) sdev(look devlist ) sroot(look camlist and passwd)\n"\
  "Commands: del_usrtable (delete usr table)\n"\
  "Commands: map(show usr map address)\n"\
  "Commands: del_usr(deltele usr  )\n"\
  "Commands: uplist  initlist  \n"\
  "Commands: help(h)  quit(q)\n"  
void test_ConnetEvent(int event)
{
	printf("event =%d\n",event);
}
void pasreInputCmd(const char *com)
{
 		if (!strcmp(com, "lsmen") ||
        	!strcmp(com, "mem"))
    	{
    	}	
		else if(!strcmp(com,"playmp3"))
		{
			//play_voices("/mnt/encoder/mp3/nannianjing.mp3",MP3_FILE,4577071,44100);
			//play_voices("/mnt/encoder/mp3/tianlong.mp3",MP3_FILE,335296,32000);
		}else if(!strcmp(com,"playmp4"))
		{
			//decode_local_music("/mnt/encoder/mp3/tianlong.mp3");
			//play_voices("/mnt/encoder/mp3/tianlong.mp3",MP3_FILE,335296,32000);
			//play_voices("/mnt/encoder/mp3/test_play1.mp3",MP3_FILE,1107804,22050);
		}
		else if(!strcmp(com,"addvol"))
		{	
			SetVol(1,0);
		}else if(!strcmp(com,"10"))
		{
			int i=0;
			for(i;i<10;i++){
				disable_gpio();
				create_event_system_voices(10);
				sleep(15);
			}
		}
		else if(!strcmp(com,"subvol"))
		{
			SetVol(0,0);
		}
		else if(!strcmp(com,"version"))
		{
			printf("==============>%s<=============\n",VERSION);
		}
		else if(!strcmp(com,"gwifi"))
		{
			startSmartConfig(create_event_system_voices);
		}
		else if(!strcmp(com,"c"))
		{
			//disable_gpio();
			//close_sys_led();
			Led_vigue_close();
			usleep(100);
			close_wm8960_voices();
		}
		else if(!strcmp(com,"o"))
		{
			//enable_gpio();
			pool_add_task(Led_vigue_open,NULL);
			//open_sys_led();
			open_wm8960_voices();
			usleep(100);
		}
#ifdef VOICS_CH
		else if(!strcmp(com,"volch0"))
		{
			set_vol_ch(0);
		}
		else if(!strcmp(com,"volch1"))
		{
			set_vol_ch(1);
		}
#endif //VOICS_CH
		else if(!strcmp(com,"qtts"))
		{
			static char buf[566];
			memset(buf,0,566);
			start_event_play_wav();
			memcpy(buf,"������+������һ���µľ�����̬������ַ��ӻ�����������Ҫ�������е��Ż��ͼ������ã����������Ĵ��³ɹ�����ں��ھ�����������֮�У�����ʵ�徭�õĴ����������������γɸ��㷺���Ի�����Ϊ������ʩ��ʵ�ֹ��ߵľ��÷�չ����̬���� ������+���ж��ƻ����ص�ٽ����Ƽ��㡢��������������Ϊ�������һ����Ϣ�������ִ�����ҵ�������Է���ҵ�ȵ��ںϴ��£���չ׳������ҵ̬�������µĲ�ҵ�����㣬Ϊ���ڴ�ҵ�����ڴ����ṩ������Ϊ��ҵ���ܻ��ṩ֧�ţ���ǿ�µľ��÷�չ�������ٽ����񾭼�������Ч������ 2015��3��5��ʮ����ȫ���˴����λ����ϣ����ǿ���������������������״������������+���ж��ƻ���",566);
			PlayQttsText(buf,0);
		}else if(!strcmp(com,"testmp3"))
		{
			createPlayEvent((const void *)"testmp3",1);
		}
		else if(!strcmp(com,"scanwifi"))
		{
			char msg[1500]={0};
			int len;
			mtk76xxScanWifi(msg,&len);
			printf("msg = %s len=%d \n",msg,len);
		}
		else if(!strcmp(com,"readwifi"))
		{
			ReadWifi();
		}
		else if(!strcmp(com,"url")){
			createPlayEvent((const void *)"http://fdfs.xmcdn.com/group7/M01/A3/8D/wKgDX1d2Rr6w3CegABHDHZzUiUs448.mp3",1);
		}
		else if(!strcmp(com,"qurl")){
			CleanUrlEvent();
		}
		else if(!strcmp(com,"uart")){
			char bufo[12]="10:30";
			char bufc[12]="00:17";
			SocSendMenu(3,0);
			sleep(2);
			printf("--------------------------\n");
			SocSendMenu(7,bufo);
			sleep(2);
			printf("--------------------------\n");
			SocSendMenu(1,bufo);
			sleep(2);
			printf("--------------------------\n");
			SocSendMenu(2,bufc);
			sleep(2);
			printf("--------------------------\n");
			SocSendMenu(4,0);
			sleep(1);
			printf("--------------------------\n");
		}else if(!strcmp(com,"qtts1")){
			QttsPlayEvent("�������ع̼�",0);
		}else if(!strcmp(com,"seek")){
			test_quikSeekTo();
		}else if(!strcmp(com,"prev")){
			test_backSeekTo();
		}else if(!strcmp(com,"amr")){
			WavtoAmrfile("/mnt/qtts/","/mnt/amr/",0);
			//createPlayEvent((const void *)"story",PLAY_NEXT);
			sleep(5);
			WavtoAmrfile("/mnt/qtts/","/mnt/amr/",1);
		}
		else if (!strcmp(com, "quit") ||
             		!strcmp(com, "q"))
    	{
    		clean_resources();
			exit(0);
			return ;
    	}else if (!strcmp(com, "guoxue"))
    	{
    		createPlayEvent((const void *)"guoxue",1);
    	}else if (!strcmp(com, "xiai"))
    	{
    		createPlayEvent((const void *)"xiai",1);
    	}else if (!strcmp(com, "english"))
    	{
    		createPlayEvent((const void *)"english",1);
    	}else if (!strcmp(com, "story"))
    	{
    		createPlayEvent((const void *)"story",1);
    	}else if (!strcmp(com, "mp3"))
    	{
    		createPlayEvent((const void *)"mp3",1);
    	}else if (!strcmp(com, "next"))
    	{
    		switch(sysMes.localplayname){
					case mp3:
						createPlayEvent((const void *)"mp3",1);
						break;
					case story:
						createPlayEvent((const void *)"story",1);
						break;
					case english:
						createPlayEvent((const void *)"english",1);
						break;
					case guoxue:
						createPlayEvent((const void *)"guoxue",1);
						break;
					default:
						break;
				}
    	}else if (!strcmp(com, "showdb"))
    	{
  
    	}
    	else if (!strcmp(com, "help") ||
           	!strcmp(com, "h"))
		{
			printf(IMHELP);
  		}
		else if(strlen(com)>=1)
  		{
			printf(IMHELP);
		}
}

#endif	//end WORK_INTER
