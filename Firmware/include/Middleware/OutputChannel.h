#ifndef _OCH_
#define _OCH_

//输出通道参数设置
#define MainChannelShuntmOhm 10 //主通道的检流电阻阻值(mR)

//PWMDAC参数配置
#define VdivUpResK 220 //运放分压部分的上端电阻(KΩ)
#define PWMDACResK 10 //PWMDAC的电阻阻值(KΩ)
#define VdivDownResK 5.1 //运放分压部分的下端电阻(KΩ)
#define CurrentOffset 99.2 //高电流通道下的电流偏差值(单位%)

//外部参考
extern xdata int Current; //电流值
extern xdata int CurrentBuf; //存储当前已经上传的电流值 

//函数
bit GetIfOutputEnabled(void);
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OutputChannel_TestRun(void);

#endif
