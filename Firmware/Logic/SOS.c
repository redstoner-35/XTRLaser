/****************************************************************************/
/** \file SOS.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�������⹦����������SOS
��ȵ�λ�Լ����+���������SOS�������ģʽ��û�е͵籣������SOSĦ��˹���벥����

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "SOS.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//SOSʱ�����ã�ÿ��λ0.125S��
#define SOSDotTime 2 //SOS�ź�(.)��ʱ��	
#define SOSDashTime 6 //SOS�ź�(-)��ʱ��	
#define SOSGapTime 7 //SOS�ź���ÿ����ʾ;�еȴ���ʱ��
#define SOSFinishGapTime 35 //ÿ��SOS����������ĵȴ�ʱ��

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	//SOS״̬ö��
	SOSState_Prepare,
	SOSState_3Dot,
	SOSState_3DotWait,
	SOSState_3Dash,
	SOSState_3DashWait,
	SOSState_3DotAgain,
	SOSState_Wait,
	}SOSStateDef;	

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata SOSStateDef SOSState; //ȫ�ֱ���״̬λ
static xdata char SOSTIM;  //SOS��ʱ

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

static void SOSFSM_Jump(SOSStateDef State,char Time)	//SOS״̬������ת����
	{
	if(SOSTIM)return; //��ʾδ����
	SOSTIM=Time; 
	SOSState=State;  //������ʱ�ȴ��׶�
	}
//SOS��ʱ��״̬���
static bit SOSTIMDetect(char Time)
	{
	//������ʱ�ж�
	if((SOSTIM%(Time*2))>(Time-1))return 1;
	//�ر�״̬����0
	return 0;
	}
/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//SOS״̬����ʱ����
void SOSTIMHandler(void)
	{
	//�Լ�ʱ����ֵ���еݼ�
	if(SOSTIM)SOSTIM--;
	}

//��λ����SOSģ��
void ResetSOSModule(void)
	{
	SOSState=SOSState_Prepare;
	SOSTIM=0;
	}

//SOS״̬������ģ��
char SOSFSM(void)
	{
	switch(SOSState)
		{
		//׼���׶�
		case SOSState_Prepare:
			 SOSTIM=0;
			 SOSFSM_Jump(SOSState_3Dot,(3*SOSDotTime*2)-1);
		   break;
		//��һ�͵ڶ�������
		case SOSState_3DotAgain:
		case SOSState_3Dot:
       if(SOSTIMDetect(SOSDotTime))return 1; //��ǰ״̬��ҪLED����������1
			 if(SOSState==SOSState_3Dot)SOSFSM_Jump(SOSState_3DotWait,SOSGapTime);  //������ʱ�ȴ��׶�
		   else SOSFSM_Jump(SOSState_Wait,SOSFinishGapTime);//������ʱ�ȴ��׶�
		   break;
		//���������ĵȴ���ʱ�׶�
	  case SOSState_3DotWait:
			 SOSFSM_Jump(SOSState_3Dash,(3*SOSDashTime*2)-1);
		   break;
		//����
		case SOSState_3Dash:
			 if(SOSTIMDetect(SOSDashTime))return 1; //��ǰ״̬��ҪLED����������1	
		   SOSFSM_Jump(SOSState_3DashWait,SOSGapTime);
		   break;			
		//����������ĵȴ���ʱ�׶�
	  case SOSState_3DashWait:
			 SOSFSM_Jump(SOSState_3DotAgain,(3*SOSDotTime*2)-1);
		   break;		
	  //�����źŷ�����ϣ��ȴ�
	  case SOSState_Wait:	
			 SOSFSM_Jump(SOSState_Prepare,0);//�ص�׼��״̬
		   break;
		}
	//�����������0�رշ����ӣ�ȷ��β��������ȷ��Ӧ
	return 0;
	}
