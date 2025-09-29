#include "LEDMgmt.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SysConfig.h"
#include "ADCCfg.h"
#include "TempControl.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "ModeControl.h"
#include "VersionCheck.h"
#include "SOS.h"
#include "BreathMode.h"
#include "Beacon.h"

//��λ�ṹ��
code ModeStrDef ModeSettings[ModeTotalDepth]=
	{
		//�ػ�״̬
    {
		Mode_OFF,
		0,
		0,  //����0mA
		0,  //�ػ�״̬��ֵΪ0ǿ�ƽ������
		true,
		false,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_OFF,							 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Disable,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_OFF,
		Mode_OFF	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}, 
		//������
		{
		Mode_Fault,
		0,
		0,  //����0mA
		0,
		false,
		false,
		//�����Ƿ�������뱬��
		false,
		//�͵�����������
		Mode_OFF,							 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Disable,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_OFF,
		Mode_OFF	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}, 	
	  //�޼�����		
		{
		Mode_Ramp,
		3000,  //��� 3A����
		550,   //��С 0.55A����
		3200,  //3.2V�ض�
		false, //���ܴ�����  
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Ramp,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Disable,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_OFF,
		Mode_OFF	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		},
    //����
		{
		Mode_Low,
		600,  //0.6A����
		0,   //��С����û�õ�������
		2850,  //2.85V�ض�
		true,
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Low,				 					//�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_Mid,
		Mode_OFF		//ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		},
    //����
		{
		Mode_Mid,
		1200,  //1.2A����
		0,   //��С����û�õ�������
		3000,  //3V�ض�
		true,
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Low,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_Jump,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_MHigh,
		Mode_Low	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}, 	
    //�и���
		{
		Mode_MHigh,
		1800,  //1.8A����
		0,   //��С����û�õ�������
		3100,  //3.1V�ض�
		true,
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Mid,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_Jump,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_High,
		Mode_Mid	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}, 	
    //����
		{
		Mode_High,
		2500,  //2.5A����
		0,   //��С����û�õ�������
		3200,  //3.2V�ض�
		true,
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_MHigh,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_Jump,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_Low,
		Mode_MHigh	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}, 	
    //����
		{
		Mode_Turbo,
		4100,  //4.1A����	
		0,   //��С����û�õ�������
		3350,  //3.35V�ض�
		false, //�������ܴ�����
		true,
		//�����Ƿ�������뱬��
		false,
		//�͵�����������
		Mode_Turbo,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Disable,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_OFF,
		Mode_High	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		},
	  //SOS��ȵ�λ
		{
		Mode_SOS,
		800,  //0.8A����	
		0,   //��С����û�õ�������
		2850,  //2.85V�ض�
		false, //���⵲λ���ܴ�����
		true,
		//�����Ƿ�������뱬��
		false,
		//�͵�����������
		Mode_SOS,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_Breath,
		Mode_Beacon	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		},
		//�������Թ�ѧ�ĶԽ���λ
		{
		Mode_Focus,
		200,  //0.2A����	
		0,   //��С����û�õ�������
		2850,  //2.85V�ض�
		false, //���⵲λ���ܴ�����
		false,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Focus,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_Low,
		Mode_OFF	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)		
		},
		//�����ն����ĵ㶯ģʽ
		{
		Mode_Burn,
		4100,  //4.1A���������¿�ʼ���ƣ�	
		200,   //�ڰ����ɿ�״̬��200mA
		3350,  //3.35V�ض�
		false, //���⵲λ���ܴ�����
		true,
		//�����Ƿ�������뱬��
		true,
		//�͵�����������
		Mode_Focus,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,        //�͵����������Ƶ�����
		//��λ�л�����
		Mode_OFF,
		Mode_OFF	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)		
		},
	  //�����ű�����λ
		{
		Mode_Breath,
		3000,  //3A����	
		160,   //����ģʽ���160mA
		3000,  //3V�ض�
		false, //���⵲λ���ܴ�����
		true,
		//�����Ƿ�������뱬��
		false,
		//�͵�����������
		Mode_Breath,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,   //�͵����������Ƶ�����
		//��λ�л�����
		Mode_Beacon,
		Mode_SOS	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		},
		//���ڿ����ű���
		{
		Mode_Beacon,
		3000,  //3A����	
		0,   	 //��С����û�õ�������
		3000,  //3V�ض�
		false, //���⵲λ���ܴ�����
		true,
		//�����Ƿ�������뱬��
		false,
		//�͵�����������
		Mode_Beacon,				 //�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
		LVPROT_Enable_OFF,   //�͵����������Ƶ�����
		//��λ�л�����
		Mode_SOS,
		Mode_Breath	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ(����OFF��ʾ�������л�)
		}			
	};

