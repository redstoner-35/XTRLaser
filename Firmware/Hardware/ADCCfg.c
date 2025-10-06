/****************************************************************************/
/** \file ADCCfg.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition
/** \Description ����ļ���������ϵͳ��ADC���ϵͳ�ĸ���ģ����ң�����񣬰����첽
������ת��ADC�����ʵ��
**
**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "ADCCfg.h"
#include "GPIO.h"
#include "ADCASync.h"
#include "delay.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
bit IsNotAllowAsync;	 //�Ƿ�����ADC�����������첽ģʽ
ADCResultStrDef Data;	 //ADC������

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')

ע�⣡���º���ΪADC�첽ת������ʵ�ֹ���������ڲ��������Լ�������ڲ�ȫ�ֱ�
�����벻Ҫ�����޸ĺ������ݣ������ڳ��˱��ļ��ڵ������κεط����ã�����
�ᵼ��ADC���湤���쳣��	
****************************************************************************/
static xdata ADCConvertTemp ADCTemp;
static ADCAsyncStateDef ADCState;	
static xdata char ADCConvertQueue[ADCConvertQueueDepth];	

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/
//��ADC�����ύ�µ�ת������	
static void ADC_SubmitMisson(char Ch)	
	{
	unsigned char i=ADCWaitChannelSelTime;
	//��鴫���ͨ�������Ƿ�Ϸ�
	if(ADCTemp.IsMissionProcessing)return;
	if(ADC_CheckIfChInvalid(Ch))return; 
	//���г�ʼ��
	ADCTemp.avgbuf=0;
	ADCTemp.Count=0;
	ADCTemp.Ch=Ch;
	ADCTemp.IsMissionProcessing=true;
	//����ADCͨ��		
	if(Ch&0x10)ADCON0|=0x80;
	else ADCON0&=0x7F; //����ADCHS[4]
	ADCON1&=0xF0;
	ADCON1|=(Ch&0x0F); //����ADCHS[3:0]					
	//����ת��
	while(--i);  			//��ʱ�ȴ�ͨ��ѡͨ��ʼ����
	ADC_StartConv();
	}	

//��ȡ����
static int ADC_ReadBackResult(int *Result,char *Queue)	
	{
	//ADCδ��ɱ���ת��
	if(ADC_GetIfStillConv())return 0; 
	//��ȡ���
	ADCTemp.Count++; //��ֵ+1
	ADCTemp.avgbuf+=(long)ADC_ReadConvResult(); //��AD�Ĵ�����ȡ���������ƽ���ۼ�
	if(ADCTemp.Count<ADCAverageCount) 
		{
		ADC_StartConv();
		return 0;//ƽ������δ������������ADC������һ�ֵĴ���
		}
	//���ת�������ؽ����׼�������ύ�µ�����
	ADCTemp.avgbuf/=(long)ADCAverageCount;
	*Result=(int)ADCTemp.avgbuf; //���ؽ��
	*Queue=ADCTemp.Ch; //����ת���Ķ���
	ADCTemp.IsMissionProcessing=false; //�����Ѵ������
  return 1;	
	}

//ADC���õ�ѹ�ο�	
static void ADC_SetVREF(bit IsUsingVDD)
	{
	ADC_DisableCmd(); //ת��ADC��׼��Ҫ��ʱ�ر�ADC	
	_nop_();
	ADC_SetVREFReg(IsUsingVDD); //����оƬ�ڲ���׼
	_nop_();
	ADC_EnableCmd(); //��׼�л���ϣ���������
	}
	
//ת����Ϻ�д�������
int	CalcNTCTemp(bool *IsNTCOK,unsigned long NTCRes); //��������		

