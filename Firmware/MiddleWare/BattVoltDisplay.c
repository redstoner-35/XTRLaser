#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "OutputChannel.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "FastOp.h"
#include "ModeControl.h"
#include "SelfTest.h"
#include "SysConfig.h"

//���״̬flag
bit IsBatteryAlert; //��ص�ѹ���ھ���ֵ	
bit IsBatteryFault; //��ص�ѹ���ڱ���ֵ		

//�ڲ�����
static xdata unsigned char BattShowTimer; //��ص�����ʾ��ʱ
static xdata AverageCalcDef BattVolt;	
static xdata unsigned char LowVoltStrobeTIM;
static xdata int VbattSample; //ȡ���ĵ�ص�ѹ
static xdata unsigned char Show2SModeTIM;   //��ʾ2Sģʽ��ʱ����
static bit IsReportingTemperature=0; //�����¶�
static bit IsWaitingKeyEventToDeassert=0; //�ڲ���־λ���ȴ������ʾ��������ʹ��״̬����Ӧ

//�ⲿȫ�ֱ���
BattStatusDef BattState; //��ص������λ
xdata int CellVoltage; //��Ч���ڵ�ص�ѹ
xdata unsigned char CommonSysFSMTIM;  //��ѹ��ʾ��ʱ��
xdata BattVshowFSMDef VshowFSMState; //��ص�ѹ��ʾ����ļ�ʱ����״̬��ת��


//�ڲ�ʹ�õ��ȵ���ʾ��
static code LEDStateDef VShowIndexCode[]=
	{
	LED_Red,
	LED_Amber,
	LED_Green,  //���������Ǻ����
	LED_Amber,
	LED_Red  //�߾���ģʽ�Ƿ��������̺��
	};

//׼����ѹ��ʾ״̬����ģ��
static void VShowFSMPrepare(void)	
	{
	VshowFSMState=BattVdis_PrepareDis;	
	if(CurrentMode->ModeIdx!=Mode_OFF)
		{
		if(LEDMode!=LED_OFF)CommonSysFSMTIM=8; //ָʾ�Ƶ���״̬��ѯ������Ϩ��LED��һ��
		LEDMode=LED_OFF;
		}	
	}

//����ϵͳ�¶���ʾ
void TriggerTShowDisplay(void)
	{
	if(!Data.IsNTCOK||VshowFSMState!=BattVdis_Waiting)return; //�ǵȴ���ʾ״̬��ֹ����
	VShowFSMPrepare();
	//�����¶�ȡ��
	IsReportingTemperature=1;
	if(IsNegative8(Data.Systemp))VbattSample=(int)Data.Systemp*-10;
	else VbattSample=(int)Data.Systemp*10;
	}

//������ص�ѹ��ʾ
void TriggerVshowDisplay(void)	
	{
	if(VshowFSMState!=BattVdis_Waiting)return; //�ǵȴ���ʾ״̬��ֹ����
	VShowFSMPrepare();
	//���е�ѹȡ��(����ΪLSB=0.01V)
	VbattSample=(int)(Data.RawBattVolt*100); 		
	}		

//���ɵ͵�����ʾ����
bit LowPowerStrobe(void)
	{
	//��������������SOSģʽ��Ϊ�˱������SOS��ʾ����������ʱ
	if(BattState!=Battery_VeryLow||CurrentMode->ModeIdx==Mode_SOS)LowVoltStrobeTIM=0;
	//�����쳣��ʼ��ʱ
	else if(!LowVoltStrobeTIM)LowVoltStrobeTIM=1; //������ʱ��
	else if(LowVoltStrobeTIM>((LowVoltStrobeGap*8)-4))return 1; //������˸��ǵ���Ϊ0
	//�����������0
	return 0;
	}
	
//������ʾ����2Sģʽ������������2�ε�����
void ShowEntered2SModeProc(void)
	{
	if(Show2SModeTIM&0x01)MakeFastStrobe(LED_Green);
	//�����ʾ����֮���������ʱ
	if(!BattShowTimer&&Show2SModeTIM)Show2SModeTIM--;
	}	
	
//��������2Sģʽ�Ŀ���������ʾ	
void LoadSleepTimer(void);         //�������غ���

