#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "musicList.h"

#define DATABASE_NAME	"music.db"  	//���ݿ���
#define MESSAGE_TABLE	"message"	//��¼������Ϣ����	

#define MUSIC_LIST	3

typedef struct{
	List_t list[MUSIC_LIST];
}MusicList_t;

MusicList_t *Mlist=NULL;

static void CreateMusicTable(const char *tableMusic){
	char sql[128]={0};
	snprintf(sql,128,"create table if not exists %s (id INTEGER PRIMARY KEY,name VARCHAR(64) not null unique);",tableMusic);            
	CreateTable(sql);
}

static void CreateMusicListMesage(const char *tableMusic){
	char sql[128]={0};
	snprintf(sql,128,"create table if not exists %s (name VARCHAR(64) not null unique,number INTEGER,DirTime INTEGER);",tableMusic);            
	CreateTable(sql);
}

static void checkpath(const char *path,char *getPath){
	int len = strlen(path);
	if(*(path+len-1)=='/'){
		strncat(getPath,path,len);
	}else{
		strncat(getPath,path,len);
		strncat(getPath,"/",1);
	}
}
static int onloadSdcardFile(const char *sdcard,List_t *list){
	DIR *dirptr = NULL;  
	char getPath[128]={0};
	int fileNum=0;
	checkpath(sdcard,getPath);
	strcat(getPath,list->listname);
	
	struct stat buf;
	if(stat(getPath, &buf)){
		return -1;
	}
	
	int mtime = (int)buf.st_mtime;
	if(mtime==list->DirTime){
		printf("%s dir  data is new \n",list->listname);
		return -1;
	}
	list->DirTime = mtime;//�������ݿ⵱��Ŀ¼����ʱ��
	struct dirent *entry;  
	if((dirptr = opendir(getPath)) == NULL)  {  
		perror("open dir failed!\n"); 
		return -1;  
	}

	while (entry = readdir(dirptr)){  
		if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,"..")){
			continue;
		}
		InsertSql(list->listname,entry->d_name);
		fileNum++;			
	}
	list->Nums =fileNum;
	closedir(dirptr);   
	return 0;
}

int SysOnloadMusicList(const char *sdcard,const char *mp3Music,const char *story,const char *english){
	char sqlPath[128]={0};
	checkpath(sdcard,sqlPath);
	strcat(sqlPath,DATABASE_NAME);
	if(OpenSql(sqlPath) != 0)
		return -1;
	snprintf(Mlist->list[0].listname,24,"%s",mp3Music);
	snprintf(Mlist->list[1].listname,24,"%s",story);
	snprintf(Mlist->list[2].listname,24,"%s",english);
	int i=0;
	CreateMusicListMesage(MESSAGE_TABLE);
	for(i=0;i<MUSIC_LIST;i++){
		GetMusicMessageSQL(MESSAGE_TABLE,&(Mlist->list[i]));
		CreateMusicTable(Mlist->list[i].listname);
		if(!onloadSdcardFile(sdcard,&(Mlist->list[i]))){
			if(InsertMusicMessageSQL(MESSAGE_TABLE,Mlist->list[i].listname,Mlist->list[i].Nums,Mlist->list[i].DirTime)){
				UpdateSqlByMessage(MESSAGE_TABLE,Mlist->list[i].listname,Mlist->list[i].Nums,Mlist->list[i].DirTime);
			}
		}
	}
	return 0;
}

/*

*/
int GetSdcardMusic(const char *sdcard,const char *musicDir,char *getMusicname,unsigned char Mode){
	int i=0;
	for(i=0;i<MUSIC_LIST;i++){
		if(!strcmp(musicDir,Mlist->list[i].listname)){
			break;
		}
		//printf("musicDir = %s Mlist->list[i].listname=%s\n",musicDir,Mlist->list[i].listname);
	}
	if(i>=MUSIC_LIST){
		return -1;
	}
	switch(Mode){
		case PLAY_NEXT:
			++Mlist->list[i].playindex;
			if(Mlist->list[i].playindex>Mlist->list[i].Nums){
				Mlist->list[i].playindex=1;//���ݿ⵱�е�ID �Ǵ�1��ʼ
			}
			break;
		case PLAY_PREV:
			--Mlist->list[i].playindex;
			if(Mlist->list[i].playindex<=0){
				Mlist->list[i].playindex=Mlist->list[i].Nums;
			}
			break;
		default:
			return -1;
	}
	GetTableSqlById(musicDir,Mlist->list[i].playindex,getMusicname);
	return 0;
}

int InitMusicList(void){
	Mlist = (MusicList_t *)calloc(1,sizeof(MusicList_t));
	if(Mlist==NULL){
		return -1;
	}
	return 0;
}
void CleanMusicList(void){
	if(Mlist){
		free(Mlist);
		Mlist=NULL;
	}
	CloseSql();
}

int checkMusicDb(const char *sdcard,const char *musicDir){

}

//#define MAIN_TEST
#ifdef MAIN_TEST
void testPlayList(void){
	char musicname[128]={0};
	int i=0;
	for(i=0;i<12;i++)	{
		//GetSdcardMusic((const char *)"sdcard",(const char *)"keji",musicname,PLAY_NEXT);
		GetSdcardMusic((const char *)"sdcard",(const char *)"keji",musicname,PLAY_PREV);
		printf("musicname[%d] = %s\n",i,musicname);
		memset(musicname,0,128);
	}
}
int main(void){
	InitMusicList();
	SysOnloadMusicList((const char *)"sdcard",(const char *)"keji",(const char *)"why",(const char *)"english");
	testPlayList();
	CleanMusicList();
	return 0;
}
#endif
