/************************************************************************************/
/** \file PinDefs.h
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ�Ϊ�ײ��Ӳ�������ļ����������������������������Ӧ��Ӳ��PIN��ģ
������ADCͨ����ӳ���ϵ��

**	History: Initial Release
**	
/************************************************************************************/
#ifndef PINDEFS
#define PINDEFS
/************************************************************************************/
/* Include files */
/************************************************************************************/
#include "GPIOCfg.h"

/************************************************************************************
������ϵͳ�ĳ���GPIO���ţ����ڿ����ⲿ�����л�����
************************************************************************************/
#define DCDCENIOP GPIO_PORT_1
#define DCDCENIOG 1
#define DCDCENIOx GPIO_PIN_3 //LD��DCDC��Դʹ������(P1.3)

/************************************************************************************
������ϵͳ��PWM������ţ����ڶ������PWM���Ʒ����ٶȣ�DCDC�����ѹ�ȵ�
************************************************************************************/
#define PWMIOP GPIO_PORT_2
#define PWMIOG 2
#define PWMIOx GPIO_PIN_3  //������ƺ�����׼DAC��������ţ�P2.3��

/************************************************************************************
������ϵͳ��ģ���������ţ������ص�ѹ������
************************************************************************************/
#define NTCInputIOG 2
#define NTCInputIOx GPIO_PIN_2 
#define NTCInputAIN 8						//NTC����(P2.2,AN8)

#define VOUTFBIOG 0
#define VOUTFBIOx GPIO_PIN_0
#define VOUTFBAIN 0						//�����ѹ��������(P0.0,AN0)

#define VBATInputIOG 3
#define VBATInputIOx GPIO_PIN_1
#define VBATInputAIN 13					//��ص�ѹ�������(P3.1,AN13)

/************************************************************************************
������ϵͳ�İ���ģ���GPIO���ţ������������𰴼�С�岿��(����ָʾ�ƺͰ�������) 
************************************************************************************/
#define SideKeyGPIOP GPIO_PORT_2
#define SideKeyGPIOG 2
#define SideKeyGPIOx GPIO_PIN_6 	//�ఴ����(P2.6)


#define RedLEDIOP GPIO_PORT_3
#define RedLEDIOG 3
#define RedLEDIOx GPIO_PIN_0		//��ɫָʾ��(P3.0)	


#define GreenLEDIOP GPIO_PORT_0
#define GreenLEDIOG 0
#define GreenLEDIOx GPIO_PIN_1		//��ɫָʾ��(P0.1)

#endif /* PINDEFS */
