/****************************************************************************/
/** \file Beacon.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统三击特殊功能组合里面的脉冲
信标闪功能（系统首先以低亮点亮几秒钟后熄灭，然后以恒定间隔令LD脉冲工作产生脉冲闪）

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "Beacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//脉冲信标闪参数
#define BeaconOnTime 75 //信标闪烁时间
#define BeaconOFFTime 3 //信标关闭时间(秒)
#define BeaconInfoTime 3 //信标在开始之前低亮提示用户的时间(秒)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	BeaconState_Init,
	BeaconState_InfoUser,
	BeaconState_ONStrobe,
	BeaconState_OFFWait,
	}BeaconStateDef;

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata BeaconStateDef State;
static xdata unsigned char BeaconOnTIM;
static xdata unsigned char BeaconOffTIM;

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

//复位状态机
void BeaconFSM_Reset(void)
	{
	State=BeaconState_InfoUser;
	BeaconOnTIM=0;
	BeaconOffTIM=8*BeaconInfoTime;
	}

//关闭时间计时
void BeaconFSM_TIMHandler(void)
	{
	if(BeaconOffTIM)BeaconOffTIM--;
	}

//信标模式状态机
char BeaconFSM(void)
	{
	switch(State)
		{
		case BeaconState_InfoUser:	
			if(BeaconOffTIM>0)return 2; //当前处于提示状态，提示用户
		  //提示时间到，跳转到OFF阶段准备开始显示
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
			break;
		//初始化
		case BeaconState_Init:
			BeaconOnTIM=BeaconOnTime;
		  State=BeaconState_ONStrobe;
      return 1;
		//等待阶段
		case BeaconState_ONStrobe:
			BeaconOnTIM--;
			if(BeaconOnTIM>0)return 1;
	    //点亮时间到，熄灭
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
		  break;
		//等待熄灭阶段结束
		case BeaconState_OFFWait:
		  if(!BeaconOffTIM)State=BeaconState_Init;
		  break;
		}
	//其余状态返回0使LED熄灭
	return 0;
	}