//ȫ�ֱ���(��λ)
ModeStrDef *CurrentMode; //��λ�ṹ��ָ��
xdata ModeIdxDef LastMode; //��λ����洢
xdata ModeIdxDef LastModeBeforeTurbo; //��һ�����뼫���ĵ�λ
xdata SysConfigDef SysCfg; //ϵͳ����	

//ȫ�ֱ���(״̬λ)
bit IsSystemLocked;		//ϵͳ�Ƿ�������
bit IsEnableIdleLED;	//�Ƿ���������ʾ
bit IsBurnMode;        //�Ƿ���뵽������ģʽ	
bit IsEnable2SMode;    //�Ƿ���˫�ģʽ	
	
//ȫ�������ʱ����
xdata unsigned char HoldChangeGearTIM; //��λģʽ�³�������
xdata unsigned char DisplayLockedTIM; //������ս��ģʽ�����˳���ʾ

//�ڲ������ͱ�־λ
static xdata unsigned int BurnModeTimer;  //�㰴��ʱ��ʱ��
static xdata unsigned char RampDIVCNT; //�޼����⽵�͵����ٶȵķ�Ƶ��ʱ��		
static bit IsRampKeyPressed;  //��־λ���û��Ƿ��°������޼�������е���
static bit IsNotifyMaxRampLimitReached; //����޼�����ﵽ������	
static bit RampEnteredStillHold;    //�޼��������󰴼���Ȼ��ס
	
//����ָ����Index����index�����ҵ�Ŀ��ģʽ�ṹ�岢����ָ��
ModeStrDef *FindTargetMode(ModeIdxDef Mode,bool *IsResultOK)
	{
	unsigned char i;
	*IsResultOK=false;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode)
		{
		*IsResultOK=true;
		break;
		}
	//���ض�Ӧ��index
	return &ModeSettings[i];
	}
	
//��ʼ��ģʽ״̬��
void ModeFSMInit(void)
	{
	bool Result;
  //��λ������͵�λģʽ����ϵͳ
	LastMode=Mode_Low;
	LastModeBeforeTurbo=Mode_Low;
	ErrCode=Fault_None; 					//û�й���
	//��ʼ���޼�����
	SysCfg.RampLimitReachDisplayTIM=0;
  ReadSysConfig(); //��EEPROM�ڶ�ȡ�޼���������
	
	CurrentMode=FindTargetMode(Mode_Ramp,&Result);//������λ���ýṹ��Ѱ���޼�����ĵ�λ����ȡ����
	if(Result)
		{
		SysCfg.RampBattThres=CurrentMode->LowVoltThres; //��ѹ������޻ָ�
		SysCfg.RampCurrentLimit=IsEnable2SMode?QueryCurrentGearILED():1800;                   			//�ҵ���λ�������޼�����ĵ�λ���������޻ָ�
		if(SysCfg.RampCurrent<CurrentMode->MinCurrent)SysCfg.RampCurrent=CurrentMode->MinCurrent;
		if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)SysCfg.RampCurrent=SysCfg.RampCurrentLimit;		//��ȡ���ݽ����󣬼�����������Ƿ�Ϸ������Ϸ���ֱ������
		CurrentMode=&ModeSettings[0]; 					//��������Ϊ��һ����
		}
	//�޷��ҵ��޼�������ֵ����λ������٣�����
  else ReportError(Fault_RampConfigError);
	//��λ������һ����ģ��
	DisplayLockedTIM=0;
	IsBurnMode=0;
	IsPauseStepDownCalc=0;                    //ÿ�γ�ʼ��clear����ͣ�¿ؼ���ı�־λ
	IsNotifyMaxRampLimitReached=0;
	RampEnteredStillHold=0;
	RampDIVCNT=RampAdjustDividingFactor; 			//��λ��Ƶ������	
	ResetSOSModule(); 
	BeaconFSM_Reset(); 
  BreathFSM_Reset();	                      //��λSOS�ͺ������ű�ģʽ״̬��
	}	

