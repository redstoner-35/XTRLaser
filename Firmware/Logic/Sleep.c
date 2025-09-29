#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "VersionCheck.h"
#include "ActiveBeacon.h"

//睡眠定时器
volatile unsigned int SleepTimer;

//禁止所有系统外设
static void DisableSysPeripheral(void)
	{
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //关闭PWM和ADC
	LED_DeInit(); //复位LED管理器
	ActiveBeacon_Start(); //启动有源夜光模块
	}

//启动所有系统外设
static void EnableSysPeripheral(void)
	{
	ADC_Init(); //初始化ADC
	PWM_Init(); //初始化PWM发生器
	LED_Init(); //初始化侧按LED
	OutputChannel_Init(); //初始化输出通道
	SystemTelemHandler(); //启动一次ADC，进行初始测量
	DisplayVBattAtStart(0); //执行一遍电池初始化函数	
	EnableADCAsync(); 			//所有外设初始化完毕，启动ADC异步处理模式
	}

//加载定时器时间
void LoadSleepTimer(void)	
	{
	//加载睡眠时间
	if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=240; //故障报错模式，系统睡眠时间变为240S
	else SleepTimer=8*SleepTimeOut; 		
	}

//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统在显示电池电压和版本号，不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//系统开机了
	if(IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//允许睡眠
	return 0;
	}	
	
//睡眠管理函数
void SleepMgmt(void)
	{
	bit sleepsel;
	unsigned char ADCSampleCounter;
	//非关机且仍然在显示电池电压的时候定时器复位禁止睡眠
	if(QueryIsSystemNotAllowToSleep())LoadSleepTimer();
	//允许睡眠开始倒计时
	if(SleepTimer>0)
		{
		SleepTimer--;
		return;
		}
	//时间到，立即进入睡眠阶段
	DisableSysPeripheral();
	do
		{		
		//令STOP=1，使单片机进入睡眠
		STOP();  
		//唤醒之后需要跟6条NOP
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		//系统已唤醒，立即开始检测
		if(GetIfSideKeyTriggerInt()) 
			{
			//检测到系统并非由LVD唤醒，立即完成初始化判断按键状态
			StartSystemTimeBase(); //启动系统定时器提供系统定时和延时函数
			MarkAsKeyPressed(); //立即标记按键按下
			SideKey_SetIntOFF(); //关闭侧按中断
			do	
				{
				delay_ms(1);
				SideKey_LogicHandler(); //处理侧按事务
				//侧按按键的监测定时器处理(使用62.5mS心跳时钟,通过2分频)
				if(!SysHFBitFlag)continue; 
				SysHFBitFlag=0;
				sleepsel=~sleepsel;
				if(sleepsel)SideKey_TIM_Callback();
				}
			while(!IsKeyEventOccurred()); //等待按键唤醒		
			//系统已完成按键事件检测，初始化其余外设		
			EnableSysPeripheral();	
			return;
			}
		//欠压自杀计时器计时中
		else if(ADCSampleCounter)ADCSampleCounter--;
		//系统由WUT唤醒，启动ADC，检测电池电压后立即睡眠
		else
			{
			ActiveBeacon_LVKill();  //进行电池电压检测
			ADCSampleCounter=11;    //欠压自杀不需要特别频繁的采样,复位采样计数器
			}
		}
	while(!SleepTimer);
	}
