#ifndef _TC_
#define _TC_

//PI����������С��������
#define ILEDRecoveryTime 120 //ʹ�û����������������ж�ʱ������������������ۼӵ����ʱ������ִ��һ�ε���(��λ��)
#define SlowStepDownTime 60 //ʹ�û����������������ж�ʱ������������������ۼӵ����ʱ������ִ��һ�ε���(��λ��)
#define IntegralCurrentTrimValue 2000 //�������������ĵ����޵������ֵ(mA)
#define IntegralFactor 16 //����ϵ��(ÿ��λ=1/8�룬Խ��ʱ�䳣��Խ�ߣ�6=ÿ���ӽ���40mA�ĵ���)
#define MinumumILED 390 //����ϵͳ���ܴﵽ����͵���(mA)

//������������
#define ILEDConstant 750 //����ϵͳ���¿صĳ�����������(mA)
#define ILEDConstantFoldback 500 //�ڽӽ��¶ȼ���ʱ�Ľ���ϵͳ�ڵĳ�����������(mA)

//�¶�����
#define ForceOffTemp 65 //���ȹػ��¶�
#define ForceDisableTurboTemp 50 //�������¶��޷����뼫��
#define ConstantTemperature 46 //�Ǽ�����λ�¿�������ά�ֵ��¶�
#define ReleaseTemperature 35 //�¿��ͷŵ��¶�

/*   �������������Զ����壬�����޸ģ�    */
#define IntegrateFullScale IntegralCurrentTrimValue*IntegralFactor

#if (IntegrateFullScale > 32000)

#error "Error 001:Invalid Integral Configuration,Trim Value or time-factor out of range!"

#endif

#if (IntegrateFullScale <= 0)

#error "Error 002:Invalid Integral Configuration,Trim Value or time-factor must not be zero or less than zero!"

#endif

//����
int ThermalILIMCalc(void); //�����¿�ģ������������
void ThermalMgmtProcess(void); //�¿ع�����
void RecalcPILoop(int LastCurrent); //������ʱ�����¼���PI��·
void ThermalPILoopCalc(void); //�¿�PI��·�ļ���

//�ⲿFlag
extern bit IsPauseStepDownCalc; //�Ƿ���ͣ�¿صļ������̣���bit=1����ǿ�Ƹ�λ�����¿�ϵͳ�����ǻ���ͣ���㣩
extern bit IsDisableTurbo; //�رռ�������
extern bit IsForceLeaveTurbo; //ǿ���˳�����

#endif
