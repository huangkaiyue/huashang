#include "comshead.h"

#include "uart/uart.h"
#include "config.h"

//显示智能会话表情
void Show_musicPicture(void){
#if defined(HUASHANG_JIAOYU)
	int faceNumS=(1+(int) (2.0*rand()/(RAND_MAX+1.0)));	
	switch(faceNumS){
		case 1:
			showFacePicture(PLAY_MUSIC_NUM1);
			break;
		case 2:
			showFacePicture(PLAY_MUSIC_NUM2);
			break;
	}
#endif	
}
void Show_KeyDownPicture(void){
#if defined(HUASHANG_JIAOYU)	
	showFacePicture(KEY_CTRL_PICTURE);
#endif
}
void Show_waitCtrlPicture(void){
#if defined(HUASHANG_JIAOYU)		
	int faceNumS=(1+(int) (4.0*rand()/(RAND_MAX+1.0)));	
	switch(faceNumS){
		case 1:
			showFacePicture(WAIT_CTRL_NUM1);
			break;
		case 2:
			showFacePicture(WAIT_CTRL_NUM2);
			break;
		case 3:
			showFacePicture(WAIT_CTRL_NUM3);
			break;
		case 4:
			showFacePicture(WAIT_CTRL_NUM4);
			break;	
		default:
			showFacePicture(WAIT_CTRL_NUM4);
			break;
	}
#endif
}
void Show_tlak_Light(void){
#if defined(HUASHANG_JIAOYU)
	showFacePicture(SHOW_TLAK_LIGHT);
#endif
}
void Close_tlak_Light(void){
#if defined(HUASHANG_JIAOYU)	
	showFacePicture(CLOSE_TLAK_LIGHT);
#endif
}
