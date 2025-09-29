#include "cms8s6990.h"
#include "GPIO.h"
#include "delay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "PWMCfg.h"
#include "SysReset.h"
#include "LowVoltProt.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SelfTest.h"
#include "SOS.h"
#include "BreathMode.h"
#include "Beacon.h"

//睡眠处理
void SleepMgmt(void);
void MaskUnusedIO(void);

//主函数
void main()
	{
	bit TaskSel=0;
	//时钟和RSTCU初始化	
	ClearSoftwareResetFlag();
  StartSystemTimeBase(); //启动系统定时器提供系统定时和延时函数
	//初始化外设
	ADC_Init(); //初始化ADC
	PWM_Init(); //启动PWM定时器
	LED_Init(); //初始化侧按LED
  SideKeyInit(); //侧按初始化	
	OutputChannel_Init(); //输出初始化	
	ModeFSMInit(); //初始化模式状态机
  DisplayVBattAtStart(1); //显示电池状况
	MaskUnusedIO();   //屏蔽掉所有单片机未使用的IO降低待机功耗
	EnableADCAsync(); //启动ADC的异步模式提高处理速度
	//主循环	
  while(1)
		{
	  //实时处理
		SystemTelemHandler();
		BatteryTelemHandler(); //获取ADC和电池信息	
		SideKey_LogicHandler(); //处理侧按事务
		ThermalMgmtProcess(); //温度管理
		ModeSwitchFSM(); //挡位状态机
		OutputChannel_Calc();  //输出通道处理
		PWM_OutputCtrlHandler(); //处理PWM输出事务	
		//8Hz交错定时处理
		if(!SysHFBitFlag)continue; //时间没到，跳过处理
			
		//Task0，处理计算量比较大的任务
    if(!TaskSel)
			{
			LEDControlHandler();//侧按指示LED控制函数	
			BattAlertTIMHandler(); //电池警报定时处理
			OutputFaultDetect(); //输出故障检测
			ThermalPILoopCalc(); 				//积分器计算	
			ShowEntered2SModeProc(); //处理进入2S模式的显示
			SleepMgmt(); //睡眠处理
			BreathFSM_TIMHandler(); //呼吸模式状态机计时处理
			
			//处理结束，对任务选择进行翻转处理下一组
			TaskSel=1;
		  }			
		//Task1，处理计算量比较小的计时任务
		else
			{	
			SideKey_TIM_Callback();//侧按按键的监测定时器处理		
			BattDisplayTIM(); //电池电量显示TIM
			DisplayErrorTIMHandler(); //故障代码显示
			ModeFSMTIMHandler(); //模式状态机处理
			HoldSwitchGearCmdHandler(); //长按换挡处理
			SOSTIMHandler(); //SOS定时器
			RuntimeUpdateTo2S(); //动态检测电池电压并更新到2S
			BeaconFSM_TIMHandler(); //信标计时器
				
			//处理结束，对任务选择进行翻转处理下一组
			TaskSel=0;
			}
		//处理完毕，令flag清零
		SysHFBitFlag=0;	
		}
	}