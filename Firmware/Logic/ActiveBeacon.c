#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "LEDMgmt.h"
#include "ModeControl.h"
#include "BattDisplay.h"
#include "ActiveBeacon.h"

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
	if(CurrentMode->ModeIdx==Mode_Fault)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);
	else GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);  //如果系统是故障状态，则点亮红灯提示用户当前系统无法运行
	}
	
//有源夜光模块开启之后，进行电池欠压自杀避免电池饿死的模块
void ActiveBeacon_LVKill(void)
	{	
	extern volatile unsigned int SleepTimer;
	//启动ADC并进行采样处理
	ADC_Init(); 																 //初始化ADC
	SystemTelemHandler(); 
	CellVoltage=(int)(Data.BatteryVoltage*1000); //启动一次ADC，获取并更新电池电压
	//电池欠压，关闭有源夜光避免电池饿死
	if(CellVoltage<ActiveBeaconOFFVolt)
		{
		LVD_Disable(); 
		LED_Init(); 
		}
	//检测完毕，关闭ADC并令睡眠定时器=0，系统立即进入睡眠
	ADC_DeInit(); 
	SleepTimer=0;
	}