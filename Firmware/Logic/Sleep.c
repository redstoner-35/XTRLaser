/****************************************************************************/
/** \file Sleep.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�ڳ�ʱ�䲻����ʱ�Զ��ر�
CPU����������ʱ�ӽ���STOP mode�����Լ������ĵ����˯�߹��ܡ�ͬʱ���ļ�ʵ���˳�
ʱ�䲻��ʱ�Զ����������ֵ籾��İ�ȫ���ơ����ⳤ�ھ��ò���������ʱ��С������ֱ��
ʹ����������¹ʡ�

**	History: 
/** \Date 2025/10/16 16:00
/** \Desc ������ʱ����ú�ִ���Զ��������ܵ���ش���ʵ�֡�

**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "LVDCtrl.h"
#include "SysConfig.h"
#include "LEDMgmt.h"
#include "VersionCheck.h"
#include "ActiveBeacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

#define SleepTimeOut 5 //ϵͳ�ڹػ����޲���״̬�½������ߵĳ�ʱʱ��(��)
#define LVKillSampleInterval 12 //ϵͳ����Դҹ�⿪��ʱ��Ƿѹ��ɱ��������(1��λ=8��)

/************** �Զ��������ܵļ��define **************/
#ifndef AutoLockTimeOut
  //���ϵͳû�ж����Զ���������ʱ�䣬����һ��0ֵ����������ȥ��
  #define AutoLockTimeOut 0
#endif

#define AutoLockCNTValue (AutoLockTimeOut/8)          //�Զ���������������ֵ
#if (AutoLockCNTValue > 0xFF)
  //�Զ�����ʱ�����������������ֵ
	#error "Auto Lock TimeOut is way too long and causing timer overflow."
	
#elif (AutoLockTimeOut < 0)
  //�Զ�����ʱ�䲻�����Ǹ���
	#error "Auto Lock TimeOut should be any number larger than zero!"

#elif (AutoLockTimeOut > 0)
  //����Ϸ����Զ�������ʱ�����ܿ���
	#message "Auto Lock Function of this firmware is enabled."
	
#else
  //�Զ�������ʱ���ر�
	#message "Auto Lock Function of this firmware is disabled.To enable this function"
	#message "you need to define the time of auto lock by add a <AutoLockTimeOut>=[time(Sec)]"
	#message "define in project global definition screen on Complier preference menu."
	#warning "Auto lock function set to disabled state will causing major security issues,USE AS YOUR OWN RISK!!!"
#endif

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

volatile unsigned int SleepTimer;	//˯�߶�ʱ��

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
static xdata unsigned char AutoLockTimer; //�Զ�������ʱ��

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

//��ֹ����ϵͳ����
static void DisableSysPeripheral(void)
	{
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //�ر�PWM��ADC
	LED_DeInit(); //��λLED������
	ActiveBeacon_Start(); //������Դҹ��ģ��
	
	//����Զ�������������ǿ�ƴ��Զ����Ѽ�ʱ��
	#if (AutoLockTimeOut > 0)
	LVD_Start();
	#endif
	}

//��������ϵͳ����
static void EnableSysPeripheral(void)
	{
	LVD_Disable(); //����ϵͳ�����������׶�֮ǰ�ر�WUT
	ADC_Init(); //��ʼ��ADC
	PWM_Init(); //��ʼ��PWM������
	LED_Init(); //��ʼ���ఴLED
	OutputChannel_Init(); //��ʼ�����ͨ��
	SystemTelemHandler(); //����һ��ADC�����г�ʼ����
	DisplayVBattAtStart(0); //ִ��һ���س�ʼ������	
	EnableADCAsync(); 			//���������ʼ����ϣ�����ADC�첽����ģʽ
	}