static void ADC_WriteOutputBuf(int ADCResult,char Ch)
	{
	float Buf,Rt,Vadc;
	unsigned long NTCRES;
	extern bit IsEnable2SMode;
	//����ADC
	Rt=ADC_IsUsingIVREF()?ADCVREF:Data.MCUVDD; //���ݻ�׼���õõ�AD��ǰ�Ļ�׼��ѹ
	Vadc=(float)ADCResult*(Rt/(float)4096);//��ADֵת��Ϊԭʼ��ѹ
	//״̬��
  switch(Ch)
		{
		//����ο���ѹ
		case ADC_INTVREFCh:
			Data.MCUVDD=ADCBGVREF*(float)4096/(float)ADCResult; //�����MCUVDD(VREF)
		  break; 
		//�����ص�ѹ
		case VBATInputAIN:
			//����ԭʼ��ص�ѹ
			#define VBatTotalResistor (VBattLowerResK+VBattUpperResK)
			Buf=(float)VBattLowerResK/(float)VBatTotalResistor;//�������ѹ�����ϵ��
			Data.RawBattVolt=Vadc/Buf; //���ݷ�ѹϵ�����Ƴ���ص�ѹ
			#undef VBatTotalResistor
		
		  //�������2Sģʽ��2�ڵ�ص��ܵ�ѹת��Ϊ���ڵ�صĵ�ѹ������ԭʼ��ص�ѹ�͵�Ч��ѹһ��
		  if(!IsEnable2SMode)Data.BatteryVoltage=Data.RawBattVolt;
			else Data.BatteryVoltage=Data.RawBattVolt/(float)2; 
		  break;
	  //���������ѹ
		case VOUTFBAIN:		
			#define VoutTotalResistor (VoutLowerResK+VoutUpperResK)
			Buf=(float)VoutLowerResK/(float)VoutTotalResistor;//�������ѹ�����ϵ��
			Data.OutputVoltage=Vadc/Buf; //���ݷ�ѹϵ�����Ƴ�DCDC�����ѹ
		  #undef VoutTotalResistor
		  break;
    //�����¶�
		case NTCInputAIN:
			Rt=((float)NTCUpperResValueK*Vadc)/(Data.MCUVDD-Vadc);//�õ�NTC+��Ƭ��IO��ͨ����Ĵ�������ֵ
			Rt*=1000; //����ֵ��KŷתΪ��
			NTCRES=(unsigned long)Rt; //ȡ��
			Data.Systemp=CalcNTCTemp(&Data.IsNTCOK,NTCRES); //�����¶�
			break;
		}
  }

//ADC�첽�������������
static void ADCEngineHandler(void)
	{
	int result;
	char Ch,i;
	//ת��ѭ��
  do
		{
		if(ADCState==ADC_ConvertComplete)ADCState=ADC_SubmitQueue; //���һ��ת����������¿�ʼ
		switch(ADCState)
			{		
			//��ʼ�ύת������
			case ADC_SubmitQueue: 
				ADC_SetVREF(1); //ÿ���ύ����֮ǰ�����û�׼ʹ��MCUVDD��ת���¶Ⱥ�MCUVDD��ѹ
				for(i=0;i<ADCConvertQueueDepth;i++)ADCConvertQueue[i]=ADCChQueue[i]; //��ת��������������ݸ��ƹ�ȥ
			  ADCState=ADC_SubmitChFromQueue;
			  break;
		  //��ADCת���߳��ύ����
			case ADC_SubmitChFromQueue: 
			  i=0;
			  while(i<ADCConvertQueueDepth)
					{
					if(!ADC_CheckIfChInvalid(ADCConvertQueue[i]))break; //�ҵ�������δ��ɵĺϷ�ת����Ŀ
					i++;
					}
				//��ת����Ŀδ���
				if(i<ADCConvertQueueDepth)	
					{
					Ch=ADCConvertQueue[i]; //���Ŀ���ͨ��ֵ
          if(Ch==VBATInputAIN||Ch==VOUTFBAIN)ADC_SetVREF(0); //��غ������ѹת��ʹ���ڲ�����ͨ��
					ADC_SubmitMisson(Ch); //�ύ��Ŀ
					ADCState=ADC_WaitMissionDone;
					}
				//����ת������ɣ���ת����ɽ׶�
  			else ADCState=ADC_ConvertComplete;		
			  break;	
			//�ύ�߳������ȴ������������
      case ADC_WaitMissionDone:
          if(!ADC_ReadBackResult(&result,&Ch))break; //���Զ�ȡ�����ת��δ��������
			    Data.RandADResult=result;     //�����ȡ����resultֵд��ȥ
			    ADC_WriteOutputBuf(result,Ch);
			    for(i=0;i<ADCConvertQueueDepth;i++)if(ADCConvertQueue[i]==Ch)ADCConvertQueue[i]=-2; //����ǰ�Ѿ����ת��������ͨ������Ϊ-2���ת�����
			    ADCState=ADC_SubmitChFromQueue; //���»ص��ύ����Ľ׶�
			    break;
			//�������������
			case ADC_ConvertComplete:break;
			//�����κηǷ�״̬��ת����ʼ�׶�
			default:ADCState=ADC_SubmitQueue;
			}
		}
	while(IsNotAllowAsync&&ADCState!=ADC_ConvertComplete);
	}	

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
	
