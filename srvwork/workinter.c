#include "config.h"

#ifdef WORK_INTER
#include "comshead.h"
#include "base/pool.h"
#include "base/tools.h"
#include "workinter.h"
#include "host/voices/callvoices.h"
#include "../udpsrv/broadcast.h"
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
#ifdef TEXT_UP
static char amr_data[5000];
#endif
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
			memcpy(buf,"互联网+”代表一种新的经济形态，即充分发挥互联网在生产要素配置中的优化和集成作用，将互联网的创新成果深度融合于经济社会各领域之中，提升实体经济的创新力和生产力，形成更广泛的以互联网为基础设施和实现工具的经济发展新形态。“ 互联网+”行动计划将重点促进以云计算、物联网、大数据为代表的新一代信息技术与现代制造业、生产性服务业等的融合创新，发展壮大新兴业态，打造新的产业增长点，为大众创业、万众创新提供环境，为产业智能化提供支撑，增强新的经济发展动力，促进国民经济提质增效升级。 2015年3月5日十二届全国人大三次会议上，李克强总理在政府工作报告中首次提出“互联网+”行动计划。",566);
			PlayQttsText(buf,0);
		}else if(!strcmp(com,"testmp3"))
		{
			createPlayEvent((const void *)"testmp3",PLAY_NEXT);
		}
#ifdef TEXT_UP
		else if(!strcmp(com,"up"))
		{
			char buf[40];
			FILE *fp;
			int i=0,k=0;
			while(1){
				if(sysMes.recorde_live==3||i==0){
					if(i==3)
						i=0;
					i++;
					sysMes.recorde_live=6;
					k++;
					if(k>5)
						sysMes.recorde_live==3;
						break;
				}else{
					printf("sleep ... (k=%d)\n",k);
					sleep(2);
					continue;
				}
				memset(buf,0,40);
				switch(i){
					case 1:
						sprintf(buf,"%s%d%s","/home/",946685274,".amr");//讲个笑话
						break;
					case 2:
						sprintf(buf,"%s%d%s","/home/",946685979,".amr");
						break;
					case 3:
						sprintf(buf,"%s%d%s","/home/",946685295,".amr");
						break;
				}
				printf("buf =%s\n",buf);
				fp =fopen(buf,"r");
				if(fp==NULL)
				{
					perror("open file error ...\n");
					break;
				}
				int fsta,fend,cur_pos;
				cur_pos = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				fsta = ftell(fp);
				fseek(fp, 0, SEEK_END);
				fend = ftell(fp);
				fseek(fp, cur_pos, SEEK_SET);
				int len = fend-fsta;
				printf("len =%d\n",len);
				fread(amr_data,len,1,fp);
				send_voices_server((const char *)amr_data,len,"amr");
				fclose(fp);
			}
		}
#endif	//end TEXT_UP
		else if(!strcmp(com,"initlist"))
		{
			updateSysList(FRIST_SMART_LIST, FRIST_PASSWD);
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
			createPlayEvent((const void *)"http://fdfs.xmcdn.com/group7/M01/A3/8D/wKgDX1d2Rr6w3CegABHDHZzUiUs448.mp3",PLAY_NEXT);
		}
		else if(!strcmp(com,"qurl")){
			CleanUrlEvent();
		}
		else if(!strcmp(com,"uart")){
			char bufo[12]="00:20";
			char bufc[12]="00:17";
			SocSendMenu(3,0);
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
			QttsPlayEvent("正在下载固件",0);
		}else if(!strcmp(com,"seek")){
			test_quikSeekTo();
		}else if(!strcmp(com,"prev")){
			test_backSeekTo();
		}else if(!strcmp(com,"amr")){
			WavtoAmrfile("/mnt/tang/");
			//createPlayEvent((const void *)"story",PLAY_NEXT);
		}
		else if (!strcmp(com, "quit") ||
             		!strcmp(com, "q"))
    	{
    		clean_resources();
			exit(0);
			return ;
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
