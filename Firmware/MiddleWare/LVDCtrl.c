/****************************************************************************/
/** \file LVDCtrl.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件负责根据需要配置系统的WUT唤醒定时器，在系统进入睡眠后周
期性踢醒系统利用ADC进行电池电压采样并根据需要关闭指示灯。同时该WUT计时器还实现了
系统在长时间放置后自动进入锁定模式避免小孩手贱引发事故的功能。

**	History: 
/** \Date 2025/10/16 16:00
/** \Desc 修改WUT计时器的运行周期至8000mS唤醒一次。并新增禁止重复尝试打开计时器的
处理避免WUT配置后被二次尝试启用导致配置异常。
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

//WUT分频比
#define WUT_DIV_1 0x00
#define WUT_DIV_8 0x10
#define WUT_DIV_32 0x20
#define WUT_DIV_256 0x30

//WUT使能mask
#define WUT_EN_Mask 0x80

//唤醒计时器的目标分频比参数和时间
#define WUT_Div_Ratio WUT_DIV_256
#define WUT_Time_ms 8000ul           //使用256分频比实现8000mS计时

//自动计算WUT计数器数值(该部分不允许修改！)
#if (WUT_Div_Ratio == WUT_DIV_1)
  #define WUTDIV 1ul
#elif (WUT_Div_Ratio == WUT_DIV_8)
  #define WUTDIV 8ul
#elif (WUT_Div_Ratio == WUT_DIV_32)
  #define WUTDIV 32ul
#elif (WUT_Div_Ratio == WUT_DIV_256)
  #define WUTDIV 256ul
#else
  #define WUTDIV 1ul //默认指定一个1UL避免系统报错
  #error "You Should Tell the System which WUT division factor you want to use!!!"
#endif

#define WUT_Count_Val ((WUT_Time_ms*1000ul)/(8ul*WUTDIV))

#if (WUT_Count_Val > 0xFFF)
  //检测WUT计数值是否超出允许值
	#error "WUT Timer Counter overflow detected!you need to reconfigure the time value."
#endif

#if (WUT_Count_Val == 0)
  //检测WUT计数值是否=0,如果等于0则禁止编译通过（会导致WUT运行异常）
	#error "WUT Timer Counter value is equal to zero,this will cause the timer to stop working!you need to reconfigure the time value."
#endif

#if (WUT_Count_Val < 0)
  //检测WUT计数值是否=0,如果小于0则禁止编译通过（会导致WUT运行异常）
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

//启动循环唤醒系统的低电压检测模块
void LVD_Start(void)
	{	
	//WUT已开启，不允许该函数执行	
	if(WUTCRH&WUT_EN_Mask)return;
  //配置WUT唤醒时间
	WUTCRL=WUT_Count_Val&0xFF;
	WUTCRH=(WUT_Count_Val>>8)&0xFF;
	//配置WUT分频系数(应用mask)
	WUTCRH|=WUT_Div_Ratio; 
	//启动WUT
	WUTCRH|=WUT_EN_Mask;
	}

//关闭循环唤醒模块的低电压检测
void LVD_Disable(void)
	{
	WUTCRH=0;
	WUTCRL=0;
	}
