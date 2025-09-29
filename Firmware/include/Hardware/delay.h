#ifndef Delay
#define Delay

//�궨��
//#define EnableMicroSecDelay //�Ƿ�����΢����ʱ
//#define EnableHBCheck //�Ƿ����������
#define UseUnifiedSystemTimeBase   //����ͳһ��ϵͳ��ʼ������

//ϵͳ������ʱ���ܺ�flag
extern volatile bit SysHFBitFlag;
#define DisableSysHBTIM() T2CON=0x00;IE&=~0x20; //����ϵͳ������ʱ����ֱ�ӹرն�ʱ���������ж�

//���������ʱ���Ƿ�����
#ifdef EnableHBCheck
void CheckIfHBTIMIsReady(void);
#endif



//����ϵͳ������ʱ������ʱ��ʱ���ĳ�ʼ��
#ifdef UseUnifiedSystemTimeBase
  
	//ʹ��ͳһ��ʼ������
	void StartSystemTimeBase(void);

#else

	//ʹ�ö�����ʼ������
	void EnableSysHBTIM(void);
	void delay_init();

#endif

//�ϳ�����ʱ
void delay_ms(int ms);
void delay_sec(int sec);

//΢�뼶�����ʱ
#ifdef EnableMicroSecDelay
void delay_us(int us);
#endif

#endif
