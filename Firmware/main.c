/****************************************************************************/
/** \file main.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ�����ϵͳ������������

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "GPIO.h"
#include "delay.h"
#include "SideKey.h"
#include "LEDMgmt.h"
#include "ADCCfg.h"
#include "PWMCfg.h"
#include "LVDCtrl.h"
#include "SysReset.h"
#include "LowVoltProt.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SelfTest.h"
#include "SOS.h"
#include "BreathMode.h"
#include "Beacon.h"

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
bit TaskSel=0;  //ѡ�����ϵͳ����

/****************************************************************************/
/*	External Function prototypes definition
****************************************************************************/
void SleepMgmt(void);
void MaskUnusedIO(void);

//������
void main()
	{
	//ʱ�Ӻ�RSTCU��ʼ��	
	ClearSoftwareResetFlag();
	LVD_Disable(); 				 //����ϵͳǰÿ��ȷ��WUT����һ����֪״̬
  StartSystemTimeBase(); //����ϵͳ��ʱ���ṩϵͳ��ʱ����ʱ����
	//��ʼ������
	ADC_Init(); //��ʼ��ADC
	PWM_Init(); //����PWM��ʱ��
	LED_Init(); //��ʼ���ఴLED
  SideKeyInit(); //�ఴ��ʼ��	
	OutputChannel_Init(); //�����ʼ��	
	ModeFSMInit(); //��ʼ��ģʽ״̬��
  DisplayVBattAtStart(1); //��ʾ���״��
	MaskUnusedIO();   //���ε����е�Ƭ��δʹ�õ�IO���ʹ�������
	EnableADCAsync(); //����ADC���첽ģʽ��ߴ����ٶ�
	//��ѭ��	
  while(1)
		{
	  //ʵʱ����
		SystemTelemHandler();
		BatteryTelemHandler(); //��ȡADC�͵����Ϣ	
		SideKey_LogicHandler(); //����ఴ����
		ThermalMgmtProcess(); //�¶ȹ���
		ModeSwitchFSM(); //��λ״̬��
		OutputChannel_Calc();  //���ͨ������
		PWM_OutputCtrlHandler(); //����PWM�������	
		//8Hz����ʱ����
		if(!SysHFBitFlag)continue; //ʱ��û������������
			
		//Task0������������Ƚϴ������
    if(!TaskSel)
			{
			LEDControlHandler();//�ఴָʾLED���ƺ���	
			BattAlertTIMHandler(); //��ؾ�����ʱ����
			OutputFaultDetect(); //������ϼ��
			ThermalPILoopCalc(); 				//����������	
			ShowEntered2SModeProc(); //�������2Sģʽ����ʾ
			SleepMgmt(); //˯�ߴ���
			BreathFSM_TIMHandler(); //����ģʽ״̬����ʱ����
			
			//���������������ѡ����з�ת������һ��
			TaskSel=1;
		  }			
		//Task1������������Ƚ�С�ļ�ʱ����
		else
			{	
			SideKey_TIM_Callback();//�ఴ�����ļ�ⶨʱ������		
			BattDisplayTIM(); //��ص�����ʾTIM
			DisplayErrorTIMHandler(); //���ϴ�����ʾ
			ModeFSMTIMHandler(); //ģʽ״̬������
			HoldSwitchGearCmdHandler(); //������������
			SOSTIMHandler(); //SOS��ʱ��
			RuntimeUpdateTo2S(); //��̬����ص�ѹ�����µ�2S
			BeaconFSM_TIMHandler(); //�ű��ʱ��
				
			//���������������ѡ����з�ת������һ��
			TaskSel=0;
			}
		//������ϣ���flag����
		SysHFBitFlag=0;	
		}
	}
