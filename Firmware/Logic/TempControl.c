#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "FastOp.h"

//�ڲ�����
static xdata int TempIntegral;
static xdata int TempProtBuf;
static xdata unsigned char StepUpLockTIM; //��ʱ��

//�ڲ�״̬λ
static bit IsNearThermalFoldBack; //���λ���Ƿ�ӽ����˳������¶�
static bit IsThermalStepDown; //���λ���Ƿ񽵵�
static bit IsTempLIMActive;  //�¿��Ƿ��Ѿ�����
static bit IsSystemShutDown; //�Ƿ񴥷��¿�ǿ�ƹػ�

//�ⲿ״̬λ
bit IsPauseStepDownCalc; //�Ƿ���ͣ�¿صļ������̣���bit=1����ǿ�Ƹ�λ�����¿�ϵͳ�����ǻ���ͣ���㣩
bit IsDisableTurbo;  //��ֹ�ٶȽ��뵽������
bit IsForceLeaveTurbo; //�Ƿ�ǿ���뿪������

//�ڲ��궨��
#define LeaveTurboTemperature ForceOffTemp-10   //�˳������¶�Ϊ�ػ������¶�-10

//��ȡϵͳ�Ƿ񴥷�����
bit QueryIsThermalStepDown(void)
	{
	//��ǰ���ڵ�����ʾ״̬�������Ͻ�����ʾ
	if(VshowFSMState!=BattVdis_Waiting)return 0; 
	//������ʾ��־λ
	return IsThermalStepDown;
	}

//������ʱ����ݵ�ǰ���µĵ�������PIֵ
void RecalcPILoop(int LastCurrent)	
	{
	int buf,ModeCur;
	//Ŀ�굲λ����Ҫ����,��λ��������
	if(!CurrentMode->IsNeedStepDown)TempProtBuf=0;
	//��Ҫ��λ��ִ�ж�Ӧ����
	else
		{	
		//��ȡ��ǰ��λ����
		ModeCur=QueryCurrentGearILED();
		//����Pֵ����
		buf=TempProtBuf+(TempIntegral/IntegralFactor); //��������ۼ�ֵ
		if(IsNegative16(buf))buf=0; //�����ۼ�ֵ����С��0
		buf=LastCurrent-buf; //�ɵ�λ������ȥ�ۼ�ֵ�õ�ʵ�ʵ���(mA)
		TempProtBuf=ModeCur-LastCurrent; //Pֵ��������µ�λ�ĵ���-�ɵ�λʵ�ʵ���(mA)
		if(IsNegative16(TempProtBuf))TempProtBuf=0; //�������������С��0
		}
	//�������������
	TempIntegral=0;
	}
	
//�����ǰ�¿ص�����ֵ
int ThermalILIMCalc(void)
	{
	int result;
	//�ж��¿��Ƿ���Ҫ���м���
	if(!IsTempLIMActive)
		{
		result=Current; 				//�¿ر��رգ��������ƽ������ٷ���ȥ����
		IsThermalStepDown=0;  	//ָʾ�¿��ѱ��ر�
		}
	//��ʼ�¿ؼ���
	else
		{
		result=TempProtBuf+(TempIntegral/IntegralFactor); //���ݻ��������
		if(IsNegative16(result))result=0; //������ֵ����
		result=Current-result; //��������ֵ���
		if(result<MinumumILED) //�Ѿ��������ˣ���ֹPID�����ۼ�
			{
		  TempProtBuf=Current-MinumumILED; //�������������޷�Ϊ��С����
		  TempIntegral=0;
		  result=MinumumILED; //�������Ʋ�����С����͵���
			}
    //�ж��¿��Ƿ��Ѿ�����			
		if(result<(Current-200))IsThermalStepDown=1;	//�¿��Ѿ�����������µ�200mA����ʾ�¿ش���
		}
	//���ؽ��	                               
	return result; 
	}

