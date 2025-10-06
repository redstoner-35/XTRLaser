#ifndef _ModeControl_
#define _ModeControl_

#include "stdbool.h"
#include "FastOp.h"

typedef enum
	{
	LVPROT_Disable=0,  //�õ�λ�رյ͵�������
	LVPROT_Enable_Jump=1, //�õ�λ�͵�������������������������ֵ��ִ������
	LVPROT_Enable_OFF=2		//�õ�λ�͵�������������������������ֵ������ִ�йػ�
	}LVProtectTypeDef;	
	
typedef struct
	{
	int RampCurrent;
	int RampBattThres;
	int RampCurrentLimit;
	unsigned char RampLimitReachDisplayTIM;
	unsigned char CfgSavedTIM;
	}SysConfigDef;	
	
typedef enum
	{
	Mode_OFF=0, //�ػ�
	Mode_Fault=1, //���ִ���
	//�޼�����
	Mode_Ramp=2, //�޼�����
	//�������ݵ�λ
	Mode_Low=3, //����
	Mode_Mid=4, //����
	Mode_MHigh=5,   //�и���
	Mode_High=6,   //����
	//���⵲λ
	Mode_Turbo=7, //����
	Mode_SOS=8, //SOS
	Mode_Focus=9, //�Խ�ר�õ�
	Mode_Burn=10, //�ն���ר�õ�
	Mode_Breath=11, //����ģʽ
	Mode_Beacon=12,  //��Ъ��ģʽ
	Mode_SOS_NoProt=13 //�ޱ���SOSģʽ
	}ModeIdxDef;
	

typedef struct
	{
  ModeIdxDef ModeIdx;
  int Current; //��λ����(mA)
	int MinCurrent; //��С����(mA)�����޼�������Ҫ
	int LowVoltThres; //�͵�ѹ����ѹ(mV)
	bool IsModeHasMemory; //�Ƿ������
	bool IsNeedStepDown; //�Ƿ���Ҫ����
	//�Ƿ�������뼫���ͱ���
	bool IsEnterTurboStrobe; 
	//�͵�����������
  ModeIdxDef ModeWhenLVAutoFall;		//�͵�����������֮�������ִ�йػ����Զ���ת�ĵ�λ
	LVProtectTypeDef LVConfig;        //�͵����������Ƶ�����
	//��λ�л�����
  ModeIdxDef ModeTargetWhenH;
	ModeIdxDef ModeTargetWhen1H;	 //ģʽ��λ�л����ã������͵���+�����л�����Ŀ�굲λ
	}ModeStrDef; 

//�ⲿ����
extern xdata unsigned char DisplayLockedTIM; //������ʾ��ʱ��
extern ModeStrDef *CurrentMode; //��ǰģʽ�ṹ��
extern xdata ModeIdxDef LastMode; //��һ����λ	
extern xdata SysConfigDef SysCfg; //�޼���������	
extern bit IsSystemLocked;		//ϵͳ�Ƿ�������
extern bit IsEnableIdleLED;	//�Ƿ���������ʾ	
extern bit IsEnable2SMode;    //�Ƿ���˫�ģʽ
	
/************************************************
����LD�����������������Զ����壬�ú�������ϵͳ��
��������ƣ�Ϊ��ȷ����������������������������
�޸Ļ����������	
************************************************/	
#ifdef USING_LD_NUBB33
	
	//ʹ��NUBB13/23/33 �Ĺ�455nm LD
	#define SingleCellModeICCMAX 1900
	#define DualCellTurboCurrent 4250

#elif defined(USING_LD_NUBB37) 

	//ʹ��NUBB27/37 ���455nm LD	
	#define SingleCellModeICCMAX 1500
	#define DualCellTurboCurrent 3750
	
#elif defined(USING_LD_NURM11T)
	
	//ʹ��NURM11T �Ĺ�˫��638nm LD
	#define SingleCellModeICCMAX 1800
	#define DualCellTurboCurrent 3500
	
#else

	//����LD����Ĭ�ϰ���1.6A��
	#define SingleCellModeICCMAX 1600
	#define DualCellTurboCurrent 3500
	
#endif

//����궨��
#define QueryCurrentGearILED() CurrentMode->Current //��ȡ��ǰ��λ�ĵ�������
#define ModeTotalDepth 14 //ϵͳһ���м�����λ			
	
//����
ModeStrDef *FindTargetMode(ModeIdxDef Mode,bool *IsResultOK);//����ָ����Index����index�����ҵ�Ŀ��ģʽ�ṹ�岢����ָ��
void ModeFSMTIMHandler(void);//��λ״̬������������ʱ������
void ModeSwitchFSM();//��λ״̬��
void SwitchToGear(ModeIdxDef TargetMode);//����ָ����λ
void ReturnToOFFState(void);//�ػ�	
void HoldSwitchGearCmdHandler(void); //�����������	
void ModeFSMInit(void); //��ʼ��״̬��	

#endif
