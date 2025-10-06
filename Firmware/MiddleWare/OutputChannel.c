/****************************************************************************/
/** \file OutputChannel.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ�Ϊ�в��豸�����ļ�����������ϲ��߼��㷴����Ŀ���������
ֵ���㲢�ٿ�PWMDAC���ָ����LD���������LD�����𶯱������ܡ�

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"
#include "delay.h"
#include "OutputChannel.h"
#include "ModeControl.h"
#include "ADCCfg.h"
#include "TempControl.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//PWMDAC��������
#define VdivUpResK 220 //�˷ŷ�ѹ���ֵ��϶˵���(K��)
#define PWMDACResK 10 //PWMDAC�ĵ�����ֵ(K��)
#define VdivDownResK 5.1 //�˷ŷ�ѹ���ֵ��¶˵���(K��)
#define CurrentOffset 99.2 //�ߵ���ͨ���µĵ���ƫ��ֵ(��λ%)

//�����ͨ����������
#define MainChannelShuntmOhm 10 //��ͨ���ļ���������ֵ(mR)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata int Current; //Ŀ�����(mA)
xdata int CurrentBuf; //�洢��ǰ�Ѿ��ϴ��ĵ���ֵ 

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable and SFR definitions('static and sfr')
****************************************************************************/
static bit IsDCDCEnabled; //DCDC�Ƿ�ʹ��
static bit IsSlowRamp; //��������Ramp

sbit DCDCEN=DCDCENIOP^DCDCENIOx; //DCDCʹ��Pin
/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/
static float Duty_Calc(int CurrentInput)		//�ڲ����ڼ���PWMDACռ�ձȵĺ���	
	{
	float buf;
	//����ʵ��ռ�ձ�
	buf=(float)CurrentInput*(float)MainChannelShuntmOhm; //���봫�����ĵ���(mA)�����Լ���������ֵ(mR)�õ��˷Ŷ�������ѹ(uV)
	buf/=(float)1000; //uVתmV
	buf/=((float)VdivDownResK/(float)(VdivUpResK+VdivDownResK+PWMDACResK)); //���˷Ŷ�������ѹ���Ե���ķ�ѹ�����õ�DAC�˵ĵ�ѹ
	buf*=(float)CurrentOffset/(float)100; //���Խ���ϵ����������
	buf/=Data.MCUVDD*(float)1000; //�����Ŀ��DAC�����ѹ��PWMDAC�����������ѹ(MCUVDD)֮��ı�ֵ
	buf*=100; //ת��Ϊ�ٷֱ�
	//������	
	return buf>100?100:buf;
	}

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/
//��ȡ����Ƿ���
bit GetIfOutputEnabled(void)
	{
	return IsDCDCEnabled;
	}

//��ʼ������
void OutputChannel_Init(void)
	{
	GPIOCfgDef OCInitCfg;
	//���ýṹ��
	OCInitCfg.Mode=GPIO_Out_PP;
  OCInitCfg.Slew=GPIO_Fast_Slew;		
	OCInitCfg.DRVCurrent=GPIO_High_Current; //��MOSFET,��Ҫ������б��
	//��ʼ��bit
	DCDCEN=0;
	//��ʼ����IO	
	GPIO_ConfigGPIOMode(DCDCENIOG,GPIOMask(DCDCENIOx),&OCInitCfg);				
	//ϵͳ�ϵ�ʱ��������Ϊ0
	Current=0;
	CurrentBuf=0;
	IsDCDCEnabled=0;
	IsSlowRamp=0;
	}
	
//���ͨ������
void OutputChannel_Calc(void)
	{
	int TargetCurrent;
	extern bit IsBurnMode;
	//���ݵ�ǰ�������������״̬�ó�ʵ��Ҫ����¿ؼ��㺯���ĵ���
	//��ȡĿ�������Ӧ���¿ؼ�Ȩ����
	if(Current>0)
		{
		//ȡ���¿���������
		TargetCurrent=ThermalILIMCalc();
		//���Ŀ�����С�ڵ�ǰ��λ���¿�����ֵ����Ӧ�õ�ǰ���õĵ���ֵ
		if(Current<TargetCurrent)TargetCurrent=Current;
		}
	//����ֵΪ0����-1��ֱ�Ӷ�ȡĿ�����ֵ
	else TargetCurrent=Current;
	//������Ч���ظ�����
	if(CurrentBuf==TargetCurrent)return;	
	//��������0���������ѡ���Ӧͨ��
	if(TargetCurrent>0)
		{
		//����LED�ĵ���б��������
		if((TargetCurrent-CurrentBuf)>600)IsSlowRamp=1; //��⵽�ǳ���ĵ���˲̬������屬�����������
		if(IsSlowRamp)
			{
			//��ʼ�������ӵ���
			if(CurrentBuf==0)
				{
				//��ǰϵͳ����Ϊ0���жϴ���ĵ���ֵ���ӵ͵�����ʼ���
				if(TargetCurrent<600)CurrentBuf=TargetCurrent;
				else CurrentBuf=600;
				}
			//ϵͳ������Ϊ0��������ʼ���ӵ���
			else if(IsBurnMode)CurrentBuf+=50;    //����ģʽ������Ѹ�����ӵ����������Ч��
      else switch(CurrentMode->ModeIdx)
				{
				
				case Mode_Beacon:CurrentBuf+=1000;break; //�ű�ģʽ�������������
				case Mode_SOS:
				case Mode_SOS_NoProt:CurrentBuf+=400;break;  //SOSģʽ�¿������ӵ���������βӰ���ж�
				default:CurrentBuf+=2;											 //���൲λ����Ĭ�ϻ�������
				}
			if(CurrentBuf>=TargetCurrent)
				{
				IsSlowRamp=0;
				CurrentBuf=TargetCurrent; //�޷���������Ŀ�������������ֵ
				}
			}
		else CurrentBuf=TargetCurrent; //ֱ��ͬ��		
		//EN���ڹر�״̬������DCDC����PWMDAC=0�ȴ�һ��ʱ��������������嵼����˸������
		if(!IsDCDCEnabled)
				{
				DCDCEN=1;  
				delay_ms(5); //��PWMDAC���=0����һ��ʱ���������ݽ���Ԥ��磬Ȼ�����͸���ֵ������˸
				IsDCDCEnabled=1; //���DCDC�Ѿ���ʼ����
				}
		//�������ռ�ձ�
		CCDACTargetDuty=Duty_Calc(CurrentBuf);
		}
	//��������0���ر��������
	else
		{
		DCDCEN=0;
	  CCDACTargetDuty=0;			//DCDC���رգ��������
		IsDCDCEnabled=0;  //���DCDC�ѱ��ر�
		//�������Ϊ0
		CurrentBuf=0;
		}	
	//��������ϴ�PWM��ֵ
	IsNeedToUploadPWM=1;
	}