void Trigger2SModeEnterInfo(void)
	{
	//�������ظ�����
	if(Show2SModeTIM)return;
	//���ض�ʱ��ȷ����ʾ�����Ŵ���
  LoadSleepTimer(); 
	Show2SModeTIM=4;	//���ʱ��=4������ʱ��ʼ
	}	
	
//����LED�ఴ������˸ָʾ��ص�ѹ�Ĵ���
static void VshowGenerateSideStrobe(LEDStateDef Color,BattVshowFSMDef NextStep)
	{
	//������Ǹ���������λ=1��ͨ������һ�α�ʾ��0
	if(IsNegative8(CommonSysFSMTIM))
		{
		MakeFastStrobe(Color);
		CommonSysFSMTIM=0; 
		}
	//����ָʾ
	LEDMode=(CommonSysFSMTIM%4)&0x7E?Color:LED_OFF; //�����ɫ��˸ָʾ��Ӧλ�ĵ�ѹ
	//��ʾ����
	if(!CommonSysFSMTIM) 
		{
		LEDMode=LED_OFF;
		CommonSysFSMTIM=10;
		VshowFSMState=NextStep; //�ȴ�һ��
		}
	}
//��ѹ��ʾ״̬�����ݶ�Ӧ�ĵ�ѹλ���������˸��ʱ��������ֵ
static void VshowFSMGenTIMValue(int Vsample,BattVshowFSMDef NextStep)
	{
	if(!CommonSysFSMTIM)	//ʱ�䵽��������
		{	
		if(!Vsample)CommonSysFSMTIM=0x80; //0x80=˲����һ��
		else CommonSysFSMTIM=(4*Vsample)-1; //������ʾ��ʱ��
		VshowFSMState=NextStep; //ִ����һ����ʾ
		}
	}
	
//���ݵ��״̬������LEDָʾ��ص���
static void SetPowerLEDBasedOnVbatt(void)	
	{
	switch(BattState)
		{
		 case Battery_Plenty:LEDMode=LED_Green;break; //��ص���������ɫ����
		 case Battery_Mid:LEDMode=LED_Amber;break; //��ص����еȻ�ɫ����
		 case Battery_Low:LEDMode=LED_Red;break;//��ص�������
		 case Battery_VeryLow:LEDMode=LED_RedBlink;break; //��ص������ز����ɫ����
		}
	}

//��ز�����ʾ��ѹ
LEDStateDef VshowEnter_ShowIndex(void)
	{
	char Index;
	if(CommonSysFSMTIM>9)
		{
		Index=((CommonSysFSMTIM-8)>>1)-1;
		if(IsReportingTemperature&&!(Data.Systemp&0x80))Index+=2;//�¶Ȳ���ʱ�¶�Ϊ������ʹ�ó�����ʾģʽ
		if(!IsReportingTemperature&&VbattSample>999)Index+=2; //��ѹ����ʱ�����ѹ����10V,ʹ�ó�����ʾģʽ
		return VShowIndexCode[Index];
		}
	return LED_OFF; //�������˸֮��(����Ǹ߾�����ʾģʽ��Ϊ�̺��)�ȴ�
	}

