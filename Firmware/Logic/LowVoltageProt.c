/****************************************************************************/
/** \file LowVoltageProt.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统常规挡位以及各个特殊功能
和无极调光挡位下的电池低电量保护降档和关机的功能。同时实现系统运行中动态更新电池
节数的功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "TempControl.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "ADCCfg.h"
#include "SelfTest.h"
#include "SysConfig.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//低电压警报和保护参数配置
#define BatteryAlertDelay 10 //电池警报延迟	
#define BatteryFaultDelay 2  //电池故障强制跳档/关机的延迟

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata unsigned char BattAlertTimer; //电池低电压告警处理
static xdata unsigned char RampCurrentRiseAttmTIM; //无极调光恢复电流的计时器	

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

static void StartBattAlertTimer(void)	//低电量保护启动定时器开始进行计时的函数
	{
	//启动定时器
	if(!BattAlertTimer)BattAlertTimer=1;
	}	

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

void RuntimeUpdateTo2S(void)	//运行过程中检测电池电压并更新到2S模式
	{
	#ifndef USING_LD_NURM11T
	//电池电压非2S模式或者2S模式已开启，退出
	if(IsEnable2SMode||Data.RawBattVolt<4.35)return;
	//当前系统为单锂，检测到高电压自动开启2S模式
	IsEnable2SMode=1;
	Trigger2SModeEnterInfo();
	SaveSysConfig(0);
	#else
	//红光LD只能单锂，如果系统是双锂模式，则强制回到单锂
	if(IsEnable2SMode)
		{
		IsEnable2SMode=0;
		SaveSysConfig(0);
		}
	//电池电压高于单锂允许值，报错
	if(Data.RawBattVolt>4.35)ReportError(Fault_InputOVP);	
	#endif
	}
	
//电池低电量报警处理函数
void BattAlertTIMHandler(void)
	{
	//无极调光警报定时
	if(RampCurrentRiseAttmTIM&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//电量警报
	if(BattAlertTimer&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	

//电池低电量保护函数
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	unsigned char Thr=BatteryFaultDelay;
	bit IsChangingGear;
	//获取手电按键的状态
	if(getSideKey1HEvent())IsChangingGear=1;
	else IsChangingGear=getSideKeyHoldEvent();
	//控制计时器启停
	if(!IsBatteryFault) //电池没有发生低压故障
		{
		Thr=BatteryAlertDelay; //没有故障可以慢一点降档
		//当前在换挡阶段或者没有告警，停止计时器,否则启动
		if(!IsBatteryAlert||IsChangingGear)BattAlertTimer=0;
		else StartBattAlertTimer();
		}
  else StartBattAlertTimer();//发生低压告警立即启动定时器
	//定时器计时已满，执行对应的动作
	if(BattAlertTimer>Thr)
		{
		//当前挡位处于需要在触发低电量保护时主动关机的状态	
		if(IsNeedToShutOff)ReturnToOFFState();
		//当前处于换挡模式不允许执行降档但是需要判断电池是否过低然后强制关闭
		else if(IsChangingGear&&IsBatteryFault)ReturnToOFFState();
		//不需要关机，触发换挡动作
		else
			{
			BattAlertTimer=0;//重置定时器至初始值
			SwitchToGear(ModeJump); //复位到指定挡位
			}
		}
	}		

//无极调光开机时恢复低压保护限流的处理	
void RampRestoreLVProtToMax(void)
	{
	if(IsBatteryAlert||IsBatteryFault)return;
	if(BattState==Battery_Plenty)SysCfg.RampCurrentLimit=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX; //电池电量回升到充足状态，复位电流限制
	
	}
	
//无极调光的低电压保护
void RampLowVoltHandler(void)
	{
	if(!IsBatteryAlert&&!IsBatteryFault)//没有告警
		{
		BattAlertTimer=0;
		if(BattState==Battery_Plenty) //电池电量回升到充足状态，缓慢增加电流限制
			{
	    if(SysCfg.RampCurrentLimit<QueryCurrentGearILED())
				 {
			   if(!RampCurrentRiseAttmTIM)RampCurrentRiseAttmTIM=1; //启动定时器开始计时
				 else if(RampCurrentRiseAttmTIM<9)return; //时间未到
         RampCurrentRiseAttmTIM=1;
				 if(SysCfg.RampBattThres>CurrentMode->LowVoltThres)SysCfg.RampBattThres=CurrentMode->LowVoltThres; //电压检测达到上限，禁止继续增加
				 else SysCfg.RampBattThres+=50; //电压检测上调50mV
         if(SysCfg.RampCurrentLimit>QueryCurrentGearILED())SysCfg.RampCurrentLimit=QueryCurrentGearILED();//增加电流之后检测电流值是否超出允许值
				 else SysCfg.RampCurrentLimit+=250;	//电流上调250mA		 
				 }
			else RampCurrentRiseAttmTIM=0; //已达到电流上限禁止继续增加
			}
		return;
		}
	else RampCurrentRiseAttmTIM=0; //触发警报，复位尝试增加电流的定时器
	//低压告警发生，启动定时器
	StartBattAlertTimer(); //发生命令启动定时器
	if(IsBatteryFault&&BattAlertTimer>4)ReturnToOFFState(); //电池电压低于关机阈值大于0.5秒，立即关闭
	else if(BattAlertTimer>BatteryAlertDelay) //电池挡位触发
		{
		if(SysCfg.RampCurrentLimit>500)SysCfg.RampCurrentLimit-=250; //电流下调250mA
		if(SysCfg.RampBattThres>2850)SysCfg.RampBattThres-=25; //减少25mV
    BattAlertTimer=1;//重置定时器
		}
	}