//��λ״̬������������ʱ������
void ModeFSMTIMHandler(void)
{
	//�޼�������صĶ�ʱ��
	if(IsLargerThanOneU8(SysCfg.CfgSavedTIM))SysCfg.CfgSavedTIM--;
	if(SysCfg.RampLimitReachDisplayTIM)
		{
		SysCfg.RampLimitReachDisplayTIM--;
		if(!SysCfg.RampLimitReachDisplayTIM)IsNotifyMaxRampLimitReached=0;
		}
	//����ģʽ��ʱ��ʱ��
	if(BurnModeTimer)BurnModeTimer--;
	//����������ʾ��ʱ��
  if(DisplayLockedTIM)DisplayLockedTIM--;
}

//��λ��ת
void SwitchToGear(ModeIdxDef TargetMode)
	{
	bool IsLastModeNeedStepDown,Result;
	ModeStrDef *ModeBuf;
	//��ǰ��λ�Ѿ���Ŀ��ֵ����ִ��
	if(TargetMode==CurrentMode->ModeIdx)return;
	//��¼����ǰ�Ľ��	
	IsLastModeNeedStepDown=CurrentMode->IsNeedStepDown; //�����Ƿ���Ҫ����
	//��ʼѰ��
	ModeBuf=FindTargetMode(TargetMode,&Result);
	if(!Result)return;                    //�Ҳ�����Ӧ�ĵ�λ���˳�
	
	//Ӧ�õ�λ��������¼��㼫������,ͬʱ��λ���⵲λ״̬��
	switch(TargetMode)
		{
		case Mode_SOS:ResetSOSModule();break;
		case Mode_Breath:BreathFSM_Reset();break;
		case Mode_Beacon:BeaconFSM_Reset();break;
		}
	CurrentMode=ModeBuf;		
	//������ϵ�λ���ǳ�����������������PI�������������
	if(TargetMode>2&&IsLastModeNeedStepDown)RecalcPILoop(Current); 	
	}

//�����ػ�����	
void ReturnToOFFState(void)
	{
	switch(CurrentMode->ModeIdx)
		{
		case Mode_Fault:
		case Mode_OFF:return;  //�Ƿ�״̬��ֱ�Ӵ������������ִ��

		//���൲λ��ִ���ж�
		default:break;
		}
  //ִ�е�λ���䲢���ص��ػ�״̬
	if(CurrentMode->IsModeHasMemory)LastMode=CurrentMode->ModeIdx;
	SwitchToGear(Mode_OFF); 
	}	
	
//���������ļ����������
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	//��ǰϵͳ�������⵲λ״̬(����ģʽ�����ն����ͳ���������ͻ)����ִ�д���	
	if(CurrentMode->ModeIdx==Mode_Burn)HoldChangeGearTIM=0; 
	//�����ɿ�����ϵͳ���ڷ�����״̬����ʱ����λ
	else if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())HoldChangeGearTIM=0; 
	//ִ�л�������
	else 
		{
		buf=HoldChangeGearTIM&0x1F; //ȡ��TIMֵ
		if(!buf&&!(HoldChangeGearTIM&0x40))HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80; //�������λ1ָʾ�������Լ���
		HoldChangeGearTIM&=0xE0; //ȥ����ԭʼ��TIMֵ
		if(buf<HoldSwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //ʱ�䵽��������
		HoldChangeGearTIM|=buf; //����ֵд��ȥ
		}
	}	

