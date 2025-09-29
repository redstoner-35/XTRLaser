#include "cms8s6990.h"
#include "SysReset.h"

//����ϵͳ��λ
#pragma optimize(0)
void TriggerSoftwareReset(void)
	{
	EA=0;
	_nop_();
	TA=0xAA;
  TA=0x55;  //��TA�Ĵ���д55AA����
  WDCON=0x80;  //��WDTCON[7]=1������ϵͳ��λ
	while(1);
	}

//��������λ��־λ
#pragma optimize(0)
void ClearSoftwareResetFlag(void)
	{
	EA=0;
	_nop_();
	TA=0xAA;
  TA=0x55;  //��TA�Ĵ���д55AA����
  WDCON&=0x7F;  //��WDTCON[7]=0������ϵͳ��λ
	}