/****************************************************************************/
/** \file ActiveBeacon.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统在进入休眠模式后令侧部按键
发出微光指示按键位置和系统状态，方便用户操作的功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "LEDMgmt.h"
#include "ModeControl.h"
#include "BattDisplay.h"
#include "ActiveBeacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

#define ActiveBeaconOFFVolt 2900 //设置当等效单节电池电压欠压后关闭有源夜光避免电池饿死的阈值(mV)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

extern volatile unsigned int SleepTimer; //系统睡眠计时器
extern xdata unsigned char AutoLockTimer; //自动锁定计时器

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
static bit IsEnableActiveBeacon=0; //内部标志位，有源信标是否开启

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

//系统进入休眠之前，启动有源夜光模块
void ActiveBeacon_Start(void)
	{
	GPIOCfgDef LEDInitCfg;
	//如果电池电压低于2.9V为了避免导致电池彻底饿死，禁止打开定位LED（当然的话还有就是用户主动关闭）
	if(CellVoltage<ActiveBeaconOFFVolt||!IsEnableIdleLED)return;
	//设置结构体
	LEDInitCfg.Mode=GPIO_IPU;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //配置为弱上拉令红色灯珠发出微光
	//配置GPIO并启动LVD
	LVD_Start();
	IsEnableActiveBeacon=1; //标记已经使能有源夜光功能
	if(CurrentMode->ModeIdx==Mode_Fault)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);
	else GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);  //如果系统是故障状态，则点亮红灯提示用户当前系统无法运行
	}
	
//有源夜光模块开启之后，进行电池欠压自杀避免电池饿死的模块
bit ActiveBeacon_LVKill(void)
	{	
	//有源夜光已被关闭，不执行欠压自杀处理
	if(!IsEnableActiveBeacon)return 1;
	//启动ADC并进行采样处理
	ADC_Init(); 																 //初始化ADC
	SystemTelemHandler(); 
	CellVoltage=(int)(Data.BatteryVoltage*1000); //启动一次ADC，获取并更新电池电压
	//电池欠压，关闭有源夜光避免电池饿死
	if(CellVoltage<ActiveBeaconOFFVolt)
		{
		IsEnableActiveBeacon=0;
		LED_Init(); 
		}
	//检测完毕，关闭ADC并令睡眠定时器=0，系统立即进入睡眠
	ADC_DeInit(); 
	SleepTimer=0;
	return 0;
	}