//�޼����⴦��
static void RampAdjHandler(void)
	{	
  int Limit;
	bit IsPress;
  //������޼���������
	IsPress=getSideKey1HEvent()|getSideKeyHoldEvent();
	Limit=IsEnable2SMode?QueryCurrentGearILED():1800;                    //˫�ģʽ���Ƶ�������ܳ���1.8A
	Limit=SysCfg.RampCurrentLimit<Limit?SysCfg.RampCurrentLimit:Limit;
	if(Limit<CurrentMode->Current&&IsPress&&SysCfg.RampCurrent>Limit)SysCfg.RampCurrent=Limit; //�ڵ��������Ƶ�������û����°������Ե��������������޷�
	//�������ȵ���
	if(getSideKeyHoldEvent()&&!IsRampKeyPressed) //�������ӵ���
			{	
			if(RampDIVCNT)RampDIVCNT--;
			else 
				{
				//ʱ�䵽����ʼ���ӵ���
				if(SysCfg.RampCurrent<Limit)SysCfg.RampCurrent++;
				else
					{
					IsNotifyMaxRampLimitReached=1; //����Ѵﵽ����
					SysCfg.RampLimitReachDisplayTIM=4; //Ϩ��0.5��ָʾ�Ѿ�������
					SysCfg.RampCurrent=Limit; //���Ƶ������ֵ	
					IsRampKeyPressed=1;
					}
				//��ʱʱ�䵽����λ����
				RampDIVCNT=RampAdjustDividingFactor;
				}
			}	
	else if(getSideKey1HEvent()&&!IsRampKeyPressed) //����+�������ٵ���
		 {
			if(RampDIVCNT)RampDIVCNT--;
			else
				{
				if(SysCfg.RampCurrent>CurrentMode->MinCurrent)SysCfg.RampCurrent--; //���ٵ���	
				else
					{
					IsNotifyMaxRampLimitReached=0;
					SysCfg.RampLimitReachDisplayTIM=4; //Ϩ��0.5��ָʾ�Ѿ�������
					SysCfg.RampCurrent=CurrentMode->MinCurrent; //���Ƶ�����Сֵ
					IsRampKeyPressed=1;
					}
				//��ʱʱ�䵽����λ����
				RampDIVCNT=RampAdjustDividingFactor;
				}
		 }
  else if(!IsPress&&IsRampKeyPressed)
		{
	  IsRampKeyPressed=0; //�û��ſ��������������		
		RampDIVCNT=RampAdjustDividingFactor; //��λ��Ƶ��ʱ��
		}
	//�������ݱ�����ж�
	if(IsPress)SysCfg.CfgSavedTIM=32; //��������˵�����ڵ�������λ��ʱ��
	else if(SysCfg.CfgSavedTIM==1)
		{
		SysCfg.CfgSavedTIM--;
		SaveSysConfig(0);  //һ��ʱ����û����˵���Ѿ�������ϣ���������
		}
	}
//���йػ��Ϳ���״ִ̬��N��+�����¼�����ĺ���
static void ProcessNClickAndHoldHandler(void)
	{
  //����ִ�д���
	switch(getSideKeyNClickAndHoldEvent())
		{
		case 1:	//����+��������Խ�ר�õ�λ
			if(CurrentMode->ModeIdx!=Mode_OFF)break;
			SwitchToGear(Mode_Focus);
			break; 
		case 2:TriggerVshowDisplay();break; //˫��+������ѯ����
		case 3:TriggerTShowDisplay();break;//����+������ѯ�¶�
		case 4:
			  //�ػ�״̬���Ļ�+���������޼����Ⲣǿ��������͵���
			  if(CurrentMode->ModeIdx!=Mode_OFF)break;
				if(CellVoltage>2850)
					{
					SwitchToGear(Mode_Ramp);
					RampRestoreLVProtToMax();
					RampEnteredStillHold=1;
					}
				else LEDMode=LED_RedBlinkFifth;	//�ֵ紦�ڹػ�״̬���ҵ�ص������㣬��˸�����ʾ����ȥ	
		    break;
		//�������ʲô������
		default:break;			
		}
	}	

