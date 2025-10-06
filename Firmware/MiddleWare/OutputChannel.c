/****************************************************************************/
/** \file OutputChannel.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件为中层设备驱动文件，负责根据上层逻辑层反馈的目标输出电流
值计算并操控PWMDAC输出指定的LD电流并完成LD的软起动保护功能。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"
#include "delay.h"
#include "OutputChannel.h"
#include "ModeControl.h"
#include "ADCCfg.h"
#include "TempControl.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//PWMDAC参数配置
#define VdivUpResK 220 //运放分压部分的上端电阻(KΩ)
#define PWMDACResK 10 //PWMDAC的电阻阻值(KΩ)
#define VdivDownResK 5.1 //运放分压部分的下端电阻(KΩ)
#define CurrentOffset 99.2 //高电流通道下的电流偏差值(单位%)

//输出主通道参数设置
#define MainChannelShuntmOhm 10 //主通道的检流电阻阻值(mR)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata int Current; //目标电流(mA)
xdata int CurrentBuf; //存储当前已经上传的电流值 

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable and SFR definitions('static and sfr')
****************************************************************************/
static bit IsDCDCEnabled; //DCDC是否使能
static bit IsSlowRamp; //开启慢速Ramp

sbit DCDCEN=DCDCENIOP^DCDCENIOx; //DCDC使能Pin
/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/
static float Duty_Calc(int CurrentInput)		//内部用于计算PWMDAC占空比的函数	
	{
	float buf;
	//计算实际占空比
	buf=(float)CurrentInput*(float)MainChannelShuntmOhm; //输入传进来的电流(mA)并乘以检流电阻阻值(mR)得到运放端整定电压(uV)
	buf/=(float)1000; //uV转mV
	buf/=((float)VdivDownResK/(float)(VdivUpResK+VdivDownResK+PWMDACResK)); //将运放端整定电压除以电阻的分压比例得到DAC端的电压
	buf*=(float)CurrentOffset/(float)100; //乘以矫正系数修正电流
	buf/=Data.MCUVDD*(float)1000; //计算出目标DAC输出电压和PWMDAC缓冲器供电电压(MCUVDD)之间的比值
	buf*=100; //转换为百分比
	//结果输出	
	return buf>100?100:buf;
	}

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/
//获取输出是否开启
bit GetIfOutputEnabled(void)
	{
	return IsDCDCEnabled;
	}

//初始化函数
void OutputChannel_Init(void)
	{
	GPIOCfgDef OCInitCfg;
	//设置结构体
	OCInitCfg.Mode=GPIO_Out_PP;
  OCInitCfg.Slew=GPIO_Fast_Slew;		
	OCInitCfg.DRVCurrent=GPIO_High_Current; //推MOSFET,需要高上升斜率
	//初始化bit
	DCDCEN=0;
	//开始配置IO	
	GPIO_ConfigGPIOMode(DCDCENIOG,GPIOMask(DCDCENIOx),&OCInitCfg);				
	//系统上电时电流配置为0
	Current=0;
	CurrentBuf=0;
	IsDCDCEnabled=0;
	IsSlowRamp=0;
	}
	
//输出通道计算
void OutputChannel_Calc(void)
	{
	int TargetCurrent;
	extern bit IsBurnMode;
	//根据当前传入电流和其余状态得出实际要怼入温控计算函数的电流
	//读取目标电流并应用温控加权数据
	if(Current>0)
		{
		//取出温控限流数据
		TargetCurrent=ThermalILIMCalc();
		//如果目标电流小于当前挡位的温控限制值，则应用当前设置的电流值
		if(Current<TargetCurrent)TargetCurrent=Current;
		}
	//电流值为0或者-1，直接读取目标电流值
	else TargetCurrent=Current;
	//避免无效的重复计算
	if(CurrentBuf==TargetCurrent)return;	
	//电流大于0开启输出，选择对应通道
	if(TargetCurrent>0)
		{
		//保护LED的电流斜率限制器
		if((TargetCurrent-CurrentBuf)>600)IsSlowRamp=1; //监测到非常大的电流瞬态，避免冲爆灯珠采用软起
		if(IsSlowRamp)
			{
			//开始线性增加电流
			if(CurrentBuf==0)
				{
				//当前系统电流为0，判断传入的电流值并从低电流开始输出
				if(TargetCurrent<600)CurrentBuf=TargetCurrent;
				else CurrentBuf=600;
				}
			//系统电流不为0，立即开始增加电流
			else if(IsBurnMode)CurrentBuf+=50;    //烧灼模式开启，迅速增加电流提高烧灼效果
      else switch(CurrentMode->ModeIdx)
				{
				
				case Mode_Beacon:CurrentBuf+=1000;break; //信标模式令电流快速增加
				case Mode_SOS:
				case Mode_SOS_NoProt:CurrentBuf+=400;break;  //SOS模式下快速增加电流避免拖尾影响判断
				default:CurrentBuf+=2;											 //其余挡位电流默认缓慢增加
				}
			if(CurrentBuf>=TargetCurrent)
				{
				IsSlowRamp=0;
				CurrentBuf=TargetCurrent; //限幅，不允许目标电流大于允许值
				}
			}
		else CurrentBuf=TargetCurrent; //直接同步		
		//EN处于关闭状态，启用DCDC后令PWMDAC=0等待一段时间解决输出电流过冲导致闪烁的问题
		if(!IsDCDCEnabled)
				{
				DCDCEN=1;  
				delay_ms(5); //让PWMDAC输出=0保持一段时间对输出电容进行预充电，然后再送给定值避免闪烁
				IsDCDCEnabled=1; //标记DCDC已经开始运行
				}
		//设置输出占空比
		CCDACTargetDuty=Duty_Calc(CurrentBuf);
		}
	//电流等于0，关闭所有输出
	else
		{
		DCDCEN=0;
	  CCDACTargetDuty=0;			//DCDC被关闭，禁用输出
		IsDCDCEnabled=0;  //标记DCDC已被关闭
		//电流输出为0
		CurrentBuf=0;
		}	
	//更新完毕上传PWM数值
	IsNeedToUploadPWM=1;
	}
