/****************************************************************************/
/** \file BreathMode.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�������⹦����������Ѳ��
���������ܣ�������������������Ȼ�󻺽������Ϩ�𣬵�һ�����ѭ��������

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "BreathMode.h"
#include "ModeControl.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//����ģʽ�Ĳ�������
#define CurrentRampUpInc 1 //����ģʽ�µ����������ٶ�
#define CurrentRampDownDec 1 //����ģʽ�µ����½����ٶ�
#define CurrentHighSustainTime 4 //����ģʽ�µ�������ߵ�ı���ʱ�䣨0.125S per LSB��
#define CurrentLowSustainTime 5  //����ģʽ�µ����ڹرյ�ı���ʱ�䣨��λ �룩

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	//�ڲ�״̬
	BreathMode_RampUp,
	BreathMode_MaintainHigh,
	BreathMode_RampDown,
	BreathMode_MaintainLow 
	}BreathModeFSMDef;
	
/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata int BreathCurrentBuf;
static xdata unsigned char BreathFSMTIM;
static xdata BreathModeFSMDef BreathFSM;
static xdata char BreathModeDivCNT;

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/
	
//��λ����ģʽ״̬��
void BreathFSM_Reset(void)
	{
	BreathFSM=BreathMode_RampUp;
	BreathCurrentBuf=CurrentMode->MinCurrent;
	BreathFSMTIM=0;
	BreathModeDivCNT=0;
	}

//����ģʽ״̬����ʱ����
void BreathFSM_TIMHandler(void)
	{
	if(BreathFSMTIM)BreathFSMTIM--;
	}
	
//����ģʽ״̬������
int BreathFSM_Calc(void)
	{
	int Imax=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX;
	//ʵ�ʵ�״̬������
	switch(BreathFSM)
		{
		case BreathMode_RampUp:
			//��ǰ��Ƶ���������ڼ���
		  if(BreathModeDivCNT>0)BreathModeDivCNT--;
			//��������͵��������������
			else if(BreathCurrentBuf<Imax)
				{
				//��̬���ط�Ƶ������ʵ�ֵ����仯��һ��
				BreathModeDivCNT=(char)(QueryCurrentGearILED()/Imax)-1;
				if(BreathModeDivCNT<0)BreathModeDivCNT=0;
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
			//��ǰ��Ƶ���������ڼ���
		  if(BreathModeDivCNT>0)BreathModeDivCNT--;
			//�������Եݼ�
			else if(BreathCurrentBuf>CurrentMode->MinCurrent)
				{
				//��̬���ط�Ƶ������ʵ�ֵ����仯��һ��
				BreathModeDivCNT=(char)(QueryCurrentGearILED()/Imax)-1;
				if(BreathModeDivCNT<0)BreathModeDivCNT=0;
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
