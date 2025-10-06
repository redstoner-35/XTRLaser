/****************************************************************************/
/** \file ActiveBeacon.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�ڽ�������ģʽ����ಿ����
����΢��ָʾ����λ�ú�ϵͳ״̬�������û������Ĺ��ܡ�

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "LEDMgmt.h"
#include "ModeControl.h"
#include "BattDisplay.h"
#include "ActiveBeacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

#define ActiveBeaconOFFVolt 2900 //���õ���Ч���ڵ�ص�ѹǷѹ��ر���Դҹ������ض�������ֵ(mV)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

extern volatile unsigned int SleepTimer; //ϵͳ˯�߼�ʱ��
extern xdata unsigned char AutoLockTimer; //�Զ�������ʱ��

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
static bit IsEnableActiveBeacon=0; //�ڲ���־λ����Դ�ű��Ƿ���

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

//ϵͳ��������֮ǰ��������Դҹ��ģ��
void ActiveBeacon_Start(void)
	{
	GPIOCfgDef LEDInitCfg;
	//�����ص�ѹ����2.9VΪ�˱��⵼�µ�س��׶�������ֹ�򿪶�λLED����Ȼ�Ļ����о����û������رգ�
	if(CellVoltage<ActiveBeaconOFFVolt||!IsEnableIdleLED)return;
	//���ýṹ��
	LEDInitCfg.Mode=GPIO_IPU;
  LEDInitCfg.Slew=GPIO_Slow_Slew;		
	LEDInitCfg.DRVCurrent=GPIO_High_Current; //����Ϊ���������ɫ���鷢��΢��
	//����GPIO������LVD
	LVD_Start();
	IsEnableActiveBeacon=1; //����Ѿ�ʹ����Դҹ�⹦��
	if(CurrentMode->ModeIdx==Mode_Fault)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);
	else GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);  //���ϵͳ�ǹ���״̬������������ʾ�û���ǰϵͳ�޷�����
	}
	
//��Դҹ��ģ�鿪��֮�󣬽��е��Ƿѹ��ɱ�����ض�����ģ��
bit ActiveBeacon_LVKill(void)
	{	
	//��Դҹ���ѱ��رգ���ִ��Ƿѹ��ɱ����
	if(!IsEnableActiveBeacon)return 1;
	//����ADC�����в�������
	ADC_Init(); 																 //��ʼ��ADC
	SystemTelemHandler(); 
	CellVoltage=(int)(Data.BatteryVoltage*1000); //����һ��ADC����ȡ�����µ�ص�ѹ
	//���Ƿѹ���ر���Դҹ������ض���
	if(CellVoltage<ActiveBeaconOFFVolt)
		{
		IsEnableActiveBeacon=0;
		LED_Init(); 
		}
	//�����ϣ��ر�ADC����˯�߶�ʱ��=0��ϵͳ��������˯��
	ADC_DeInit(); 
	SleepTimer=0;
	return 0;
	}
