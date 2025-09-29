#include "cms8s6990.h"
#include "delay.h"
#include "SideKey.h"
#include "PWMCfg.h"
#include "PinDefs.h"
#include "ModeControl.h"
#include "OutputChannel.h"
#include "BattDisplay.h"
#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "VersionCheck.h"
#include "ActiveBeacon.h"

//˯�߶�ʱ��
volatile unsigned int SleepTimer;

//��ֹ����ϵͳ����
static void DisableSysPeripheral(void)
	{
	DisableSysHBTIM(); 
	PWM_DeInit();
	ADC_DeInit(); //�ر�PWM��ADC
	LED_DeInit(); //��λLED������
	ActiveBeacon_Start(); //������Դҹ��ģ��
	}

//��������ϵͳ����
static void EnableSysPeripheral(void)
	{
	ADC_Init(); //��ʼ��ADC
	PWM_Init(); //��ʼ��PWM������
	LED_Init(); //��ʼ���ఴLED
	OutputChannel_Init(); //��ʼ�����ͨ��
	SystemTelemHandler(); //����һ��ADC�����г�ʼ����
	DisplayVBattAtStart(0); //ִ��һ���س�ʼ������	
	EnableADCAsync(); 			//���������ʼ����ϣ�����ADC�첽����ģʽ
	}

//���ض�ʱ��ʱ��
void LoadSleepTimer(void)	
	{
	//����˯��ʱ��
	if(CurrentMode->ModeIdx==Mode_Fault)SleepTimer=240; //���ϱ���ģʽ��ϵͳ˯��ʱ���Ϊ240S
	else SleepTimer=8*SleepTimeOut; 		
	}

//���ϵͳ�Ƿ��������˯�ߵ�����
static char QueryIsSystemNotAllowToSleep(void)
	{
	//ϵͳ����ʾ��ص�ѹ�Ͱ汾�ţ�������˯��
	if(VshowFSMState!=BattVdis_Waiting||VChkFSMState!=VersionCheck_InAct)return 1;
	//ϵͳ������
	if(IsLargerThanOneU8(CurrentMode->ModeIdx))return 1;
	//����˯��
	return 0;
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
		//Ƿѹ��ɱ��ʱ����ʱ��
		else if(ADCSampleCounter)ADCSampleCounter--;
		//ϵͳ��WUT���ѣ�����ADC������ص�ѹ������˯��
		else
			{
			ActiveBeacon_LVKill();  //���е�ص�ѹ���
			ADCSampleCounter=11;    //Ƿѹ��ɱ����Ҫ�ر�Ƶ���Ĳ���,��λ����������
			}
		}
	while(!SleepTimer);
	}