//�����ϸ��ѹ��ʾ��״̬������
static void BatVshowFSM(void)
	{
	//������ʾ״̬��
	switch(VshowFSMState)
		{
		case BattVdis_PrepareDis: //׼����ʾ
			if(CommonSysFSMTIM)break;
	    CommonSysFSMTIM=15; //�ӳ�1.75��
			VshowFSMState=BattVdis_DelayBeforeDisplay; //��ʾͷ��
		  break;
		//�ӳٲ���ʾ��ͷ
		case BattVdis_DelayBeforeDisplay: 
			//ͷ����ʾ������ʼ��ʽ��ʾ��ѹ
			LEDMode=VshowEnter_ShowIndex();
		  if(CommonSysFSMTIM)break;
			//��ص�ѹΪ����10V�����������������봦����С�����һλ�Ľ��
		  if(VbattSample>999)
			   {
				 /********************************************************
				 �������������ԭ���ǵ�ص�ѹ�ᱻ����Ϊ������1LSB=0.01V������
				 ��ص�ѹΪ12.59V����֮��ͻ���1259����ô��ʱ������Ҫ��С��
				 �����λ�������������жϣ��õ�һλС���Ľ�����������������
				 ��λʵ���ϵ��ڸ���ĵ�ص�ѹ�е�С�������λ���������ֻ��Ҫ
				 ͨ����10�������Ϳ���ȡ��С���������2λ��Ȼ������������4
				 ����н�λ����С�����һλ+1��ʵ�������������ˡ�������������
				 ������10֮��ͻ��Զ�ȥ��С�������λ��ֵ����1λС����
				 *********************************************************/					 
				 if((VbattSample%10)>4)VbattSample+=10;
				 VbattSample/=10;
				 }
			//���ü�ʱ����ʾ��һ���ѹ
			VshowFSMGenTIMValue(VbattSample/100,BattVdis_Show10V);
		  break;
    //��ʾʮλ
		case BattVdis_Show10V:
			VshowGenerateSideStrobe(LED_Red,BattVdis_Gap10to1V); //���ô��������ɺ�ɫ�ಿ��˸
		  break;
		//ʮλ�͸�λ֮��ļ��
		case BattVdis_Gap10to1V:
			VbattSample%=100;
			VshowFSMGenTIMValue(VbattSample/10,BattVdis_Show1V); //���ü�ʱ����ʼ��ʾ��һ��	
			break;	
		//��ʾ��λ
		case BattVdis_Show1V:
		  VshowGenerateSideStrobe(LED_Amber,BattVdis_Gap1to0_1V); //���ô��������ɻ�ɫ�ಿ��˸
		  break;
		//��λ��ʮ��λ֮��ļ��		
		case BattVdis_Gap1to0_1V:	
			//�¶Ȳ�������֮��ֱ�ӽ���ȴ��׶�
			if(IsReportingTemperature)
				{
				CommonSysFSMTIM=10;  
				VshowFSMState=BattVdis_WaitShowTempState; 
				}
			else VshowFSMGenTIMValue(VbattSample%10,BattVdis_Show0_1V);
			break;
		//��ʾС�����һλ(0.1V)
		case BattVdis_Show0_1V:
		  VshowGenerateSideStrobe(LED_Green,BattVdis_WaitShowChargeLvl); //���ô�����������ɫ�ಿ��˸
			break;
		//�ȴ�һ��ʱ�����ʾ��ǰ�¶�ˮƽ
		case BattVdis_WaitShowTempState: 
			if(CommonSysFSMTIM)break;
			VshowFSMState=BattVdis_ShowTempState;
		  CommonSysFSMTIM=31;
			break;
	 
		//�ȴ���ǰ�¶�ˮƽ��ʾ����
		case BattVdis_ShowTempState:
			if(CommonSysFSMTIM<25&&CommonSysFSMTIM&0xF8)
				{
				if(Data.Systemp<45)LEDMode=LED_Green;
				else if(Data.Systemp<55)LEDMode=LED_Amber;
				else LEDMode=LED_Red;
				}
			//��ʾ������LEDϨ��һ��ʱ��
			else LEDMode=LED_OFF;
			//�ȴ��¶�״̬��ʾʱ�䵽������֮����ת���ȴ��û��ɿ������Ĵ���
			if(!CommonSysFSMTIM)VshowFSMState=BattVdis_ShowChargeLvl;
			break;		  
		//�ȴ�һ��ʱ�����ʾ��ǰ����
		case BattVdis_WaitShowChargeLvl:
			if(CommonSysFSMTIM)break;
			//1LMģʽ�Լ��ػ��µ���ָʾ�Ʋ���פ������������Ҫ���������ʱ��LED����
			if(CurrentMode->ModeIdx==Mode_OFF)BattShowTimer=18; 
			VshowFSMState=BattVdis_ShowChargeLvl; //�ȴ�������ʾ״̬����
      break;
	  //�ȴ����������ʾ����
		case BattVdis_ShowChargeLvl:
			IsReportingTemperature=0;  									//clear���¶���ʾ��־λ
			VbattSample=0;                              //��ѹ��ʾÿ�ν�����clear����ѹ��������
		  if(BattShowTimer)SetPowerLEDBasedOnVbatt();//��ʾ����
			else if(!getSideKeyNClickAndHoldEvent())VshowFSMState=BattVdis_Waiting; //�û���Ȼ���°������ȴ��û��ɿ�,�ɿ���ص��ȴ��׶�
      break;
		}
	}
