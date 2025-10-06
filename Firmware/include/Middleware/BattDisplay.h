#ifndef _BattDs_
#define _BattDs_

//״̬����enum
typedef enum
	{
	Battery_Plenty, //��ص�������
	Battery_Mid, //��ص�����Ϊ����
	Battery_Low, //��ص�������
	Battery_VeryLow //��ص������ز���
	}BattStatusDef;

typedef enum
	{
  BattVdis_Waiting, //�ȴ���ʾ�׶�
	BattVdis_PrepareDis, //׼����ʾ
	BattVdis_DelayBeforeDisplay, //�ӳ�һ��ʱ��
	BattVdis_Show10V, //��ʾʮλ
	BattVdis_Gap10to1V, //ʮλ�͸�λ֮��ĵȴ�
	BattVdis_Show1V, //��ʾ��λ
	BattVdis_Gap1to0_1V, //��λ��ʮ��λ֮��ĵȴ�
	BattVdis_Show0_1V, //��ʾС�����һλ(0.1V)
	BattVdis_WaitShowChargeLvl, //�ȴ�һ��ʱ�����ʾ��ǰ����
	BattVdis_ShowChargeLvl, //��ʾ��ص����ĵȴ�
	BattVdis_WaitShowTempState,
	BattVdis_ShowTempState	
	}BattVshowFSMDef; //��ص�����ʾ����

//�ⲿ�ο�
extern bit IsBatteryAlert; //��ص͵羯�淢��
extern bit IsBatteryFault; //��ص͵������Ϸ���
extern BattStatusDef BattState; //���״̬
extern xdata int CellVoltage; //�˲�֮��ĵ�����Ч���ڵ�ѹ������Ӧ������ڲ�ƽ��״̬�µ�ʵ�ʵ��ڵ�ѹ��
extern xdata BattVshowFSMDef VshowFSMState; //״̬��״̬	

//����������ʾ�ຯ��	
void Trigger2SModeEnterInfo(void);//��������2Sģʽ�Ŀ���������ʾ
void TriggerTShowDisplay(void); //�����¶���ʾ	
void TriggerVshowDisplay(void); //������ص�ѹ��ʾ	
	
//����
bit IsKeyFSMCanEnable(void); //�ڲ��������������ڽ����ϵ��״ε��������ڼ��ֹ����״̬����Ӧ
void ShowEntered2SModeProc(void); //�������2Sģʽ����2�ε���ʾ����
void BattDisplayTIM(void); //��ص�����ʾ��������
void DisplayVBattAtStart(bit IsPOR); //������ʱ��ʾ��ص�ѹ
void BatteryTelemHandler(void);  //��ز�����ָʾ�ƿ���
bit LowPowerStrobe(void); //�͵�����ʾ��
	
#endif