//�¿�ϵͳ�л���׷���¶ȱ仯ʵ�ֺ����Ĵ���
static void ThermalIntegralHandler(bool IsStepDown,bool IsEnableFastAdj)
	{
	int Buf;
	//�������壬�������ֵС��������ϵͳ��Ҫ���ٵ���������������Ժ��¶ȹҹ��Ŀɱ����ʹ���
	#define IsEnableQuickItg (abs(TempIntegral)<(IntegrateFullScale-Buf)&&IsEnableFastAdj)
	//�����¶Ȳ�ͻ�����ֵ
	if(IsStepDown)Buf=Data.Systemp-(LeaveTurboTemperature-8);
	else Buf=(ReleaseTemperature+5)-Data.Systemp; //����ģʽ��ϵͳ�¶����ֵΪǿ�Ƽ������¶�-8������ģʽΪ�ָ��¶�+5
	if(IsNegative16(Buf))Buf=0; //�¶Ȳ��Ϊ����
	//���л��������ε���ֵ�ļ���
	if(IsEnableQuickItg)Buf<<=1;      //���ٵ�������,�����ֵ=�²�*2
	else Buf=0;
	Buf++;  													//������Ҫ��֤Bufʼ��Ϊ1(���ٵ��������ú����ֵ�����Ϊ0)ȷ��������������Ӧ
  //Ӧ�û�����ֵ�����ֻ���
	TempIntegral+=(IsStepDown?Buf:-Buf);
	#undef IsEnableQuickItg            //����궨��ֻ�Ǹú����ľֲ����壬��Ҫ�ں���ĩβ���õ������������ʹ�õ���������
	}
	
static void ThermalIntegralCommitToProtHandler(void)	
	{
	//��ǰ�������ۼƵĲ���С�ڸ�λʱ�䣬�˳�
	if(abs(TempIntegral)<(ILEDRecoveryTime*8))return;
	//�����������ۻ��ı仯�ɱ���Ӧ�õ���������������
	TempProtBuf+=TempIntegral/IntegralFactor;
	TempIntegral=0;						
	}

//�¿�PI������
void ThermalPILoopCalc(void)	
	{
	int ProtFact,Err,ConstantILED;
	bool IsSwitchToITGTrack;
	//PI���رգ���λ��ֵ
	if(!IsTempLIMActive)
		{
		IsNearThermalFoldBack=0;
		TempIntegral=0;
		TempProtBuf=0;
		IsThermalStepDown=0;
		}
	//����PI���ļ���(�������������û����ͣ��ʱ�����)
	else if(!IsPauseStepDownCalc&&GetIfOutputEnabled())
		{			
		//��ȡ�����¶�ֵ�ͺ�������
		ConstantILED=ILEDConstant;
		if(IsNearThermalFoldBack)ConstantILED=ILEDConstantFoldback; //�ӽ��¶����ޣ�����ʹ�ö����µ��ĳ�������
		else ConstantILED=ILEDConstant;
		//�¶����Ϊ�����¶ȴ��ں���ֵ��
		if(Data.Systemp>ConstantTemperature)
			{		
			/**************************************************************
			��ȫ�������ƣ����Ͼ�Ҫ����ǿ�Ƶ��������¶��ˣ�����ʹ�ܱ�־λ��
			����������ǿ�Ƽ���ʹ��P����������͵������������Ա����¶ȼ�
			����ȥ����������´����˳������ı�������
			**************************************************************/
			if(Data.Systemp>LeaveTurboTemperature-3)IsNearThermalFoldBack=1;
			//������(P)
			Err=Data.Systemp-ConstantTemperature;  //���ֵ����Ŀ���¶�-�����¶�
			StepUpLockTIM=24; //����֮���¶ȹ�����֮��ֹͣ3��
				
			//������λ�ߵ�����ǿ��ʱ�ؽ���
			if((CurrentMode->ModeIdx==Mode_Turbo||CurrentMode->ModeIdx==Mode_Burn)&&CurrentBuf>2850)
				{
				//Ѹ�ٽ�����������������
				TempProtBuf+=(20*Err); 
				}
			//����ִ�б����¿�
			else if(Err>2)
				{
				//���������	
				if(CurrentMode->ModeIdx==Mode_Turbo||CurrentMode->ModeIdx==Mode_Burn)
					{
					//����������ģʽ�����б�����ӽ����ٶȣ�����ʹ�õ�һ����б��
	        ProtFact=CurrentBuf/1000;
					}
				else ProtFact=CurrentBuf/1300;
				//�������ύ
				if(IsNegative16(ProtFact))ProtFact=0;
				ProtFact++; //��֤������ʼ����1ȷ��������ȷ����

			  //��ǰLED�����ѱ����Ƶ�����������Χ�ڣ���ֹ���ٽ���������ʹ�ñ�������ٽ���
				if(CurrentBuf<ConstantILED)ThermalIntegralCommitToProtHandler();
				else 
					{
					//����û�дﵽ�������ޣ������ύ��������
					if(IsLargerThanThreeU16(Err))ProtFact*=(Err+2); 			//�¶�������3���϶ȣ����ű���ϵ��
				  TempProtBuf+=(ProtFact*Err);													//��buf�ύ������	
					}
				//���Ʊ��������ֻ�ܴﵽILEDMIN
				if(TempProtBuf>(Current-MinumumILED))TempProtBuf=(Current-MinumumILED); 
				StepUpLockTIM=60; //�������������ͣ7.5��
				}
			//������(I)
			ThermalIntegralHandler(true,CurrentBuf<ConstantILED?true:false); //����С�ڳ���ֵʱʹ�ܿ��ٵ���
			}
		//�¶�С�ں���ֵ���¶����Ϊ����
		else if(Data.Systemp<ConstantTemperature)
			{
			//�������жϵ����Ƿ������ֻ�������
			Err=ConstantTemperature-Data.Systemp;								//������Ŀ���¶�ֵ��ȥϵͳ�¶�
			if(Err>4)
				IsSwitchToITGTrack=true; 	 //�¶ȴ���4�����ϵĸ����˵��ϵͳʹ�ñ������ȿ�����ȴ��������������ٻ��֣�Ѹ����������
			else if(CurrentBuf>(ConstantILED-200))
				IsSwitchToITGTrack=false;  //��ǰϵͳ�����Ѿ��������ӽ�����ˮƽ��ʹ�û�����ÿ��+1��������
			else
				IsSwitchToITGTrack=true; 	 //��ǰϵͳ��������궨�ĳ�������Զ�����������������������������
			//������(P)
			if(StepUpLockTIM)StepUpLockTIM--; //��ǰ����������û�ﵽ����������ʱ��
			else
				{
				//�����ﵽ��������ֵ����ʼʹ�û�������⻺������
				if(IsSwitchToITGTrack)ThermalIntegralCommitToProtHandler();
				//ִ�б�������
				else
					{
					if(IsLargerThanOneU16(Err))TempProtBuf-=Err; //��������
					if(IsNegative16(TempProtBuf))TempProtBuf=0;
					}			
				//�¶������˺ܶ࣬ϵͳ�Ѿ������������ǿ�ƽ���ǰ�ĳ�����������λ���λ
				if(IsNearThermalFoldBack)
					{
					//��ǰִ�еĵ������ڶ��������λfoldback���λ
					if(CurrentBuf>ILEDConstant)IsNearThermalFoldBack=0;  
					}
				}
			//��������ֵ�޷�(�����Ǹ���)
			if(IsNegative16(TempProtBuf))TempProtBuf=0; 
			//������(I)
			ThermalIntegralHandler(false,IsSwitchToITGTrack); //�������ڳ���ֵ�������ģʽʱʹ�ܿ��ٵ���
			}
		}
	}

