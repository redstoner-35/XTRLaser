#ifndef ADC
#define ADC

#include "stdbool.h"

//�ṹ��
typedef struct
	{
  int Systemp; //ϵͳ�¶�
	int	RandADResult;   //������ΪRNG�������������
	float OutputVoltage; //DCDC�����ѹ(V)
	float BatteryVoltage; //��Ч���ڵ�ص�ѹ(V)
	float RawBattVolt; //ԭʼ�ĵ�ص�ѹ(V)
	float MCUVDD; //��Ƭ����VDD
	bool IsNTCOK; //NTC�Ƿ�OK
	}ADCResultStrDef;

//ADC��׼��ѹ�������׼ͨ������
#define ADCVREF 2.00 //ADCƬ�ڻ�׼LDO�ĵ�ѹ
#define ADC_INTVREFCh 31 //ADC��ͨ��Ƭ�ڴ�϶��׼������ͨ������	
#define ADCBGVREF 1.20 //ADC����ͨ����϶��׼�ĵ�ѹ	
#define ADCWaitChannelSelTime 160 //ADC�ȴ�ͨ��ѡͨ����ʱ	
	
//ADC�Ĵ��������궨��	
#define ADC_StartConv() ADCON0|=0x02 //ADC����ת��
#define ADC_GetIfStillConv()	ADCON0&0x02  //���ADC�Ƿ���Ȼ��ת����Ҫ������
#define ADC_ReadConvResult()	(ADRESL|(ADRESH<<8)) //��ȡADCת���ļĴ������
#define ADC_EnableCmd() ADCON1|=0x80  //ʹ��ADC IP
#define ADC_DisableCmd() ADCON1&=0x7F  //�ر�ADC IP	
#define ADC_SetVREFReg(IsVDD) ADCLDO=(!IsVDD?0xA0:0x00) //���û�׼
#define ADC_IsUsingIVREF() ADCLDO&0x80 //���ADC�Ƿ���ʹ��Ƭ�ڻ�׼	
#define ADC_CheckIfChInvalid(Ch) (Ch<0||(Ch>22&&Ch<ADC_INTVREFCh)) //���ͨ�������Ƿ�Ϸ�	
	
//ADC��������궨��
#define EnableADCAsync() IsNotAllowAsync=0
#define DisableADCAsync() IsNotAllowAsync=1

//ADC�ⲿ�ɼ��Ĳ�������
#define VoutUpperResK 680
#define VoutLowerResK 68 	 //�������ѹ������������
#define VBattUpperResK 470
#define VBattLowerResK 100 //��ؼ���ѹ������������
#define NTCUpperResValueK 470 //NTC���������������ֵ

//�ⲿADC��������
extern ADCResultStrDef Data;
extern bit IsNotAllowAsync; //�Ƿ������첽ת��

//�ⲿ����
void ADC_Init(void);
void ADC_DeInit(void);
void SystemTelemHandler(void);

#endif