#ifndef _LVProt_
#define _LvProt_

#include "stdbool.h"
#include "ModeControl.h"

//��������
#define BatteryMaximumTurboVdroop 1.4  //�������������У�����������ĺ�����ǰ��ѹ��(V)
#define BatteryAlertDelay 10 //��ؾ����ӳ�	
#define BatteryFaultDelay 2 //��ع���ǿ������/�ػ����ӳ�

//����
void RuntimeUpdateTo2S(void); //��̬����ص�ѹ�����µ�2S
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //��ͨ��λ�ľ�������
void RampLowVoltHandler(void); //�޼������ר������
void BattAlertTIMHandler(void); //��ص͵�������������
void RampRestoreLVProtToMax(void); //ÿ�ο��������޼�ģʽʱ���Իָ�����

#endif
