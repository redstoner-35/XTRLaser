#ifndef _OCH_
#define _OCH_

//���Ƿѹ��������
#define BoostChipUVLO 2.45f     //�����ڲ�boostоƬ����ά�����е���СUVLO��ѹ(V)�����ڴ˵�ѹ��ϵͳǿ�ƹر�

//�ⲿ�ο�
extern xdata int Current; //����ֵ
extern xdata int CurrentBuf; //�洢��ǰ�Ѿ��ϴ��ĵ���ֵ 

//����
bit GetIfOutputEnabled(void);
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OutputChannel_TestRun(void);

#endif