//���ϵͳ�Ƿ��������˯�ߵ�����
static char QueryIsSystemNotAllowToSleep(void)
	{
	//ϵͳ����ʾ��ص�ѹ�Ͱ汾�ţ�������˯��
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//ϵͳ������
	if(Current>0||IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//����˯��
	return 0;
	}	

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//���ض�ʱ��ʱ��
void LoadSleepTimer(void)	
	{
	//�����Զ�����ʱ��
  #if (AutoLockTimeOut > 0)
	AutoLockTimer=(unsigned char)AutoLockCNTValue;
	#endif
	//����˯��ʱ��
	if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=240; //���ϱ���ģʽ��ϵͳ˯��ʱ���Ϊ240S
	else SleepTimer=8*SleepTimeOut; 		
	}

//˯�߹�����
void SleepMgmt(void)
	{
	bit sleepsel;
	unsigned char ADCSampleCounter;
	//�ǹػ�����Ȼ����ʾ��ص�ѹ��ʱ��ʱ����λ��ֹ˯��
	if(QueryIsSystemNotAllowToSleep())LoadSleepTimer();
	//����˯�߿�ʼ����ʱ
	if(SleepTimer>0)
		{
		SleepTimer--;
		return;
		}
	//ʱ�䵽����������˯�߽׶�
	DisableSysPeripheral();
	ADCSampleCounter=LVKillSampleInterval; //װ��Ƿѹ��ɱ��ʱģ���ʱ��ֵ
	do
		{		
		//��STOP=1��ʹ��Ƭ������˯��
		STOP();  
		//����֮����Ҫ��6��NOP
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		//ϵͳ�ѻ��ѣ�������ʼ���
		if(GetIfSideKeyTriggerInt()) 
			{
			//��⵽ϵͳ������LVD���ѣ�������ɳ�ʼ���жϰ���״̬
			StartSystemTimeBase(); //����ϵͳ��ʱ���ṩϵͳ��ʱ����ʱ����
			MarkAsKeyPressed(); //������ǰ�������
			SideKey_SetIntOFF(); //�رղఴ�ж�
			do	
				{
				delay_ms(1);
				SideKey_LogicHandler(); //����ఴ����
				//�ఴ�����ļ�ⶨʱ������(ʹ��62.5mS����ʱ��,ͨ��2��Ƶ)
				if(!SysHFBitFlag)continue; 
				SysHFBitFlag=0;
				sleepsel=~sleepsel;
				if(sleepsel)SideKey_TIM_Callback();
				}
			while(!IsKeyEventOccurred()); //�ȴ���������		
			//ϵͳ����ɰ����¼���⣬��ʼ����������		
			EnableSysPeripheral();	
			return;
			}
		else //ִ���Զ�������Ƿѹ��ɱ��ʱ���жϴ���	
			{
			#if (AutoLockTimeOut > 0)
			//�ж�ϵͳ�Ƿ��Ѿ��Զ�����
			sleepsel=0;													//��ʼ����־λ�����ж��Ƿ���Ҫ�ر�WUT��ʱ��
			if(AutoLockTimer)AutoLockTimer--;		//ʱ�仹û���������¼�
			else if(!IsSystemLocked)
				{
				//ϵͳδ������ִ����������
				IsSystemLocked=1;
				LastMode=Mode_Low;								//����ϵͳ�ĵ�λģʽ���������
				//�������ο���ָʾ��������ģʽ
				StartSystemTimeBase(); 						//����ϵͳ��ʱ���ṩϵͳ��ʱ����ʱ����
				LED_Init(); 						          //��ʼ���ఴLED
				LEDMode=LED_RedBlinkThird;
				while(LEDMode==LED_RedBlinkThird)if(SysHFBitFlag)
					{
					//�������κ�ɫ����
					sleepsel=~sleepsel;
					if(sleepsel)LEDControlHandler();	
					SysHFBitFlag=0;
					}	
				//��˸��ʾ��ϣ��������ò��ر�LED
				sleepsel=0;
				SaveSysConfig(0);
				DisableSysHBTIM();    
				LED_DeInit(); 				//��λLED���������ر�ϵͳ��ʱ��
				ActiveBeacon_Start(); //������Դҹ��ģ��
				}
			else sleepsel=1; //ϵͳ����������ʱ�����ж�LVD�Ƿ���Ҫ�ر��������Ƿ�ر�WUT
			#else 
			//�Զ�������ʱ���رգ�ʹ��ϵͳ�����ж�Ƿѹ��ɱ�Ƿ����
			sleepsel=1;		
			#endif
			//�ж�ϵͳ���Ƿ�ִ��Ƿѹ��ɱ
			if(ADCSampleCounter)ADCSampleCounter--;
			else if(!ActiveBeacon_LVKill())ADCSampleCounter=LVKillSampleInterval;    //Ƿѹ��ɱ����ʧ�ܵ�ص�ѹ��Ȼ�㹻,��λ����������
      else if(sleepsel)LVD_Disable();   //Ƿѹ��ɱ�Ѿ�������Զ������Ѿ���ʱ�������ر�WUT���볹����˯
			}
		}
	while(!SleepTimer);
	}