//��������ͨģʽ
static void PowerToNormalMode(ModeIdxDef Mode)
	{
	if(CellVoltage>3050)SwitchToGear(Mode); //��������		
	else if(CellVoltage>2750)SwitchToGear(Mode_Low); //��������������
	else if(CurrentMode->ModeIdx==Mode_OFF)LEDMode=LED_RedBlinkFifth;	//�ֵ紦�ڹػ�״̬���ҵ�ص������㣬��˸�����ʾ����ȥ	
	else ReturnToOFFState();	 //��ص������ز��㣬���ֵ翪�ţ�ֱ�ӹػ�
	}
	
//���Խ��뼫���ͱ����Ĵ���
void TryEnterTurboProcess(char Count)	
	{
	//��˫��ģʽ���˳�
  if(Count!=2)return;
	//δ����2Sģʽ��˫���л���1.8A��λ
  if(!IsEnable2SMode)PowerToNormalMode(Mode_MHigh);
	//��ص���������û�д����رռ����ı�������������
	else if(CellVoltage>3450&&!IsDisableTurbo)
			{
			if(CurrentMode->ModeIdx>1)LastModeBeforeTurbo=CurrentMode->ModeIdx; //���½��뼫��֮ǰ�ĵ�λ
			if(LastMode>2&&LastMode<7)LastMode=CurrentMode->ModeIdx; //�뿪ѭ������ʱ�򣬸���ѭ����λ��������
		  SwitchToGear(Mode_Turbo); 
			}
	//��ص�ص���������߼������������Կ�������ȥ
	else PowerToNormalMode(Mode_High);	
	}
	
	
//����ģʽ״̬���ı�����ģ�鴦��	
static void ModeSwitchFSMTableDriver(char ClickCount)
	{
	if(CurrentMode->IsEnterTurboStrobe)TryEnterTurboProcess(ClickCount);//��ȡ��ǰ��ģʽ�ṹ�壬ִ�н��뼫�����߱����ļ��	
  if(IsLargerThanOneU8(CurrentMode->ModeIdx)) //����1�ıȽ�									
		{
		//ϵͳ�ڿ���״̬���ұ�־λ��Ч֮����ִ�е�����ʾ�������
		ProcessNClickAndHoldHandler();
		//�ఴ�����ػ�	
		if(ClickCount==1)ReturnToOFFState();
		}
 	if(HoldChangeGearTIM&0x80)	 
		{
		//����λ���ݿ��ڵ�״̬��ʹ�ܳ���������������������ʱ��ִ��˳�򻻵�
		HoldChangeGearTIM&=0x7F;
    if(!IsEnable2SMode&&CurrentMode->ModeTargetWhenH==Mode_High)SwitchToGear(Mode_Low);       //��˫﮵�ģʽ�����⵽˳�򻻵���ͼ�����ߵ�λ���򻻵��͵�����ѭ��			
		else if(CurrentMode->ModeTargetWhenH!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhenH); 		
		}
	
	if(HoldChangeGearTIM&0x20)  
		{
		//����λ���ݿ��ڵ�״̬��ʹ�ܵ���+����������������������ʱ��ִ�����򻻵�
		HoldChangeGearTIM&=0xDF; 
		if(CurrentMode->ModeTargetWhen1H!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhen1H); 
		}
	
	if(CurrentMode->LVConfig)BatteryLowAlertProcess(CurrentMode->LVConfig&0x02,CurrentMode->ModeWhenLVAutoFall); //ִ�е͵�������
	}	
	
