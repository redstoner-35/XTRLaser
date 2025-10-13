/****************************************************************************/
/** \file Sleep.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现系统在长时间不操作时自动关闭
CPU和数字外设时钟进入STOP mode极大节约电池消耗的深度睡眠功能。同时该文件实现了长
时间不用时自动锁定激光手电本体的安全机制。避免长期静置并忘记锁定时被小孩拿起直接
使用造成严重事故。

**	History: 
/** \Date 2025/10/16 16:00
/** \Desc 新增长时间放置后执行自动锁定功能的相关代码实现。

**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "SysConfig.h"
#include "LEDMgmt.h"
#include "VersionCheck.h"
#include "ActiveBeacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

#define SleepTimeOut 5 //系统在关机且无操作状态下进入休眠的超时时间(秒)
#define LVKillSampleInterval 12 //系统在有源夜光开启时的欠压自杀采样周期(1单位=8秒)

/************** 自动锁定功能的检测define **************/
#ifndef AutoLockTimeOut
  //如果系统没有定义自动锁定结束时间，则定义一个0值避免编译过不去。
  #define AutoLockTimeOut 0
#endif

#define AutoLockCNTValue (AutoLockTimeOut/8)          //自动锁定计数器的数值
#if (AutoLockCNTValue > 0xFF)
  //自动锁定时间过长，超过了允许值
	#error "Auto Lock TimeOut is way too long and causing timer overflow."
	
#elif (AutoLockTimeOut < 0)
  //自动锁定时间不允许是负数
	#error "Auto Lock TimeOut should be any number larger than zero!"

#elif (AutoLockTimeOut > 0)
  //输入合法的自动锁定延时，功能开启
	#message "Auto Lock Function of this firmware is enabled."
	
#else
  //自动锁定计时器关闭
	#message "Auto Lock Function of this firmware is disabled.To enable this function"
	#message "you need to define the time of auto lock by add a <AutoLockTimeOut>=[time(Sec)]"
	#message "define in project global definition screen on Complier preference menu."
	#warning "Auto lock function set to disabled state will causing major security issues,USE AS YOUR OWN RISK!!!"
#endif

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

volatile unsigned int SleepTimer;	//睡眠定时器

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
static xdata unsigned char AutoLockTimer; //自动锁定计时器

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

//禁止所有系统外设
static void DisableSysPeripheral(void)
	{
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //关闭PWM和ADC
	LED_DeInit(); //复位LED管理器
	ActiveBeacon_Start(); //启动有源夜光模块
	
	//如果自动锁定开启，则强制打开自动唤醒计时器
	#if (AutoLockTimeOut > 0)
	LVD_Start();
	#endif
	}

//启动所有系统外设
static void EnableSysPeripheral(void)
	{
	LVD_Disable(); //启动系统到正常工作阶段之前关闭WUT
	ADC_Init(); //初始化ADC
	PWM_Init(); //初始化PWM发生器
	LED_Init(); //初始化侧按LED
	OutputChannel_Init(); //初始化输出通道
	SystemTelemHandler(); //启动一次ADC，进行初始测量
	DisplayVBattAtStart(0); //执行一遍电池初始化函数	
	EnableADCAsync(); 			//所有外设初始化完毕，启动ADC异步处理模式
	}

//检测系统是否允许进入睡眠的条件
static char QueryIsSystemNotAllowToSleep(void)
	{
	//系统在显示电池电压和版本号，不允许睡眠
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//系统开机了
	if(Current>0||IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//允许睡眠
	return 0;
	}	

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//加载定时器时间
void LoadSleepTimer(void)	
	{
	//加载自动锁定时间
  #if (AutoLockTimeOut > 0)
	AutoLockTimer=(unsigned char)AutoLockCNTValue;
	#endif
	//加载睡眠时间
	if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=240; //故障报错模式，系统睡眠时间变为240S
	else SleepTimer=8*SleepTimeOut; 		
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
	ADCSampleCounter=LVKillSampleInterval; //装载欠压自杀计时模块的时间值
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
		else //执行自动锁定和欠压自杀计时器判断处理	
			{
			#if (AutoLockTimeOut > 0)
			//判断系统是否已经自动锁定
			sleepsel=0;													//初始化标志位用于判断是否需要关闭WUT计时器
			if(AutoLockTimer)AutoLockTimer--;		//时间还没到继续往下减
			else if(!IsSystemLocked)
				{
				//系统未上锁，执行上锁处理
				IsSystemLocked=1;
				LastMode=Mode_Low;								//重置系统的挡位模式记忆至最低
				//制造三次快闪指示进入锁定模式
				StartSystemTimeBase(); 						//启动系统定时器提供系统定时和延时函数
				LED_Init(); 						          //初始化侧按LED
				LEDMode=LED_RedBlinkThird;
				while(LEDMode==LED_RedBlinkThird)if(SysHFBitFlag)
					{
					//制造三次红色快闪
					sleepsel=~sleepsel;
					if(sleepsel)LEDControlHandler();	
					SysHFBitFlag=0;
					}	
				//闪烁提示完毕，保存配置并关闭LED
				sleepsel=0;
				SaveSysConfig(0);
				DisableSysHBTIM();    
				LED_DeInit(); 				//复位LED管理器并关闭系统定时器
				ActiveBeacon_Start(); //启动有源夜光模块
				}
			else sleepsel=1; //系统已锁定，此时可以判断LVD是否需要关闭来决定是否关闭WUT
			#else 
			//自动锁定计时器关闭，使得系统仅需判断欠压自杀是否结束
			sleepsel=1;		
			#endif
			//判断系统的是否执行欠压自杀
			if(ADCSampleCounter)ADCSampleCounter--;
			else if(!ActiveBeacon_LVKill())ADCSampleCounter=LVKillSampleInterval;    //欠压自杀处理失败电池电压仍然足够,复位采样计数器
      else if(sleepsel)LVD_Disable();   //欠压自杀已经完成且自动锁定已经计时结束，关闭WUT进入彻底深睡
			}
		}
	while(!SleepTimer);
	}
