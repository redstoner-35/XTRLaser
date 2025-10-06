/****************************************************************************/
/** \file BreathMode.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统三击特殊功能组合里面的巡航
呼吸闪功能（灯珠从最暗缓缓升到最亮然后缓降到最暗后熄灭，等一会后再循环反复）

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "BreathMode.h"
#include "ModeControl.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//呼吸模式的参数配置
#define CurrentRampUpInc 1 //呼吸模式下电流上升的速度
#define CurrentRampDownDec 1 //呼吸模式下电流下降的速度
#define CurrentHighSustainTime 4 //呼吸模式下电流在最高点的保持时间（0.125S per LSB）
#define CurrentLowSustainTime 5  //呼吸模式下电流在关闭点的保持时间（单位 秒）

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	//内部状态
	BreathMode_RampUp,
	BreathMode_MaintainHigh,
	BreathMode_RampDown,
	BreathMode_MaintainLow 
	}BreathModeFSMDef;
	
/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata int BreathCurrentBuf;
static xdata unsigned char BreathFSMTIM;
static xdata BreathModeFSMDef BreathFSM;
static xdata char BreathModeDivCNT;

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/
	
//复位呼吸模式状态机
void BreathFSM_Reset(void)
	{
	BreathFSM=BreathMode_RampUp;
	BreathCurrentBuf=CurrentMode->MinCurrent;
	BreathFSMTIM=0;
	BreathModeDivCNT=0;
	}

//呼吸模式状态机定时处理
void BreathFSM_TIMHandler(void)
	{
	if(BreathFSMTIM)BreathFSMTIM--;
	}
	
//呼吸模式状态机运算
int BreathFSM_Calc(void)
	{
	int Imax=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX;
	//实际的状态机流程
	switch(BreathFSM)
		{
		case BreathMode_RampUp:
			//当前分频计数器增在计数
		  if(BreathModeDivCNT>0)BreathModeDivCNT--;
			//电流从最低点线性上升到最高
			else if(BreathCurrentBuf<Imax)
				{
				//动态加载分频计数器实现电流变化率一致
				BreathModeDivCNT=(char)(QueryCurrentGearILED()/Imax)-1;
				if(BreathModeDivCNT<0)BreathModeDivCNT=0;
				//电流还没到最高，线性上升
				BreathCurrentBuf+=CurrentRampUpInc;
				if(BreathCurrentBuf>Imax)BreathCurrentBuf=Imax; //限制电流结果不能超过挡位额定最高
				}
			else
				{
				//电流已经到最高了，进入保持状态
				BreathFSM=BreathMode_MaintainHigh;
				BreathFSMTIM=CurrentHighSustainTime;
				}
			break;
		case BreathMode_MaintainHigh:
			//保持计时器仍在计时，保持在最高电流
			if(BreathFSMTIM)return Imax;
		  //保持计时器结束，开始进入电流递减阶段
		  BreathFSM=BreathMode_RampDown;
			break;
		case BreathMode_RampDown:
			//当前分频计数器增在计数
		  if(BreathModeDivCNT>0)BreathModeDivCNT--;
			//电流线性递减
			else if(BreathCurrentBuf>CurrentMode->MinCurrent)
				{
				//动态加载分频计数器实现电流变化率一致
				BreathModeDivCNT=(char)(QueryCurrentGearILED()/Imax)-1;
				if(BreathModeDivCNT<0)BreathModeDivCNT=0;
				//电流还没到最低，线性递减
				BreathCurrentBuf-=CurrentRampDownDec;
				if(BreathCurrentBuf<CurrentMode->MinCurrent)BreathCurrentBuf=CurrentMode->MinCurrent; //限制电流不允许小于最小值
				}
			else
				{
				//电流到最低了，进入关闭状态
				BreathFSM=BreathMode_MaintainLow;
				BreathFSMTIM=CurrentLowSustainTime*8;
				}
			break;
		case BreathMode_MaintainLow:
			//保持计时器仍在计时，返回0结果令LED熄灭
			if(BreathFSMTIM)return 0;
		  //保持计时器结束，重新进入电流爬升阶段开始新一轮循环	
      BreathFSM=BreathMode_RampUp;		
		  break;
		}
	//其余默认情况返回缓存结果
	return BreathCurrentBuf;
	}
