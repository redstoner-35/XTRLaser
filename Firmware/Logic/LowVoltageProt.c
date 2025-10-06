/****************************************************************************/
/** \file LowVoltageProt.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ���浲λ�Լ��������⹦��
���޼����⵲λ�µĵ�ص͵������������͹ػ��Ĺ��ܡ�ͬʱʵ��ϵͳ�����ж�̬���µ��
�����Ĺ��ܡ�

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "BattDisplay.h"
#include "ModeControl.h"
#include "LowVoltProt.h"
#include "TempControl.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "ADCCfg.h"
#include "SelfTest.h"
#include "SysConfig.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//�͵�ѹ�����ͱ�����������
#define BatteryAlertDelay 10 //��ؾ����ӳ�	
#define BatteryFaultDelay 2  //��ع���ǿ������/�ػ����ӳ�

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata unsigned char BattAlertTimer; //��ص͵�ѹ�澯����
static xdata unsigned char RampCurrentRiseAttmTIM; //�޼�����ָ������ļ�ʱ��	

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

static void StartBattAlertTimer(void)	//�͵�������������ʱ����ʼ���м�ʱ�ĺ���
	{
	//������ʱ��
	if(!BattAlertTimer)BattAlertTimer=1;
	}	

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

void RuntimeUpdateTo2S(void)	//���й����м���ص�ѹ�����µ�2Sģʽ
	{
	#ifndef USING_LD_NURM11T
	//��ص�ѹ��2Sģʽ����2Sģʽ�ѿ������˳�
	if(IsEnable2SMode||Data.RawBattVolt<4.35)return;
	//��ǰϵͳΪ��ﮣ���⵽�ߵ�ѹ�Զ�����2Sģʽ
	IsEnable2SMode=1;
	Trigger2SModeEnterInfo();
	SaveSysConfig(0);
	#else
	//���LDֻ�ܵ�ﮣ����ϵͳ��˫�ģʽ����ǿ�ƻص����
	if(IsEnable2SMode)
		{
		IsEnable2SMode=0;
		SaveSysConfig(0);
		}
	//��ص�ѹ���ڵ������ֵ������
	if(Data.RawBattVolt>4.35)ReportError(Fault_InputOVP);	
	#endif
	}
	
//��ص͵�������������
void BattAlertTIMHandler(void)
	{
	//�޼����⾯����ʱ
	if(RampCurrentRiseAttmTIM&&RampCurrentRiseAttmTIM<9)RampCurrentRiseAttmTIM++;
	//��������
	if(BattAlertTimer&&BattAlertTimer<(BatteryAlertDelay+1))BattAlertTimer++;
	}	

//��ص͵�����������
void BatteryLowAlertProcess(bool IsNeedToShutOff,ModeIdxDef ModeJump)
	{
	unsigned char Thr=BatteryFaultDelay;
	bit IsChangingGear;
	//��ȡ�ֵ簴����״̬
	if(getSideKey1HEvent())IsChangingGear=1;
	else IsChangingGear=getSideKeyHoldEvent();
	//���Ƽ�ʱ����ͣ
	if(!IsBatteryFault) //���û�з�����ѹ����
		{
		Thr=BatteryAlertDelay; //û�й��Ͽ�����һ�㽵��
		//��ǰ�ڻ����׶λ���û�и澯��ֹͣ��ʱ��,��������
		if(!IsBatteryAlert||IsChangingGear)BattAlertTimer=0;
		else StartBattAlertTimer();
		}
  else StartBattAlertTimer();//������ѹ�澯����������ʱ��
	//��ʱ����ʱ������ִ�ж�Ӧ�Ķ���
	if(BattAlertTimer>Thr)
		{
		//��ǰ��λ������Ҫ�ڴ����͵�������ʱ�����ػ���״̬	
		if(IsNeedToShutOff)ReturnToOFFState();
		//��ǰ���ڻ���ģʽ������ִ�н���������Ҫ�жϵ���Ƿ����Ȼ��ǿ�ƹر�
		else if(IsChangingGear&&IsBatteryFault)ReturnToOFFState();
		//����Ҫ�ػ���������������
		else
			{
			BattAlertTimer=0;//���ö�ʱ������ʼֵ
			SwitchToGear(ModeJump); //��λ��ָ����λ
			}
		}
	}		

//�޼����⿪��ʱ�ָ���ѹ���������Ĵ���	
void RampRestoreLVProtToMax(void)
	{
	if(IsBatteryAlert||IsBatteryFault)return;
	if(BattState==Battery_Plenty)SysCfg.RampCurrentLimit=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX; //��ص�������������״̬����λ��������
	
	}
	
//�޼�����ĵ͵�ѹ����
void RampLowVoltHandler(void)
	{
	if(!IsBatteryAlert&&!IsBatteryFault)//û�и澯
		{
		BattAlertTimer=0;
		if(BattState==Battery_Plenty) //��ص�������������״̬���������ӵ�������
			{
	    if(SysCfg.RampCurrentLimit<QueryCurrentGearILED())
				 {
			   if(!RampCurrentRiseAttmTIM)RampCurrentRiseAttmTIM=1; //������ʱ����ʼ��ʱ
				 else if(RampCurrentRiseAttmTIM<9)return; //ʱ��δ��
         RampCurrentRiseAttmTIM=1;
				 if(SysCfg.RampBattThres>CurrentMode->LowVoltThres)SysCfg.RampBattThres=CurrentMode->LowVoltThres; //��ѹ���ﵽ���ޣ���ֹ��������
				 else SysCfg.RampBattThres+=50; //��ѹ����ϵ�50mV
         if(SysCfg.RampCurrentLimit>QueryCurrentGearILED())SysCfg.RampCurrentLimit=QueryCurrentGearILED();//���ӵ���֮�������ֵ�Ƿ񳬳�����ֵ
				 else SysCfg.RampCurrentLimit+=250;	//�����ϵ�250mA		 
				 }
			else RampCurrentRiseAttmTIM=0; //�Ѵﵽ�������޽�ֹ��������
			}
		return;
		}
	else RampCurrentRiseAttmTIM=0; //������������λ�������ӵ����Ķ�ʱ��
	//��ѹ�澯������������ʱ��
	StartBattAlertTimer(); //��������������ʱ��
	if(IsBatteryFault&&BattAlertTimer>4)ReturnToOFFState(); //��ص�ѹ���ڹػ���ֵ����0.5�룬�����ر�
	else if(BattAlertTimer>BatteryAlertDelay) //��ص�λ����
		{
		if(SysCfg.RampCurrentLimit>500)SysCfg.RampCurrentLimit-=250; //�����µ�250mA
		if(SysCfg.RampBattThres>2850)SysCfg.RampBattThres-=25; //����25mV
    BattAlertTimer=1;//���ö�ʱ��
		}
	}
