#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "musicList.h"

#define DATABASE_NAME	"music.db"  	//数据库名
#define MESSAGE_TABLE	"message"	//记录歌曲信息表名	

typedef struct{
	List_t list[MUSIC_LIST];
}MusicList_t;

MusicList_t *Mlist=NULL;


//数据库表:保存歌曲ID号和名字 (用来播放列表用)
static void CreateMusicTable(const char *tableMusic){
	char sql[128]={0};
	snprintf(sql,128,"create table if not exists %s (id INTEGER PRIMARY KEY,name VARCHAR(64) not null unique);",tableMusic);            
	CreateTable(sql);
}
//保存歌曲列表信息,缓存播放历史记录值
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
//根据目录，获取表名，去掉后面的 / 符号
static void GetTableName(const char *path,char *destPath){
	int len = strlen(path);
	if(*(path+len-1)=='/'){
		strncat(destPath,path,len-1);
	}else{
		strncat(destPath,path,len);
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
	list->DirTime = mtime;//更新数据库当中目录最新时间
	struct dirent *entry;  
	if((dirptr = opendir(getPath)) == NULL)  {  
		perror("open dir failed!\n"); 
		return -1;  
	}

	while (entry = readdir(dirptr)){  
		if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,"..")){
			continue;
		}
		printf("list->listname = %s\n",list->listname);
		InsertSql(list->listname,entry->d_name);
		fileNum++;			
	}
	list->Nums =fileNum;
	closedir(dirptr);   
	return 0;
}
static enum{
	mp3_N=1,
	story_N,
	english_N,
	guoxue_N,
};

