#ifndef _LVProt_
#define _LvProt_

//�ڲ�����
#include "stdbool.h"
#include "ModeControl.h"

//����
void RuntimeUpdateTo2S(void); //��̬����ص�ѹ�����µ�2S
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump); //��ͨ��λ�ľ�������
void RampLowVoltHandler(void); //�޼������ר������
void BattAlertTIMHandler(void); //��ص͵�������������
void RampRestoreLVProtToMax(void); //ÿ�ο��������޼�ģʽʱ���Իָ�����

#endif