ע�⣺���º���ΪADC�첽ת�������Լ�ADC�ĳ�ʼ���ͳ��ܲ��������������ȡ�ⲿͨ
���ĵ�ѹ����������ⲿ�������á��������ڳ�ʼ���׶κ��������ڵ���������
��ĺ�����ADC���г�ʼ���ͳ��ܲ������Լ����������ADC�����첽������
****************************************************************************/	

//�������ݻ�ȡ	
void SystemTelemHandler(void)
	{
  //����ADC�첽����
	ADCEngineHandler();
	}	
	
//��λADC�첽����
static void ResetADCAsyncEngine(void)	
	{
	unsigned char i;	
	for(i=0;i<ADCConvertQueueDepth;i++)ADCConvertQueue[i]=-2;	
	ADCState=ADC_SubmitQueue;
	ADCTemp.avgbuf=0;
	ADCTemp.Count=0;
	ADCTemp.Ch=0;
	ADCTemp.IsMissionProcessing=false;
	IsNotAllowAsync=1; //��ʼ��ʱ��ֹ�첽����	
	}

//�ر�ADC
void ADC_DeInit(void)
	{
	GPIOCfgDef ADCDeInitCfg;	
	//���üĴ����ر�ADC
	ADCON1=0x00; //�ر�ADC
	ADCLDO=0x00; //�ر�Ƭ�ڻ�׼
	
	//��ն��в���λ�첽����
  ResetADCAsyncEngine();
	//����Ҫ���õ�ADC����GPIO����Ϊ��ͨGPIOģʽ
	GPIO_SetMUXMode(VOUTFBIOG,VOUTFBIOx,GPIO_AF_GPIO);
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_GPIO);
	//����Ҫ���õ�ADC��������Ϊ�������
	ADCDeInitCfg.Mode=GPIO_Out_PP;
  ADCDeInitCfg.Slew=GPIO_Slow_Slew;		
	ADCDeInitCfg.DRVCurrent=GPIO_Low_Current; //����Ϊ�͵����������
	
  GPIO_ConfigGPIOMode(VOUTFBIOG,GPIOMask(VOUTFBIOG),&ADCDeInitCfg); 
	GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCDeInitCfg); 
	//����Ҫ���õ�ADC����GPIOȫ�����0
  GPIO_WriteBit(VBATInputIOG,VBATInputIOx,0);
	GPIO_WriteBit(VOUTFBIOG,VOUTFBIOx,0);
	}

//ADC��ʼ��
void ADC_Init(void)
	{
	GPIOCfgDef ADCInitCfg;
	//��ʼ��GPIO
	ADCInitCfg.Mode=GPIO_Input_Floating;
  ADCInitCfg.Slew=GPIO_Slow_Slew;		
	ADCInitCfg.DRVCurrent=GPIO_Low_Current; //����Ϊ��������	

  GPIO_ConfigGPIOMode(VOUTFBIOG,GPIOMask(VOUTFBIOG),&ADCInitCfg); 
	GPIO_ConfigGPIOMode(VBATInputIOG,GPIOMask(VBATInputIOx),&ADCInitCfg); 
	GPIO_ConfigGPIOMode(NTCInputIOG,GPIOMask(NTCInputIOx),&ADCInitCfg); 	//����Ӧ��IO����Ϊָ����ģʽ
	
  GPIO_SetMUXMode(NTCInputIOG,NTCInputIOx,GPIO_AF_Analog);
	GPIO_SetMUXMode(VOUTFBIOG,VOUTFBIOx,GPIO_AF_Analog);
	GPIO_SetMUXMode(VBATInputIOG,VBATInputIOx,GPIO_AF_Analog); //��GPIO��������Ϊģ������	
	//����ADC
	ADCON0=0x40; //AN31=�ڲ�1.2V��׼������Ҷ���
	ADCON1=0x60; //Fadc=Fsys/128=375KHz
	ADCON2=0x00; //�ر�ADCӲ���������ܣ�ʹ�������������ADC
	ADCMPC=0x00; //�ر�ADC�Ƚ�������ɲ������
	ADDLYL=0x00; //��ADCӲ������������ʱ����Ϊ0
	ADCMPH=0x0F;
	ADCMPL=0xFF; //ADC�Ƚ���Ĭ��ֵ����Ϊ0x0FFF
  ADCLDO=0xA0; //ʹ��оƬ����ADC��׼�����2.0V
	
	//��ʼ���첽ADC����
	ResetADCAsyncEngine();
	//ADC������ϣ�ʹ��ADCģ��
	ADC_EnableCmd(); 
	}	
