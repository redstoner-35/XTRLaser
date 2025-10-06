#ifndef _BCON_
#define _BCON_
	
//函数
void BeaconFSM_Reset(void); //复位状态机
void BeaconFSM_TIMHandler(void); //关闭时间计时
char BeaconFSM(void); //信标模式状态机

#endif
