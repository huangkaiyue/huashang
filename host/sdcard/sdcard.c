#include <stdio.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bits/types.h>
#include <dirent.h>

#include "config.h"
#include "systools.h"

#define SEC			1
#define MIN			60*SEC
#define HOUR		60*MIN
#define DAY			24*HOUR
#define MONTH		30*DAY
#define REMOVE_MP3_NUM 10

#define SD_MIN		50

/****************************************************************************************
*��������:	��ȡĳ��Ŀ¼�µ��ļ�
*����:		cache_path Ŀ¼·��  filename �����ȡ�����ļ���  playMp3Num�ڼ����ļ���
*����ֵ:	-2 �򿪲��ɹ� -1 ��ȡ�굱ǰĿ¼�����ļ� 0�ɹ���ȡ��
****************************************************************************************/
int get_mp3filenmae(char *cache_path,char *filename,int playMp3Num){
	DIR *dirptr = NULL;  
	struct dirent *entry;  
	unsigned char mount=0;
	int ret = 0;
	char tmp[64]={0};
	if((dirptr = opendir(cache_path)) == NULL)  
	{  
		printf("open dir !\n"); 
		return -2;  
	}
	while (entry = readdir(dirptr))  
	{  
		if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,"..")){
			usleep(1000);
			continue;
		}
		if(mount==0){
			strcpy(tmp,entry->d_name);
		}
		mount++;
		if(mount==playMp3Num)
		{
			strcpy(filename,entry->d_name);
			break;
		}
		usleep(100);
	}
	printf("mount = %d playMp3Num=%d\n",mount,playMp3Num);
	if(mount<playMp3Num)
	{
		ret = -1;
		strcpy(filename,tmp);
	}
	closedir(dirptr);   
	return ret;
}

//�Ƿ�С��SDʣ��ռ����޴�С
void CheckSdcardInfo(char * sdpath)
{
	int Capacity =0;
	GetStorageInfo(sdpath,&Capacity,FREE);
	if(Capacity<SD_MIN)
		return -1;
	else
		return 0;
}
//ɾ����ʱ�䲻�õ��ļ�
void DelSdcardMp3file(char * sdpath)
{
	char filepath[128]={0};
	int delmp3Num=0;
	int ret;
	struct stat Mp3info;
	DIR *dirptr = NULL;  
	struct dirent *entry;  
	unsigned char mount=0;
	time_t timep;
	
	time(&timep);
#ifdef LOG_DELMP3
	writeLog("/home/mp3filename.txt",sdpath);
#endif
	if((dirptr = opendir(sdpath)) == NULL)	
	{  
		printf("open dir !\n"); 
		return ;	
	}
	while (entry = readdir(dirptr))  
	{  
		//ȥ����ǰĿ¼����һ��Ŀ¼
		if( !strcmp(entry->d_name,".")||!strcmp(entry->d_name,"..") ){
			usleep(1000);
			continue;
		}
#ifdef LOG_DELMP3
		writeLog("/home/mp3filename.txt",entry->d_name);
#endif
		printf("filename: %s\n",entry->d_name);
		snprintf(filepath,128,"%s%s",sdpath,entry->d_name);
		if( stat(filepath, &Mp3info) != 0 ){
			break;
		}
		if((timep-Mp3info.st_ctime)<7*MIN){
			usleep(1000);
			continue;
		}else{			//ɾ����ʱ�䲻�õ��ļ�
			if( remove(filepath) == 0){
				printf("Removed %s.\n", filepath);
				delmp3Num++;
				if(delmp3Num>REMOVE_MP3_NUM){
					break;
				}
			}
			else
				perror("remove");
			usleep(1000);
		}
		usleep(1000);
	}
	printf("del file end ... \n");
}

#if 0
int main(int argc,char **argv){
	int Capacity =0;
	GetStorageInfo(argv[1],&Capacity,ALL);
	printf("%s  size %d Mb\n",argv[1],Capacity);
	return 0;
}
#endif

