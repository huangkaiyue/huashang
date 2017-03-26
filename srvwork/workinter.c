#include "config.h"

#ifdef WORK_INTER
#include "comshead.h"
#include "base/pool.h"
#include "base/tools.h"
#include "workinter.h"
#include "host/voices/callvoices.h"
#include "../host/voices/gpio_7620.h"
#include "StreamFile.h"

#define IMHELP    \
  "Commands: poolnum(pn)\n"\
  "Commands: lsmem(mem) devlist(show devices list )\n"\
  "Commands: susr(look usrdb) sdev(look devlist ) sroot(look camlist and passwd)\n"\
  "Commands: del_usrtable (delete usr table)\n"\
  "Commands: map(show usr map address)\n"\
  "Commands: del_usr(deltele usr  )\n"\
  "Commands: uplist  initlist  \n"\
  "Commands: help(h)  quit(q)\n"  
void test_ConnetEvent(int event){
	printf("event =%d\n",event);
}
/*
@ 调试命令，编译固件需要去掉
@
*/
void pasreInputCmd(const char *com){
	if (!strcmp(com, "1")){
		test_start_playurl();
	}else if(!strcmp(com, "2")){
		test_stop_playurl();
	}else if(!strcmp(com, "3")){
		test_single_playurl();
	}
}

#if 0
void pasreInputCmd(const char *com){
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
			Setwm8960Vol(1,0);
		}else if(!strcmp(com,"10"))
		{
			int i=0;
			for(i;i<10;i++){
				disable_gpio();
				Create_PlaySystemEventVoices(10);
				sleep(15);
			}
		}
		else if(!strcmp(com,"subvol"))
		{
			Setwm8960Vol(0,0);
		}
		else if(!strcmp(com,"gwifi"))
		{
			startSmartConfig(Create_PlaySystemEventVoices);
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
		else if(!strcmp(com,"qtts"))
		{
			static char buf[566];
			memset(buf,0,566);
			start_event_play_wav(8);
			memcpy(buf,"互联网+”代表一种新的经济形态，即充分发挥互联网在生产要素配置中的优化和集成作用，将互联网的创新成果深度融合于经济社会各领域之中，提升实体经济的创新力和生产力，形成更广泛的以互联网为基础设施和实现工具的经济发展新形态。“ 互联网+”行动计划将重点促进以云计算、物联网、大数据为代表的新一代信息技术与现代制造业、生产性服务业等的融合创新，发展壮大新兴业态，打造新的产业增长点，为大众创业、万众创新提供环境，为产业智能化提供支撑，增强新的经济发展动力，促进国民经济提质增效升级。 2015年3月5日十二届全国人大三次会议上，李克强总理在政府工作报告中首次提出“互联网+”行动计划。",566);
			PlayQttsText(buf,0);
		}else if(!strcmp(com,"testmp3"))
		{
			createPlayEvent((const void *)"testmp3",1);
		}

		else if(!strcmp(com,"url")){
			//createPlayEvent((const void *)"http://fdfs.xmcdn.com/group7/M01/A3/8D/wKgDX1d2Rr6w3CegABHDHZzUiUs448.mp3",1);
			Player_t *App=(Player_t *)calloc(1,sizeof(Player_t));
			if(App==NULL){
				perror("calloc error !!!");
			}
			snprintf(App->playfilename,128,"%s","http://beta.app.tuling123.com/file/soundset/music/bazhixiaoe.mp3");	
			snprintf(App->musicname,64,"%s","speek");
			App->musicTime = 0;
			createPlayEvent(App,1);
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
			Create_PlayQttsEvent("正在下载固件",0);
		}else if(!strcmp(com,"seek")){
			test_quikSeekTo();
		}else if(!strcmp(com,"prev")){
			test_backSeekTo();
		}
		else if (!strcmp(com, "quit") ||
             		!strcmp(com, "q"))
    	{
    		CleanSystemResources();
			exit(0);
			return ;
    	}else if (!strcmp(com, "1"))
    	{
    		if(sysMes.localplayname==english){
				keyStreamPlay();
			}else{
    		createPlayEvent((const void *)"guoxue",1);
			}
    	}else if (!strcmp(com, "xiai"))
    	{
    		createPlayEvent((const void *)"xiai",1);
    	}else if (!strcmp(com, "2"))
    	{
    		if(sysMes.localplayname==english){
				keyStreamPlay();
			}else{
    			createPlayEvent((const void *)"english",1);
			}
    	}else if (!strcmp(com, "3"))
    	{
    		if(sysMes.localplayname==story){
				keyStreamPlay();
			}else{
    			createPlayEvent((const void *)"story",1);
			}
    	}else if (!strcmp(com, "4"))
    	{
    		if(sysMes.localplayname==mp3){
				keyStreamPlay();
			}else{
    			createPlayEvent((const void *)"mp3",1);
			}
    	}else if (!strcmp(com, "mp3"))
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
#endif

#endif	//end WORK_INTER
