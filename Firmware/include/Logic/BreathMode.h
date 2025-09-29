#ifndef _BreathMode_
#define _BreathMode_

//�ڲ�״̬
typedef enum
	{
	BreathMode_RampUp,
	BreathMode_MaintainHigh,
	BreathMode_RampDown,
	BreathMode_MaintainLow 
	}BreathModeFSMDef;

//����
#define CurrentRampUpInc 1 //����ģʽ�µ����������ٶ�
#define CurrentRampDownDec 1 //����ģʽ�µ����½����ٶ�
#define CurrentHighSustainTime 4 //����ģʽ�µ�������ߵ�ı���ʱ�䣨0.125S per LSB��
#define CurrentLowSustainTime 5  //����ģʽ�µ����ڹرյ�ı���ʱ�䣨��λ �룩

//����
void BreathFSM_Reset(void);
void BreathFSM_TIMHandler(void);
int BreathFSM_Calc(void);	
	
#endif
