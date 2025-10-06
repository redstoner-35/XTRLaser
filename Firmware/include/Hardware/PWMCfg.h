#ifndef _PWM_
#define _PWM_

//外部引用
extern xdata float CCDACTargetDuty;	//恒流注入DAC的目标占空比
extern bit IsNeedToUploadPWM; //需要更新PWM寄存器应用输出
	
//函数
void PWM_Init(void);
void PWM_DeInit(void);
void PWM_OutputCtrlHandler(void);	
	
#endif
