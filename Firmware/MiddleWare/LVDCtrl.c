/****************************************************************************/
/** \file LVDCtrl.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ����������Ҫ����ϵͳ��WUT���Ѷ�ʱ������ϵͳ����˯�ߺ���
��������ϵͳ����ADC���е�ص�ѹ������������Ҫ�ر�ָʾ�ơ�ͬʱ��WUT��ʱ����ʵ����
ϵͳ�ڳ�ʱ����ú��Զ���������ģʽ����С���ּ������¹ʵĹ��ܡ�

**	History: 
/** \Date 2025/10/16 16:00
/** \Desc �޸�WUT��ʱ��������������8000mS����һ�Ρ���������ֹ�ظ����Դ򿪼�ʱ����
�������WUT���ú󱻶��γ������õ��������쳣��
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "LVDCtrl.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//WUT��Ƶ��
#define WUT_DIV_1 0x00
#define WUT_DIV_8 0x10
#define WUT_DIV_32 0x20
#define WUT_DIV_256 0x30

//WUTʹ��mask
#define WUT_EN_Mask 0x80

//���Ѽ�ʱ����Ŀ���Ƶ�Ȳ�����ʱ��
#define WUT_Div_Ratio WUT_DIV_256
#define WUT_Time_ms 8000ul           //ʹ��256��Ƶ��ʵ��8000mS��ʱ

//�Զ�����WUT��������ֵ(�ò��ֲ������޸ģ�)
#if (WUT_Div_Ratio == WUT_DIV_1)
  #define WUTDIV 1ul
#elif (WUT_Div_Ratio == WUT_DIV_8)
  #define WUTDIV 8ul
#elif (WUT_Div_Ratio == WUT_DIV_32)
  #define WUTDIV 32ul
#elif (WUT_Div_Ratio == WUT_DIV_256)
  #define WUTDIV 256ul
#else
  #define WUTDIV 1ul //Ĭ��ָ��һ��1UL����ϵͳ����
  #error "You Should Tell the System which WUT division factor you want to use!!!"
#endif

#define WUT_Count_Val ((WUT_Time_ms*1000ul)/(8ul*WUTDIV))

#if (WUT_Count_Val > 0xFFF)
  //���WUT����ֵ�Ƿ񳬳�����ֵ
	#error "WUT Timer Counter overflow detected!you need to reconfigure the time value."
#endif

#if (WUT_Count_Val == 0)
  //���WUT����ֵ�Ƿ�=0,�������0���ֹ����ͨ�����ᵼ��WUT�����쳣��
	#error "WUT Timer Counter value is equal to zero,this will cause the timer to stop working!you need to reconfigure the time value."
#endif

#if (WUT_Count_Val < 0)
  //���WUT����ֵ�Ƿ�=0,���С��0���ֹ����ͨ�����ᵼ��WUT�����쳣��
	#error "WUT Timer Counter value is less than zero,this will cause the timer to stop working!you need to reconfigure the time value."
#endif


/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

//����ѭ������ϵͳ�ĵ͵�ѹ���ģ��
void LVD_Start(void)
	{	
	//WUT�ѿ�����������ú���ִ��	
	if(WUTCRH&WUT_EN_Mask)return;
  //����WUT����ʱ��
	WUTCRL=WUT_Count_Val&0xFF;
	WUTCRH=(WUT_Count_Val>>8)&0xFF;
	//����WUT��Ƶϵ��(Ӧ��mask)
	WUTCRH|=WUT_Div_Ratio; 
	//����WUT
	WUTCRH|=WUT_EN_Mask;
	}

//�ر�ѭ������ģ��ĵ͵�ѹ���
void LVD_Disable(void)
	{
	WUTCRH=0;
	WUTCRL=0;
	}