//��λ״̬��
void ModeSwitchFSM(void)
	{
	char ClickCount;
	ModeIdxDef ModeBeforeFSMSwitch;
	//��ȡ����״̬
	if(!IsKeyFSMCanEnable())return;         //�ڳ����ϵ�׶��������״̬��δ���ã�����������ִ��
	ClickCount=getSideKeyShortPressCount();	//��ȡ�����������������Ĳ���
		
	//��λ����������
	if(LastMode<3||LastMode>6)LastMode=Mode_Low;				//ȫ�ֳ������
		
	//����FSM�������߼�����		
  ModeBeforeFSMSwitch=CurrentMode->ModeIdx;		 //���½���֮ǰ�ĵ�λ
	if(VChkFSMState==VersionCheck_InAct)switch(ModeBeforeFSMSwitch)	
		{
		//�ػ�״̬
		case Mode_OFF:		  
		   //��������ģʽ
       if(IsSystemLocked)
				{
				//����������̵������β��ұ���״̬
				if(ClickCount==5)
					{
					LEDMode=LED_GreenBlinkThird; 
					IsSystemLocked=0;
					if(CellVoltage>2850)DisplayLockedTIM=5;  //��ص�ѹ�㹻ʱ��LD����0.5��ָʾ�����ɹ�
					SaveSysConfig(0);
					}
				//���ఴ���¼�����ɫ�������ʾ������
				else if(IsKeyEventOccurred())LEDMode=LED_RedBlinkFifth;
				//�������ദ��
				break;
				}
		  
			//ϵͳ��δ���ڰ�ס��״̬�£��ఴ�������������޼�����
			if(!RampEnteredStillHold&&getSideKeyLongPressEvent())
				{
				if(CellVoltage>2850)
					{
					SwitchToGear(Mode_Ramp);
					RampRestoreLVProtToMax();
					RampEnteredStillHold=1;
					}
				else LEDMode=LED_RedBlinkFifth;	//�ֵ紦�ڹػ�״̬���ҵ�ص������㣬��˸�����ʾ����ȥ	
				}				
			//�û�������ģʽ�˳����ȴ��û��ɿ�������ִ�н����޼�����ģʽ�Ĳ���
			else if(RampEnteredStillHold&&!getSideKeyHoldEvent())RampEnteredStillHold=0;	
				
		  //������ģʽ�����������ػ���ִ��һ��������������ת���޼�����
			else switch(ClickCount)
				{
				case 1:
					//�ఴ��������������ѭ����λ��һ�ιرյ�ģʽ�����ڿ����˼���������£�
					PowerToNormalMode(LastMode);
					break; 
        case 3:
					//�ػ�״̬�²ఴ��������SOS
          if(CellVoltage>2750)SwitchToGear(Mode_SOS); //��������������SOS
				  else LEDMode=LED_RedBlinkFifth;
          break;
        case 4:
          //�ػ�״̬�ҵ�ѹ���㣬ϵͳ�¶�����ʱ��������ģʽ
          if(!IsDisableTurbo&&CellVoltage>3350)  
						{
						BurnModeTimer=8*BurnModeTimeOut; //��λ����ģʽ��ʱ��
						IsBurnMode=1;                    //��ǽ�������ģʽ
						SwitchToGear(Mode_Burn);
						}
					//��ص�ѹ���㣬�޷���������ģʽ
					else LEDMode=LED_RedBlinkFifth;
					break;	
        case 5:
          //�ѽ�������״̬��������
				  if(IsSystemLocked)break;  
			    //�ػ�״̬����ϵͳδ�����������������ģʽ���ఴ��ɫ�����Σ����Ƶ���1.5��                
					LEDMode=LED_RedBlinkThird; 
					IsSystemLocked=1;
					SaveSysConfig(0);
				  break;
		    case 6:
					#ifndef USING_LD_NURM11T
					//�ػ�״̬��6������1S��2S֮���л���ؽ���
					if(Data.RawBattVolt>4.35)IsEnable2SMode=1; 	//��ǰ��װ�ĵ����2�ڣ�ʼ�ձ��ֿ���2Sģʽ
					else IsEnable2SMode=IsEnable2SMode?0:1; 		//��ǰ��װ�ĵ����1�ڣ�����ת״̬��1S/2S֮���л�
					//����ಿ������˸��ʾ�û���ǰ�Ľ�������
					if(!IsEnable2SMode)MakeFastStrobe(LED_Amber);
					else Trigger2SModeEnterInfo();               //���ģʽ��ɫ��һ�Σ�˫�ģʽ��ɫ������
					//���2Sģʽ���ã����������޼�����������޽����޷�
				  if(!IsEnable2SMode)
						{
						//1Sģʽ���޼�����������ܳ���1.8A
						if(SysCfg.RampCurrent>1800)SysCfg.RampCurrent=1800;
						if(SysCfg.RampCurrentLimit>1800)SysCfg.RampCurrentLimit=1800;
						}
					//������ĺ����������
					SaveSysConfig(0);
          #endif						
					break;
				case 7:
					//7���л���Դҹ�⹦��
					IsEnableIdleLED=IsEnableIdleLED?0:1; //��ת״̬
					MakeFastStrobe(IsEnableIdleLED?LED_Green:LED_Red);  //������ʾһ��
					SaveSysConfig(0);  //��������
				  break;
			  case 8:
					//8����ѯ�̼��汾
					VersionCheck_Trigger();
				  break;
				//�������ʲô������
        default:break;				
				}		
		  //N��+������ѯ��ѹ���¶ȺͰ�ȫ����
			ProcessNClickAndHoldHandler();
  		break;
		//���ִ���	
		case Mode_Fault:
		  //������������
		  if(IsErrorFatal())break;	 
			//����������״̬�û����°�ť���������������⹦��ģ���������Ϩ��
			if(getSideKeyLongPressEvent())ClearError();
		  break;		
    //�޼�����״̬				
    case Mode_Ramp:
			  if(RampEnteredStillHold)
					{
					SysCfg.RampLimitReachDisplayTIM=0;
					//�ȴ������ſ��ٴ���
					if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())RampEnteredStillHold=0;
					}
				else RampAdjHandler();					    //�޼����⴦��
		    //ִ�е͵�ѹ����
				RampLowVoltHandler(); 				
		    break;
		//����״̬
    case Mode_Turbo:
			  if(IsForceLeaveTurbo)PowerToNormalMode(Mode_Low); //�¶ȴﵽ����ֵ��ǿ�Ʒ��ص�����
			  else if(ClickCount==2)
					{
					//˫����������������ǰ�ĵ�λ������Ǿ۽���λ���룬�������͵�λ��
					if(LastModeBeforeTurbo==Mode_Focus)PowerToNormalMode(Mode_Low);
					else PowerToNormalMode(LastModeBeforeTurbo); 
					LastModeBeforeTurbo=Mode_Low;   //ÿ��ʹ���˼������������λ����
					}
		    break;		
    //����ģʽ
    case Mode_Burn:
			  //ϵͳ���ȴﵽ�˳������ı�׼��ʱ���޲�����ϵͳ�ر�
        if(IsForceLeaveTurbo||!BurnModeTimer)
					{
					RampEnteredStillHold=1; //��ǵ�ǰϵͳ���ڰ���״̬����ֹ�����޼���������û��ڰ��Ű�����ʱ��ϵͳ��Ϊ�͵����رպ�����޼�����
					ReturnToOFFState();
					}
		    break;
		}
		
	//����FSM�еı���������
	if(ModeBeforeFSMSwitch==CurrentMode->ModeIdx&&VChkFSMState==VersionCheck_InAct)
		{
		//���״̬��FSM���в������ߵ�ǰ���ڰ汾���״̬������������������ִ�б�����
		ModeSwitchFSMTableDriver(ClickCount); 
		}
	//������������Ӧ��ϣ��������״̬
	ClearShortPressEvent(); 
	//ϵͳ���ڵ��ģʽ��ǿ�����Ƹ����ͼ��������ص�����		
  if(!IsEnable2SMode&&(CurrentMode->ModeIdx==Mode_High||CurrentMode->ModeIdx==Mode_Turbo))PowerToNormalMode(Mode_Low);
  //������ģʽ�����Burnλ
	if(IsBurnMode&&CurrentMode->ModeIdx!=Mode_Burn)IsBurnMode=0;
  //Ӧ���������
	if(DisplayLockedTIM)Current=200; //�û���������˳���������100mA���ݵ�����ʾһ��
	else if(VChkFSMState!=VersionCheck_InAct)Current=VersionCheckFSM()?100:0; //�汾��ʾ����
	else if(LowPowerStrobe())Current=30; //������ѹ��������ʱ����˸��ʾ
	else switch(CurrentMode->ModeIdx)
		{
		case Mode_Beacon:
			  //�ű�ģʽ��������״̬������
		    IsPauseStepDownCalc=1;		//Ĭ��ϵͳ�����¿ؼ�����ͣ��״̬
				switch(BeaconFSM())
					{
					case 0:Current=0;break; //0��ʾ�õ����ر�
					case 2:Current=200;break; //��200mA������ʾ��֪�û��ѽ����ű�ģʽ
					default:
						IsPauseStepDownCalc=0;     //���������ʱ���¿�����
						Current=IsEnable2SMode?QueryCurrentGearILED():1800; //����ֵ����ϵͳĬ�ϵ����������
						break;
					} 	
			  break;
		case Mode_Breath:	
			  //����ģʽ��������״̬������
				Current=BreathFSM_Calc();
				IsPauseStepDownCalc=Current>650?0:1;   //����ģʽ�µ�������650mA�Ž��м���
				break;
		case Mode_Burn:
			  //����ģʽ��������������ʹ����ߵ����������ɿ�ʹ�õ͵�������LD���жԽ�
			  if(!IsEnable2SMode)Current=getSideKeyHoldEvent()?1800:CurrentMode->MinCurrent;
				else Current=getSideKeyHoldEvent()?QueryCurrentGearILED():CurrentMode->MinCurrent;
		    //����ģʽ��ֻ�а��°����Ŵ��¿ؼ���
		    if(Current==CurrentMode->MinCurrent)IsPauseStepDownCalc=1; //�����ɿ�����ͣ�¿ؼ���
				else	
					{
					//�������£���λ����ģʽ��ʱ��ʱ���������¿�����
					BurnModeTimer=8*BurnModeTimeOut;
					IsPauseStepDownCalc=0;
					}
				break; 			
		case Mode_SOS:
				//SOSģʽ������״̬������
			  Current=SOSFSM()?QueryCurrentGearILED():0;
				IsPauseStepDownCalc=!Current?1:0;              //SOS��λ�������鴦��Ϩ��״̬������ͣ�¿ؼ���
				break; 
		case Mode_Ramp:
			  IsPauseStepDownCalc=0;              //�޼����⵲λ�¼���ʼ�տ���
				//�޼�����ģʽȡ�ṹ��������
				if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)Current=SysCfg.RampCurrentLimit;
				else Current=SysCfg.RampCurrent;	
		    //�޼�����ģʽָʾ(�޼�����ģʽ�ڵִ������޺����Ϩ����ߵ���25%)
				if(SysCfg.RampLimitReachDisplayTIM)Current=IsNotifyMaxRampLimitReached?Current>>2:30;
		    break;
		
		//������λʹ������ֵ��ΪĿ�����	
		default:
			  IsPauseStepDownCalc=0;              //���൲λ����ʼ�տ���
			  Current=QueryCurrentGearILED();	
		    break;
		}
	//ϵͳ���ڵ��ģʽ�����Ƶ���ֵ��߲��ܳ���1.8A
	if(!IsEnable2SMode&&Current>1800)Current=1800;
	}
