#ifndef _OCH_
#define _OCH_

//���ͨ����������
#define MainChannelShuntmOhm 10 //��ͨ���ļ���������ֵ(mR)

//PWMDAC��������
#define VdivUpResK 220 //�˷ŷ�ѹ���ֵ��϶˵���(K��)
#define PWMDACResK 10 //PWMDAC�ĵ�����ֵ(K��)
#define VdivDownResK 5.1 //�˷ŷ�ѹ���ֵ��¶˵���(K��)
#define CurrentOffset 99.2 //�ߵ���ͨ���µĵ���ƫ��ֵ(��λ%)

//�ⲿ�ο�
extern xdata int Current; //����ֵ
extern xdata int CurrentBuf; //�洢��ǰ�Ѿ��ϴ��ĵ���ֵ 

//����
bit GetIfOutputEnabled(void);
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OutputChannel_TestRun(void);

#endif
