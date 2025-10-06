/****************************************************************************/
/** \file Beacon.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�������⹦��������������
�ű������ܣ�ϵͳ�����Ե������������Ӻ�Ϩ��Ȼ���Ժ㶨�����LD���幤��������������

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "Beacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//�����ű�������
#define BeaconOnTime 75 //�ű���˸ʱ��
#define BeaconOFFTime 3 //�ű�ر�ʱ��(��)
#define BeaconInfoTime 3 //�ű��ڿ�ʼ֮ǰ������ʾ�û���ʱ��(��)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

typedef enum
	{
	BeaconState_Init,
	BeaconState_InfoUser,
	BeaconState_ONStrobe,
	BeaconState_OFFWait,
	}BeaconStateDef;

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata BeaconStateDef State;
static xdata unsigned char BeaconOnTIM;
static xdata unsigned char BeaconOffTIM;

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

//��λ״̬��
void BeaconFSM_Reset(void)
	{
	State=BeaconState_InfoUser;
	BeaconOnTIM=0;
	BeaconOffTIM=8*BeaconInfoTime;
	}

//�ر�ʱ���ʱ
void BeaconFSM_TIMHandler(void)
	{
	if(BeaconOffTIM)BeaconOffTIM--;
	}

//�ű�ģʽ״̬��
char BeaconFSM(void)
	{
	switch(State)
		{
		case BeaconState_InfoUser:	
			if(BeaconOffTIM>0)return 2; //��ǰ������ʾ״̬����ʾ�û�
		  //��ʾʱ�䵽����ת��OFF�׶�׼����ʼ��ʾ
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
			break;
		//��ʼ��
		case BeaconState_Init:
			BeaconOnTIM=BeaconOnTime;
		  State=BeaconState_ONStrobe;
      return 1;
		//�ȴ��׶�
		case BeaconState_ONStrobe:
			BeaconOnTIM--;
			if(BeaconOnTIM>0)return 1;
	    //����ʱ�䵽��Ϩ��
		  BeaconOffTIM=BeaconOFFTime*8;
		  State=BeaconState_OFFWait;
		  break;
		//�ȴ�Ϩ��׶ν���
		case BeaconState_OFFWait:
		  if(!BeaconOffTIM)State=BeaconState_Init;
		  break;
		}
	//����״̬����0ʹLEDϨ��
	return 0;
	}
