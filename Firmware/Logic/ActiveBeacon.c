#include "GPIO.h"
#include "PinDefs.h"
#include "cms8s6990.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "LEDMgmt.h"
#include "ModeControl.h"
#include "BattDisplay.h"
#include "ActiveBeacon.h"

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
	if(CurrentMode->ModeIdx==Mode_Fault)GPIO_ConfigGPIOMode(RedLEDIOG,GPIOMask(RedLEDIOx),&LEDInitCfg);
	else GPIO_ConfigGPIOMode(GreenLEDIOG,GPIOMask(GreenLEDIOx),&LEDInitCfg);  //���ϵͳ�ǹ���״̬������������ʾ�û���ǰϵͳ�޷�����
	}
	
//��Դҹ��ģ�鿪��֮�󣬽��е��Ƿѹ��ɱ�����ض�����ģ��
void ActiveBeacon_LVKill(void)
	{	
	extern volatile unsigned int SleepTimer;
	//����ADC�����в�������
	ADC_Init(); 																 //��ʼ��ADC
	SystemTelemHandler(); 
	CellVoltage=(int)(Data.BatteryVoltage*1000); //����һ��ADC����ȡ�����µ�ص�ѹ
	//���Ƿѹ���ر���Դҹ������ض���
	if(CellVoltage<ActiveBeaconOFFVolt)
		{
		LVD_Disable(); 
		LED_Init(); 
		}
	//�����ϣ��ر�ADC����˯�߶�ʱ��=0��ϵͳ��������˯��
	ADC_DeInit(); 
	SleepTimer=0;
	}