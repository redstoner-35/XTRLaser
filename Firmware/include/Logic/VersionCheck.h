#ifndef VersionCheck
#define VersionCheck

typedef enum
	{
	VersionCheck_InAct,
	VersionCheck_StartInit,
	VersionCheck_ShowNumber,
	VersionCheck_ShowNumberWait,
	VersionCheck_LoadNextNumber,
	}VersionChkFSMDef;

//�ⲿ�ο�
extern xdata VersionChkFSMDef VChkFSMState;	
	
//����
void VersionCheck_Trigger(void);
char VersionCheckFSM(void);	
	
#endif
