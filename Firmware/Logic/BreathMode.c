#include "BreathMode.h"
#include "ModeControl.h"

//�ڲ�����
static xdata int BreathCurrentBuf;
static xdata unsigned char BreathFSMTIM;
static xdata BreathModeFSMDef BreathFSM;


//��λ����ģʽ״̬��
void BreathFSM_Reset(void)
	{
	BreathFSM=BreathMode_RampUp;
	BreathCurrentBuf=CurrentMode->MinCurrent;
	BreathFSMTIM=0;
	}

//����ģʽ״̬����ʱ����
void BreathFSM_TIMHandler(void)
	{
	if(BreathFSMTIM)BreathFSMTIM--;
	}
	
//����ģʽ״̬������
int BreathFSM_Calc(void)
	{
	int Imax=IsEnable2SMode?QueryCurrentGearILED():1800;
	//ʵ�ʵ�״̬������
	switch(BreathFSM)
		{
		case BreathMode_RampUp:
			//��������͵��������������
			if(BreathCurrentBuf<Imax)
				{
				//������û����ߣ���������
				BreathCurrentBuf+=CurrentRampUpInc;
				if(BreathCurrentBuf>Imax)BreathCurrentBuf=Imax; //���Ƶ���������ܳ�����λ����
				}
			else
				{
				//�����Ѿ�������ˣ����뱣��״̬
				BreathFSM=BreathMode_MaintainHigh;
				BreathFSMTIM=CurrentHighSustainTime;
				}
			break;
		case BreathMode_MaintainHigh:
			//���ּ�ʱ�����ڼ�ʱ����������ߵ���
			if(BreathFSMTIM)return Imax;
		  //���ּ�ʱ����������ʼ��������ݼ��׶�
		  BreathFSM=BreathMode_RampDown;
			break;
		case BreathMode_RampDown:
			//�������Եݼ�
			if(BreathCurrentBuf>CurrentMode->MinCurrent)
				{
				//������û����ͣ����Եݼ�
				BreathCurrentBuf-=CurrentRampDownDec;
				if(BreathCurrentBuf<CurrentMode->MinCurrent)BreathCurrentBuf=CurrentMode->MinCurrent; //���Ƶ���������С����Сֵ
				}
			else
				{
				//����������ˣ�����ر�״̬
				BreathFSM=BreathMode_MaintainLow;
				BreathFSMTIM=CurrentLowSustainTime*8;
				}
			break;
		case BreathMode_MaintainLow:
			//���ּ�ʱ�����ڼ�ʱ������0�����LEDϨ��
			if(BreathFSMTIM)return 0;
		  //���ּ�ʱ�����������½�����������׶ο�ʼ��һ��ѭ��	
      BreathFSM=BreathMode_RampUp;		
		  break;
		}
	//����Ĭ��������ػ�����
	return BreathCurrentBuf;
	}
