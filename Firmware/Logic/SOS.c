/****************************************************************************/
/** \file SOS.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统三击特殊功能组合里面的SOS
求救挡位以及五击+长按进入的SOS紧急求救模式（没有低电保护）的SOS摩尔斯电码播报。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "SOS.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//SOS时序配置（每单位0.125S）
#define SOSDotTime 2 //SOS信号(.)的时间	
#define SOSDashTime 6 //SOS信号(-)的时间	
#define SOSGapTime 7 //SOS信号在每次显示途中等待的时间
#define SOSFinishGapTime 35 //每轮SOS发出结束后的等待时间

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	//SOS状态枚举
	SOSState_Prepare,
	SOSState_3Dot,
	SOSState_3DotWait,
	SOSState_3Dash,
	SOSState_3DashWait,
	SOSState_3DotAgain,
	SOSState_Wait,
	}SOSStateDef;	

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata SOSStateDef SOSState; //全局变量状态位
static xdata char SOSTIM;  //SOS计时

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

static void SOSFSM_Jump(SOSStateDef State,char Time)	//SOS状态机的跳转处理
	{
	if(SOSTIM)return; //显示未结束
	SOSTIM=Time; 
	SOSState=State;  //进入延时等待阶段
	}
//SOS定时器状态监测
static bit SOSTIMDetect(char Time)
	{
	//触发定时判断
	if((SOSTIM%(Time*2))>(Time-1))return 1;
	//关闭状态返回0
	return 0;
	}
/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//SOS状态机的时序处理
void SOSTIMHandler(void)
	{
	//对计时器数值进行递减
	if(SOSTIM)SOSTIM--;
	}

//复位整个SOS模块
void ResetSOSModule(void)
	{
	SOSState=SOSState_Prepare;
	SOSTIM=0;
	}

//SOS状态机处理模块
char SOSFSM(void)
	{
	switch(SOSState)
		{
		//准备阶段
		case SOSState_Prepare:
			 SOSTIM=0;
			 SOSFSM_Jump(SOSState_3Dot,(3*SOSDotTime*2)-1);
		   break;
		//第一和第二次三点
		case SOSState_3DotAgain:
		case SOSState_3Dot:
       if(SOSTIMDetect(SOSDotTime))return 1; //当前状态需要LED电流，返回1
			 if(SOSState==SOSState_3Dot)SOSFSM_Jump(SOSState_3DotWait,SOSGapTime);  //进入延时等待阶段
		   else SOSFSM_Jump(SOSState_Wait,SOSFinishGapTime);//进入延时等待阶段
		   break;
		//三点结束后的等待延时阶段
	  case SOSState_3DotWait:
			 SOSFSM_Jump(SOSState_3Dash,(3*SOSDashTime*2)-1);
		   break;
		//三划
		case SOSState_3Dash:
			 if(SOSTIMDetect(SOSDashTime))return 1; //当前状态需要LED电流，返回1	
		   SOSFSM_Jump(SOSState_3DashWait,SOSGapTime);
		   break;			
		//三划结束后的等待延时阶段
	  case SOSState_3DashWait:
			 SOSFSM_Jump(SOSState_3DotAgain,(3*SOSDotTime*2)-1);
		   break;		
	  //本轮信号发出完毕，等待
	  case SOSState_Wait:	
			 SOSFSM_Jump(SOSState_Prepare,0);//回到准备状态
		   break;
		}
	//其余情况返回0关闭防反接，确保尾按可以正确响应
	return 0;
	}