//�����¶�ʹ�ܿ��Ƶ�ʩ���ش�����
static bit TempSchmittTrigger(bit ValueIN,char HighThreshold,char LowThreshold)	
	{
	if(Data.Systemp>HighThreshold)return 1;
	if(Data.Systemp<LowThreshold)return 0;
	//��ֵ���֣�û�иı�
	return ValueIN;
	}

//�¶ȹ�����
void ThermalMgmtProcess(void)
	{
	bit ThermalStatus;
	//�¶ȴ�����������ִ���¶ȿ���
	if(Data.IsNTCOK)
		{
		//�ֵ��¶ȹ���ʱ�Լ�����������
		IsForceLeaveTurbo=TempSchmittTrigger(IsForceLeaveTurbo,LeaveTurboTemperature,ForceDisableTurboTemp-10);	//�¶Ⱦ���ػ������ļ�಻��10�ȣ������˳�����
		IsDisableTurbo=TempSchmittTrigger(IsDisableTurbo,ForceDisableTurboTemp,ForceDisableTurboTemp-10); //�¶ȴﵽ�رռ���������ֵ���رռ���
		//���ȹػ�����
		IsSystemShutDown=TempSchmittTrigger(IsSystemShutDown,ForceOffTemp,ConstantTemperature-5);
		if(IsSystemShutDown)ReportError(Fault_OverHeat); //������
		else if(ErrCode==Fault_OverHeat)ClearError(); //��������ǰ����
		//PI��ʹ�ܿ���
		if(!CurrentMode->IsNeedStepDown)IsTempLIMActive=0; //��ǰ��λ����Ҫ����
		else //ʹ��ʩ���غ��������¿��Ƿ񼤻�
			{
			ThermalStatus=TempSchmittTrigger(IsTempLIMActive,ConstantTemperature,ReleaseTemperature); //��ȡʩ���ش������Ľ��
			if(ThermalStatus)IsTempLIMActive=1;//ʩ���غ���Ҫ�󼤻��¿أ���������
			else if(!ThermalStatus&&!TempProtBuf&&IsNegative16(TempIntegral))IsTempLIMActive=0; //ʩ���غ���Ҫ��ر��¿أ��ȴ���������Ϊ0���������ر�
			}
		}
	//�¶ȴ��������ϣ����ش���
	else ReportError(Fault_NTCFailed);
	}	
