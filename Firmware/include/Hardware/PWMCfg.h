#ifndef _PWM_
#define _PWM_

//�ⲿ����
extern xdata float CCDACTargetDuty;	//����ע��DAC��Ŀ��ռ�ձ�
extern bit IsNeedToUploadPWM; //��Ҫ����PWM�Ĵ���Ӧ�����
	
//����
void PWM_Init(void);
void PWM_DeInit(void);
void PWM_OutputCtrlHandler(void);	
	
#endif
