#ifndef _BCON_
#define _BCON_

typedef enum
	{
	BeaconState_Init,
	BeaconState_InfoUser,
	BeaconState_ONStrobe,
	BeaconState_OFFWait,
	}BeaconStateDef;

//����
#define BeaconOnTime 70 //�ű���˸ʱ��
#define BeaconOFFTime 3 //�ű�ر�ʱ��
#define BeaconInfoTime 3 //�ű��ڿ�ʼ֮ǰ������ʾ�û���ʱ��
	
//����
void BeaconFSM_Reset(void); //��λ״̬��
void BeaconFSM_TIMHandler(void); //�ر�ʱ���ʱ
char BeaconFSM(void); //�ű�ģʽ״̬��

#endif
