#include "LEDMgmt.h"
#include "delay.h"
#include "ADCCfg.h"
#include "BattDisplay.h"
#include "ModeControl.h"
#include "SelfTest.h"
#include "LowVoltProt.h"
#include "OutputChannel.h"

//�ڲ�����
static xdata unsigned char ErrDisplayIndex; //������ʾ��ʱ
static xdata unsigned char ShortDetectTIM=0; //��·����ʱ��
static xdata unsigned char ShortBlankTIM; //��·blank��ʱ��
static code FaultCodeDef NonCriticalFault[]={ //�������Ĵ������
	Fault_DCDCOpen,
  Fault_DCDCShort, //��·�Ͷ�·�������󱨣���������
  Fault_InputOVP
	};
	
//�ⲿȫ�ֲο�
xdata FaultCodeDef ErrCode; //�������	

//��ѯ�����Ƿ�����
bit IsErrorFatal(void)	
	{
	unsigned char i;
	for(i=0;i<sizeof(NonCriticalFault);i++)
		if(NonCriticalFault[i]==ErrCode)return 0;
	//Ѱ����Ŀǰ���еĴ����뷢������������
	return 1;
	}

//�������
void ReportError(FaultCodeDef Code)
	{
	ErrCode=Code;
	if(CurrentMode->ModeIdx==Mode_Fault)return;
	SwitchToGear(Mode_Fault);  //ָʾ���Ϸ���
	}

//��������
void ClearError(void)
	{
	ErrCode=Fault_None;
	SwitchToGear(Mode_OFF);
	}

//����ID��ʾ��ʱ����	
void DisplayErrorTIMHandler(void)	
	{
	//û�д���������λ��ʱ��
	if(ErrCode==Fault_None)ErrDisplayIndex=0;
	else //�������󣬿�ʼ��ʱ
		{
		ErrDisplayIndex++;
    if(ErrDisplayIndex>=(15+(6*(int)ErrCode)))ErrDisplayIndex=0; //���޵��ˣ���ʼ��ת
		}
	}

//���ִ���ʱ��ʾDCDC�Ĵ���ID
void DisplayErrorIDHandler(void)
	{
	int buf;
	//�ȵ���ʾ����̽�����
  if(ErrDisplayIndex<5)
		{
		if(ErrDisplayIndex<3)LEDMode=(LEDStateDef)(ErrDisplayIndex+1);	
		else LEDMode=LED_OFF;
		}
	//��˸ָ��������ʾErr ID
	else if(ErrDisplayIndex<(5+(6*(int)ErrCode)))
		{
		buf=(ErrDisplayIndex-5)/3; 
		if(!(buf%2))LEDMode=LED_Red;
		else LEDMode=LED_OFF;  //���մ���ID��˸ָ������
		}
  else LEDMode=LED_OFF; //LEDϨ��
	}
//�ڲ����������ϼ�����
static char ErrTIMCounter(char buf,char Count)
	{
	//�ۼӼ�����
	return buf<8?buf+Count:8;
	}

//������ϼ��
void OutputFaultDetect(void)
	{
	char buf,OErrID;
	//������ϼ��
	if(!GetIfOutputEnabled())ShortBlankTIM=0; //DCDC�ر�
	else if(ShortBlankTIM<FaultBlankingInterval)ShortBlankTIM++; //ʱ��δ����������
	else  //��ʼ���
		{		
		buf=ShortDetectTIM&0x0F; //ȡ����ʱ��ֵ					
		//�����ѹ����
		if(Data.BatteryVoltage>4.35)ReportError(Fault_InputOVP);		
		//��·���	
		if(Data.OutputVoltage<Data.RawBattVolt-1.0) //�����·
			{
			buf=ErrTIMCounter(buf,2); //��ʱ���ۼ�
			OErrID=0;
			}
		//DCDC����ʧ�ܼ��
		else if(Data.OutputVoltage<11.1)
			{
			buf=ErrTIMCounter(buf,1); //��ʱ���ۼ�
			OErrID=2;
			}			
		//�����·���
		else if(Data.OutputVoltage>17.0) 
			{
			buf=ErrTIMCounter(buf,1); //��ʱ���ۼ�
			OErrID=1;
			}
		else buf=buf>0?buf-1:0; //û�з����������������
		//���ж�ʱ����ֵ�Ļ�д
		ShortDetectTIM=buf|(OErrID<<4);
		//״̬���
		if(buf<8)return; //û�й���,����ִ��
		switch((ShortDetectTIM>>4)&0x0F)	
			{
			case 1:ReportError(Fault_DCDCOpen);break;
			case 2:ReportError(Fault_DCDCFailedToStart);break;
			default:ReportError(Fault_DCDCShort);
			}
		}
	}