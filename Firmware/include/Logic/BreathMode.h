#ifndef _BreathMode_
#define _BreathMode_

//内部状态
typedef enum
	{
	BreathMode_RampUp,
	BreathMode_MaintainHigh,
	BreathMode_RampDown,
	BreathMode_MaintainLow 
	}BreathModeFSMDef;

//参数
#define CurrentRampUpInc 1 //呼吸模式下电流上升的速度
#define CurrentRampDownDec 1 //呼吸模式下电流下降的速度
#define CurrentHighSustainTime 4 //呼吸模式下电流在最高点的保持时间（0.125S per LSB）
#define CurrentLowSustainTime 5  //呼吸模式下电流在关闭点的保持时间（单位 秒）

//函数
void BreathFSM_Reset(void);
void BreathFSM_TIMHandler(void);
int BreathFSM_Calc(void);	
	
#endif
