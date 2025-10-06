/****************************************************************************/
/** \file PWM.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ�����ʵ��ϵͳ�������PWM��������ʵ��PWMDAC���ܿ��Ƽ�����
�ĵ���������

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

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/
#define PWM_Enable() 	PWMFBKC=0x00;PWMCNTE=0x01 //ʹ��ͨ��0�ļ�������PWM��ʼ����(PWMʹ�ܲ���)
#define SysFreq 48000000 //ϵͳʱ��Ƶ��(��λHz)
#define PWMFreq 6000 //PWMƵ��(��λHz)	
#define PWMStepConstant (SysFreq/PWMFreq)-1 //PWM�����Զ�����

#if (PWMStepConstant > 0xFFFE)
  //�Զ����PWM����ֵ�Ƿ�Ϸ�
	#error "PWM Frequency is too low which causing PWM Counter to overflow!"
#endif
/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata float CCDACTargetDuty;	//Ŀ��PWMռ�ձ�(%)
bit IsNeedToUploadPWM; 				//�Ƿ���Ҫ����PWM

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable and SFR definitions('static and sfr')
****************************************************************************/
static bit IsPWMLoading; 				 //PWM���ڼ�����
static bit IsNeedToEnableOutput; //�Ƿ���Ҫ�������

sbit PWMOut=PWMIOP^PWMIOx;
/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

static void UploadPWMValue(void)	//�ϴ�PWMֵ
	{
	PWMLOADEN=0x01; //����ͨ��0��PWMֵ
	while(PWMLOADEN&0x01); //�ȴ����ؽ���
	}
/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

void PWM_DeInit(void)	//�ر�PWM��ʱ��
	{
	//����Ϊ��ͨGPIO
	GPIO_SetMUXMode(PWMIOG,PWMIOx,GPIO_AF_GPIO);
	//�ر�PWMģ��
	PWMCNTCLR=0x01; //��λPWM0�ļ�����
	PWMOE=0x00;
	PWMCNTE=0x00;		//�ر�PWM������
	PWM45PSC=0x00;
	PWM01PSC=0x00;  //�ر�PWM��Ƶ��ʱ��
	}

void PWM_Init(void)	//PWM��ʱ����ʼ��
	{
	GPIOCfgDef PWMInitCfg;
	//���ýṹ��
	PWMInitCfg.Mode=GPIO_Out_PP;
  PWMInitCfg.Slew=GPIO_Fast_Slew;		
	PWMInitCfg.DRVCurrent=GPIO_High_Current; //��FanPWM������Ҫ�ܸߵ�����б��
	//����GPIO
  PWMOut=0; 				//PWM������ڷ�PWMģʽ��ʼ�����0
	GPIO_ConfigGPIOMode(PWMIOG,GPIOMask(PWMIOx),&PWMInitCfg); 
	//����PWM������
	PWMCON=0x00; 		//PWMͨ��Ϊ��ͨ������ģʽ�����¼������رշǶԳƼ�������	
	PWMOE=0x01; 		//��PWM���ͨ��0
	PWM01PSC=0x01;  //��Ԥ��Ƶ���ͼ�����ʱ�� 
  PWM0DIV=0xff;   //��Fpwmcnt=Fsys=48MHz(����Ƶ)
  PWMPINV=0x00; 	//����ͨ��������Ϊ�������ģʽ
	PWMCNTM=0x01; 	//ͨ��0��4����Ϊ�Զ�����ģʽ
	PWMCNTCLR=0x01; //��ʼ��PWM��ʱ��λͨ��0��4�Ķ�ʱ��
	PWMDTE=0x00; 		//�ر�����ʱ��
	PWMMASKD=0x00; 
	PWMMASKE=0x01; 	//PWM���빦�����ã�Ĭ��״̬�½�ֹͨ��0��4���
	//������������
	PWMP0H=(PWMStepConstant>>8)&0xFF;
	PWMP0L=PWMStepConstant&0xFF;	
	//����ռ�ձ�����
  PWMD0H=0;
	PWMD0L=0;	
	//��ʼ������
	CCDACTargetDuty=0;
	IsPWMLoading=0; 
	IsNeedToUploadPWM=0;
	//����PWM
	PWM_Enable();
	UploadPWMValue();	
	//PWM��ʼ����ϣ�����������Ϊ���ù���
  GPIO_SetMUXMode(PWMIOG,PWMIOx,GPIO_AF_PWMCH0);
	}

void PWM_OutputCtrlHandler(void)	//����PWM�ṹ���ڵ����ý������
	{
	int value;
	float buf;
	//��ǰϵͳδ�������
	if(!IsNeedToUploadPWM)return; //����Ҫ����
	//���μ����ѿ�ʼ�����н������
	else if(IsPWMLoading) 
		{
	  if(PWMLOADEN&0x01)return;//���ؼĴ�����λΪ0����ʾ���سɹ�
	  //���ؽ���
		if(IsNeedToEnableOutput)PWMMASKE&=0xFE;
		else PWMMASKE|=0x01;   //����PWMMASKE�Ĵ����������״̬���ö�Ӧ��ͨ��
		IsNeedToUploadPWM=0;
		IsPWMLoading=0;  //���ڼ���״̬Ϊ���
		}
	//���μ����ѱ�����ʼ�����м��ش���
	else
		{
		//PWMռ�ձȲ�������
		if(CCDACTargetDuty>100)CCDACTargetDuty=100;
		if(CCDACTargetDuty<0)CCDACTargetDuty=0;
		//���üĴ���װ��CC DAC��PWM������ֵ
		buf=CCDACTargetDuty*(float)PWMStepConstant;
		buf/=(float)100;
		value=(int)buf;
		IsNeedToEnableOutput=value?1:0;	//��������ļĴ�������ֵ�ж��Ƿ���Ҫ�������
		PWMD0H=(value>>8)&0xFF;
		PWMD0L=value&0xFF;			
		//PWM�Ĵ�����ֵ��װ�룬Ӧ����ֵ		
		IsPWMLoading=1; //��Ǽ��ع��̽�����
		PWMLOADEN|=0x01; //��ʼ����
		}
	}