//��ص���״̬��
static void BatteryStateFSM(void)
	{
	int thres;
	//������ֵ
  if(CurrentMode->ModeIdx!=Mode_Turbo)thres=3650;
  else thres=3550;
	//״̬������	
	switch(BattState) 
		 {
		 //��ص�������
		 case Battery_Plenty: 
				if(CellVoltage<thres)BattState=Battery_Mid; //��ص�ѹС��ָ����ֵ���ص������е�״̬
			  break;
		 //��ص�����Ϊ����
		 case Battery_Mid:
			  if(CellVoltage>(thres+250))BattState=Battery_Plenty; //��ص�ѹ������ֵ���ص�����״̬
				if(CellVoltage<(thres-200))BattState=Battery_Low; //��ص�ѹ����3.3���л��������͵�״̬
				break;
		 //��ص�������
		 case Battery_Low:
		    if(CellVoltage>(thres+50))BattState=Battery_Mid; //��ص�ѹ����3.6���л��������еȵ�״̬
			  if(CellVoltage<2950)BattState=Battery_VeryLow; //��ص�ѹ����3.0���������ز���
		    break;
		 //��ص������ز���
		 case Battery_VeryLow:
			  if(CellVoltage>3300)BattState=Battery_Low; //��ص�ѹ������3.3����ת����������׶�
		    break;
		 }
	}

//��λ��ص�ѹ��⻺��
static void ResetBattAvg(void)	
	{
	BattVolt.Min=32766;
	BattVolt.Max=-32766; //��λ�����С������
	BattVolt.Count=0;
  BattVolt.AvgBuf=0; //���ƽ���������ͻ���
	}
	
//������ʱ��ʾ��ص�ѹ
void DisplayVBattAtStart(bit IsPOR)
	{
	unsigned char i=10;
	//��ʼ��ƽ��ֵ����,��λ��־λ
	ResetBattAvg();
	//���е�ص�ѹ�Ϸ��Լ�鲢�Զ����и��´���
	#ifndef USING_LD_NURM11T	
	if(Data.RawBattVolt>8.60)ReportError(Fault_InputOVP);
  else if(Data.RawBattVolt>4.35&&!IsEnable2SMode)		
		{
		//��ǰϵͳΪ��ﮣ���⵽�ߵ�ѹ�Զ�����2Sģʽ
		IsEnable2SMode=1;
		SaveSysConfig(0);
		}	
	#else
	//���LDֻ�ܵ�ﮣ�����̼�����Ϊ���ģʽ���������ѹ����4.35Vʱ����
	if(Data.RawBattVolt>4.35)ReportError(Fault_InputOVP);
	#endif
  //��λ��ص�ѹ״̬�͵����ʾ״̬��
  VshowFSMState=BattVdis_Waiting;		
	do
		{
		SystemTelemHandler();
		CellVoltage=(int)(Data.BatteryVoltage*1000); //��ȡ�����µ�ص�ѹ
		BatteryStateFSM(); //����ѭ��ִ��״̬�����µ����յĵ��״̬
		}
	while(--i);
	//������ص�����ʾ(���޴���������)
	if(!IsPOR||CurrentMode->ModeIdx!=Mode_OFF)return;
	if(IsEnable2SMode)	
		{
		//2Sģʽ�����ָʾ���Ի�ɫ��������ָʾ����2Sģʽ
		MakeFastStrobe(LED_Amber);
		delay_ms(200);
		MakeFastStrobe(LED_Amber);
		//������˸���ӳٰ����ټ���������������
		for(i=48;i;i--)delay_ms(10); 
		}
	//������ص�����������ʹ�ܰ�������
	IsWaitingKeyEventToDeassert=1;
	BattShowTimer=18;
	}
	
