#include "cms8s6990.h"
#include "LVDCtrl.h"

//�����͵�ѹ���ģ��
void LVD_Start(void)
	{	
  //����WUT����ʱ��=8uS*256*4096=8388mS
	WUTCRL=0xFF;
	WUTCRH=0xBF;
	}

//�رյ͵�ѹ���
void LVD_Disable(void)
	{
	WUTCRH=0;
	WUTCRL=0;
	}