void SaveSystemPlayNum(void){
	List_t cachelist;
	int i=0;
	for(i=0;i<MUSIC_LIST;i++){
		snprintf(cachelist.listname,24,"%s%s","cache",Mlist->list[i].listname);
		if(InsertMusicMessageSQL(MESSAGE_TABLE,cachelist.listname,Mlist->list[i].playindex,0)){
			UpdateSqlByMessage(MESSAGE_TABLE,cachelist.listname,Mlist->list[i].playindex,0);
		}
		printf("Mlist->list[%d].playindex =%d\n",i,Mlist->list[i].playindex);
	}
#ifdef DOWN_URL_MUSIC
	SaveXiaiMusicPlayIndex();
#endif
}
/*
开机读取目录文件，加载到数据库当中
*/
int SysOnloadMusicList(const char *sdcard,const char *mp3Music,const char *story,const char *english,const char *guoxue){
	char sqlPath[128]={0};
	
	checkpath(sdcard,sqlPath);
	strcat(sqlPath,DATABASE_NAME);
	if(OpenSql(sqlPath) != 0)
		return -1;

	int i=0,listNum=MUSIC_LIST;
	GetTableName(mp3Music,Mlist->list[0].listname);
	GetTableName(story,Mlist->list[1].listname);
	GetTableName(english,Mlist->list[2].listname);
	GetTableName(guoxue,Mlist->list[3].listname);

	List_t cachelist;
	for(i=0;i<MUSIC_LIST;i++){
		// 缓存目录当中的 cachemp3 字段里面，Nums记录的是关机之后，播放的num标号
		memset(&cachelist,0,sizeof(List_t));
		snprintf(cachelist.listname,24,"%s%s","cache",Mlist->list[i].listname);
		GetMusicMessageSQL(MESSAGE_TABLE,&cachelist);
		Mlist->list[i].playindex=cachelist.Nums;
		printf("Mlist->list[%d].playindex =%d\n",i,Mlist->list[i].playindex);
	}


#ifdef DOWN_URL_MUSIC	
	CreateMusicTable(XIMALA_MUSIC);
	GetXiaiMusicPlayIndex();
#endif	
	CreateMusicListMesage(MESSAGE_TABLE);
	for(i=0;i<listNum;i++){
		GetMusicMessageSQL(MESSAGE_TABLE,&(Mlist->list[i]));
		CreateMusicTable(Mlist->list[i].listname);
		//printf("Mlist->list[0].listname =%s\n",Mlist->list[i].listname);
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
		if(!strncmp(Mlist->list[i].listname,musicDir,strlen(Mlist->list[i].listname))){
			break;
		}
		//printf("musicDir = %s Mlist->list[i].listname=%s\n",musicDir,Mlist->list[i].listname);
	}
	if(i>=MUSIC_LIST){
		printf("warning is GetSdcardMusic \n");
		return -1;
	}
	switch(Mode){
		case PLAY_NEXT:
			++Mlist->list[i].playindex;
			if(Mlist->list[i].playindex>Mlist->list[i].Nums){
				Mlist->list[i].playindex=1;//数据库当中的ID 是从1开始
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
	printf("GetSdcardMusic : playindex=%d (%d)\n",Mlist->list[i].playindex,Mlist->list[i].Nums);
	return GetTableSqlById(Mlist->list[i].listname,Mlist->list[i].playindex,getMusicname);
}
#ifdef DOWN_URL_MUSIC
typedef struct{
	unsigned char playMode;
	char listname[24];
	int Nums;
	int playindex;
}NetMusic_t;
static NetMusic_t *netmusic=NULL;
int InsertXimalayaMusic(const char *musicDir,const char *musicName){
	return InsertSql(musicDir,musicName);
}
int DelXimalayaMusic(const char *musicDir,const char *musicName){
	return del_DBdata(musicDir,musicName);
}
static int table_result(void *args,int nRow,int nColumn,char **dbResult){
	char *getMusicname = (char *)args;
	int result, i, j, index;
	// dbResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据  
	if(nRow==0){
		return -1;
	}
	index = nColumn; 
	if(netmusic->playMode==PLAY_NEXT){
		if(++netmusic->playindex>=nRow){
			netmusic->playindex=1;
		}
	}else if(netmusic->playMode==PLAY_PREV){
		if(--netmusic->playindex<=0){
			netmusic->playindex=nRow;
		}
	}
	printf( "all cloumn: %d records  netmusic->playindex = %d\n", nRow ,netmusic->playindex);	
	int res = (netmusic->playindex-1)*nColumn+nColumn-1;
	snprintf(getMusicname,128,dbResult [res]);	
#if 0	
	for( i = 0; i < nRow ; i++ )  { 		
			for( j = 0 ; j < nColumn; j++ )  {  
				if(netmusic->playindex==i){
				printf( "char: %s\t value: %s\n", dbResult[j], dbResult [index] );	
				//dbResult 的字段值是连续的，从第0索引到第 nColumn - 1索引都是字段名称，从第 nColumn 索引开始，后面都是字段值，
				//它把一个二维的表（传统的行列表示法）用一个扁平的形式来表示  
			}  
			++index;	
		} 
	}
#endif	
	return 0;
}
int PlayxiaiMusic(const char *sdcard,const char *musicDir,char *getMusicname,unsigned char Mode){
	netmusic->playMode = Mode;
	return GetTableSql(musicDir,getMusicname,table_result);	
}
void SaveXiaiMusicPlayIndex(void){
	char cacheName[24]={0};
	snprintf(cacheName,24,"%s%s","cache",XIMALA_MUSIC);
	if(InsertMusicMessageSQL(MESSAGE_TABLE,cacheName,netmusic->playindex,0)){
		UpdateSqlByMessage(MESSAGE_TABLE,cacheName,netmusic->playindex,0);
	}
}
void GetXiaiMusicPlayIndex(void){
	List_t cachelist;
	memset(&cachelist,0,sizeof(List_t));
	// 缓存目录当中的 cachemp3 字段里面，Nums记录的是关机之后，播放的num标号
	snprintf(cachelist.listname,24,"%s%s","cache",XIMALA_MUSIC);
	GetMusicMessageSQL(MESSAGE_TABLE,&cachelist);
	netmusic->playindex=cachelist.Nums;
	printf("netmusic->playindex=%d\n",netmusic->playindex);
}
#endif
int InitMusicList(void){
	Mlist = (MusicList_t *)calloc(1,sizeof(MusicList_t));
	if(Mlist==NULL){
		return -1;
	}
#ifdef DOWN_URL_MUSIC	
	netmusic= (NetMusic_t *)calloc(1,sizeof(NetMusic_t));
	if(netmusic==NULL){
		return -1;
	}
#endif
	return 0;
}
void CleanMusicList(void){
	if(Mlist){
		free(Mlist);
		Mlist=NULL;
	}
	if(netmusic){
		free(netmusic);
		netmusic=NULL;
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
	for(i=0;i<7;i++)	{
		//GetSdcardMusic((const char *)"sdcard",(const char *)"keji",musicname,PLAY_NEXT);
		GetSdcardMusic((const char *)"sdcard/",(const char *)"why/",musicname,PLAY_PREV);
		printf("musicname[%d] = %s\n",i,musicname);
		memset(musicname,0,128);
	}
}
void test_NetMusic(void){
	char getMusicname[128]={0};
	int i=0;
	int ret=-1;
	for(i=0;i<3;i++)	{
		ret = PlayxiaiMusic((const char *)"sdcard/",(const char *)XIMALA_MUSIC,getMusicname,PLAY_NEXT);
		if(ret){
			printf("get XIMALA_MUSIC failed \n");
			continue;
		}
		printf("XIMALA_MUSIC[%d] = %s\n",i,getMusicname);
		memset(getMusicname,0,128);
	}
}
int main(void){
	InitMusicList();
	SysOnloadMusicList((const char *)"sdcard/",(const char *)"keji/",(const char *)"why/",(const char *)"english/",(const char *)"guoxue/");
	testPlayList();
#if 1
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic11");
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic12");
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic13");
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic14");
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic15");
	InsertXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic16");
#else
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic11");
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic12");
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic13");
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic14");
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic15");
	DelXimalayaMusic((const char *)XIMALA_MUSIC,(const char *)"testmusic16");
#endif
	test_NetMusic();
	SaveSystemPlayNum();
	CleanMusicList();
	return 0;
}
#endif
