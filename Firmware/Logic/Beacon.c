#include "Beacon.h"

//�ڲ���־λ
static xdata BeaconStateDef State;
static xdata unsigned char BeaconOnTIM;
static xdata unsigned char BeaconOffTIM;

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