//�ڲ��������������ڽ����ϵ��״ε��������ڼ��ֹ����״̬����Ӧ
bit IsKeyFSMCanEnable(void)
	{
	//��ǰ��������λclear����������������Ӧ
	if(!IsWaitingKeyEventToDeassert)return 1;
	//��ǰ��ص�����ʾ��Ȼ�ڼ�ʱ���ȴ�
	if(BattShowTimer||Show2SModeTIM)return 0;
	else
		{
		//�����Ѿ��ſ���û���¼�������Ӧ����λbit
		if(!IsKeyEventOccurred())IsWaitingKeyEventToDeassert=0;
		//�������������¼�
	  ClearShortPressEvent(); 
		getSideKeyLongPressEvent();
		}
	//�����������ֹ������Ӧ
	return 0;
	}

//��ص�����ʾ��ʱ�Ĵ���
void BattDisplayTIM(void)
	{
	long buf;
	//����ƽ��ģ�����
	if(BattVolt.Count<VBattAvgCount)		
		{
		buf=(long)(Data.BatteryVoltage*1000);
		BattVolt.Count++;
		BattVolt.AvgBuf+=buf;
		if(BattVolt.Min>buf)BattVolt.Min=buf;
		if(BattVolt.Max<buf)BattVolt.Max=buf; //��ֵ��ȡ
		}
	else //ƽ�������������µ�ѹ
		{
		BattVolt.AvgBuf-=(long)BattVolt.Min+(long)BattVolt.Max; //ȥ��������
		BattVolt.AvgBuf/=(long)(BattVolt.Count-2); //��ƽ��ֵ
		CellVoltage=(int)BattVolt.AvgBuf;	//�õ����յĵ�ص�ѹ(��λmV)
		ResetBattAvg(); //��λ����
		}
	//�͵�ѹ��ʾ��˸��ʱ��
	if(LowVoltStrobeTIM==LowVoltStrobeGap*8)LowVoltStrobeTIM=1;//ʱ�䵽�����ֵ���¼�ʱ
	else if(LowVoltStrobeTIM)LowVoltStrobeTIM++;
	//��ص�ѹ��ʾ�ļ�ʱ������	
	if(CommonSysFSMTIM)CommonSysFSMTIM--;
	//�����ʾ��ʱ��
	if(BattShowTimer)BattShowTimer--;
	}

//��ز���������ָʾ�ƿ���
void BatteryTelemHandler(void)
	{
	int AlertThr;
	extern bit IsDisplayLocked;
	//���ݵ�ص�ѹ����flagʵ�ֵ͵�ѹ�����͹ػ�����
	if(CurrentMode->ModeIdx==Mode_Ramp)AlertThr=SysCfg.RampBattThres; //�޼�����ģʽ�£�ʹ�ýṹ���ڵĶ�̬��ֵ
	else AlertThr=CurrentMode->LowVoltThres; //�ӵ�ǰĿ�굲λ��ȡģʽֵ  
  if(CellVoltage>2750)		
		{
		IsBatteryAlert=CellVoltage>AlertThr?0:1; //����bit���ݸ�����λ����ֵ�����ж�
		IsBatteryFault=0; //��ص�ѹû�е���Σ��ֵ��fault=0
		}
	else
		{
		IsBatteryAlert=0; //����bit�����ǿ���������bit
		IsBatteryFault=1; //����bit=1
		}
	//��ص���ָʾ״̬��
	BatteryStateFSM();
	//LED����
	if(IsOneTimeStrobe())return; //Ϊ�˱������ֻ����һ�ε�Ƶ��ָʾ����ִ�п��� 
	if(ErrCode!=Fault_None)DisplayErrorIDHandler(); //�й��Ϸ����Ҳ���Ӧ���������Ĺ����룬��ʾ����
	else if(VshowFSMState!=BattVdis_Waiting)BatVshowFSM();//��ص�ѹ��ʾ������ִ��״̬��
	else if(BattShowTimer||CurrentMode->ModeIdx>1)SetPowerLEDBasedOnVbatt(); //�û���ѯ���������ֵ翪����ָʾ����
  else LEDMode=LED_OFF; //�ֵ紦�ڹر�״̬����û�а������µĶ�������LED����Ϊ�ر�
	}
	