#ifndef _TC_
#define _TC_

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
