/****************************************************************************/
/** \file SideKey.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件负责实现系统的电子侧按按键的多模态按键操作的识别并自动
输出对应的按键事件

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "delay.h"
#include "SideKey.h"
#include "GPIO.h"
#include "cms8s6990.h"
#include "PinDefs.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//按键检测延时和按键检测mask(每个单位=0.125秒)
#define LongPressTimeForTac 2 //开启战术模式后的长按按键检测延时(按下时间超过这个数值则判定为长按)
#define LongPressTime 5 //长按按键检测延时(按下时间超过这个数值则判定为长按)
#define ContShortPressWindow 4 //连续多次按下时侧按的检测释抑时间(在该时间以内按下的短按才算入短按次数内)
#define KeyReleaseDetectMask 0xFF //按键按下的监测Mask

//侧按按键的中断向量和按键Flag清除自动定义，不得修改！
#define SideKeyINTFlagMsk (0x01<<SideKeyGPIOx)

#if (SideKeyGPIOG == 0)
	#define SideKeyIRQ P0EI_VECTOR
	#define ClearKeyIntFlag() P0EXTIF=0  
	#define SideKeyIFlagReg P0EXTIF
	#define SideKeyPriorityMsk 0x01
#elif (SideKeyGPIOG == 1)
	#define SideKeyIRQ P1EI_VECTOR
	#define ClearKeyIntFlag() P1EXTIF=0  
	#define SideKeyIFlagReg P1EXTIF
	#define SideKeyPriorityMsk 0x02
#elif (SideKeyGPIOG == 2)
	#define SideKeyIRQ P2EI_VECTOR
	#define ClearKeyIntFlag() P2EXTIF=0  
	#define SideKeyIFlagReg P2EXTIF
	#define SideKeyPriorityMsk 0x04
#elif (SideKeyGPIOG == 3)	
	#define SideKeyIRQ P3EI_VECTOR	
	#define ClearKeyIntFlag() P3EXTIF=0  
	#define SideKeyIFlagReg P3EXTIF
	#define SideKeyPriorityMsk 0x08
#else
	#error "Invalid GPIO Group Number for SideKey GPIO!"
#endif

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/
typedef struct
{
//按键事件结构体定义
char LongPressDetected;	
char ShortPressCount;
char ShortPressEvent;
HoldEventDef HoldStat;
}KeyEventStrDef;

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
sbit KeyPress=SideKeyGPIOP^SideKeyGPIOx; //侧按按键输入
static bit IsKeyPressed; //按键是否按下
static unsigned char KeyTimer[2];//计时器0用于按键按下计时，计时器1用于连按检测计时
static KeyEventStrDef Keyevent; //按键事件
static xdata unsigned char KeyState;	//内部按键检测用的变量
/****************************************************************************/
/*	External function prototypes
****************************************************************************/
void LoadSleepTimer(void);

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

static void ClickAndHoldEventHandler(int PressCount)	//在单击双击三击+长按触发的时候清除单击事件的记录
  {
	KeyTimer[1]=0; //关闭后部检测定时器
	Keyevent.ShortPressEvent=0;
	Keyevent.ShortPressCount=0; //短按次数为0
	Keyevent.LongPressDetected=0;
	//多击+长按
	Keyevent.HoldStat=(HoldEventDef)(PressCount+1);
	}

static void SideKey_Int_Callback(void)		//侧按GPIO中断回调处理
	{
	unsigned char time;
  //开始响应
	if(GPIO_GetExtIntMode(SideKeyGPIOG,SideKeyGPIOx)==GPIO_Int_Rising)
		{
		IsKeyPressed = 0;
		time=KeyTimer[0]&0x7F;//从计时器取出按键按下时间
		KeyTimer[0]=0;//复位并关闭定时器0
		if(Keyevent.LongPressDetected||Keyevent.HoldStat!=HoldEvent_None)//如果已经检测到长按事件则下面什么都不做
		  {
			Keyevent.HoldStat=HoldEvent_None;
			Keyevent.LongPressDetected=0;//清除检测到的表示
	    }
		else if(time<(unsigned char)LongPressTime)//短按事件发生      
			{
		  if(Keyevent.ShortPressCount<8)Keyevent.ShortPressCount++;//累加有效的短按次数
		  KeyTimer[1]=0x80;//启动短按完毕等待统计的计时器
		  }			
		}
	//按键按下，调用标记函数
	else MarkAsKeyPressed();
	//关闭侧按中断
	SideKey_SetIntOFF();
	}

/****************************************************************************/
/*	Function implementation - Hardware interrupts
****************************************************************************/
void Key_IRQHandler(void) interrupt SideKeyIRQ 
  {
	//侧按中断触发，响应中断
	SideKey_Int_Callback();  //进行按键响应
	//按键Flag清除
  ClearKeyIntFlag();
	}
		
/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//获得侧按是否触发中断
char GetIfSideKeyTriggerInt(void)
	{
	if(!KeyPress)return 1;
	return SideKeyIFlagReg&SideKeyINTFlagMsk;
	}

//获取侧按实时GPIO状态
bit GetSideKeyRawGPIOState(void)
	{
	return KeyPress;
	}

//初始化侧按键
void SideKeyInit(void)
  {
	GPIOCfgDef KeyInitCfg;
	//设置结构体
	KeyInitCfg.Mode=GPIO_IPU;
  KeyInitCfg.Slew=GPIO_Slow_Slew;		
	KeyInitCfg.DRVCurrent=GPIO_Low_Current; //配置为上拉输入
	//按键输入初始化
	GPIO_SetMUXMode(SideKeyGPIOG,SideKeyGPIOx,GPIO_AF_GPIO); //配置为GPIO			
  GPIO_ConfigGPIOMode(SideKeyGPIOG,GPIOMask(SideKeyGPIOx),&KeyInitCfg);//按键输入
	GPIO_EnableInt(SideKeyGPIOG,GPIOMask(SideKeyGPIOx)); //使能中断功能
	GPIO_SetExtIntMode(SideKeyGPIOG,SideKeyGPIOx,GPIO_Int_Falling);//设置为下降沿触发
	EIP1|=SideKeyPriorityMsk; //将按键中断设置为高优先级
	//初始化结构体内容和定时器
	LoadSleepTimer();
	KeyState=0xFF;
	KeyTimer[0]=0x00;
	KeyTimer[1]=0x00;
	Keyevent.ShortPressCount=0;
	Keyevent.ShortPressEvent=0;
	Keyevent.HoldStat=HoldEvent_None;
	}

//检测是否有事件发生
bit IsKeyEventOccurred(void)
	{
	if(Keyevent.HoldStat!=HoldEvent_None)return 1;
	if(Keyevent.ShortPressEvent)return 1;
	//什么也没有，退出不处理
	return 0;	
	}	

//侧按按键计时模块
void SideKey_TIM_Callback(void)
  {
	unsigned char buf,i,Time;
	extern bit IsBurnMode;
	//定时器处理（其中0用于短按/长按判断计时，1用于连续短按终止计时）
	for(i=0;i<2;i++)if(KeyTimer[i]&0x80)
		{
		buf=KeyTimer[i]&0x7F;
		if(!i)Time=IsBurnMode?(unsigned char)LongPressTimeForTac:(unsigned char)LongPressTime;
		else Time=(unsigned char)ContShortPressWindow;
		if(buf<Time)buf++;
		KeyTimer[i]&=0x80;
		KeyTimer[i]|=buf; //将数值取出来，加1再写回去
		}
	}

//标记按键按下
void MarkAsKeyPressed(void)
	{	
	//标记按键已被按下
	IsKeyPressed = 1;//标记按键按下
	if(KeyTimer[1]&0x80)KeyTimer[1]=0x80;//复位
	if(!(KeyTimer[0]&0x80))KeyTimer[0]=0x80;//启动计时
	}		
	
//关闭侧按的GPIO中断
void SideKey_SetIntOFF(void)
	{
	//禁止INT0中断
	GPIO_DisableInt(SideKeyGPIOG,GPIOMask(SideKeyGPIOx)); //禁止中断功能
	KeyState=0xAA; //复位检测模块
	}

//侧按键逻辑处理函数
void SideKey_LogicHandler(void)
  {		
	unsigned char buf;
	extern bit IsBurnMode;
	//对按键进行去抖以及重新打开中断的判断
	if(!GPIO_CheckIfIntEnabled(SideKeyGPIOG,GPIOMask(SideKeyGPIOx)))
		{
		//进行按键检查
		KeyState<<=1;
		if(KeyPress)KeyState++;//附加结果
		//重新打开中断
		buf=KeyState&KeyReleaseDetectMask;	
		if(buf==KeyReleaseDetectMask||buf==0x00)
			{
			LoadSleepTimer(); //加载定时器
			ClearKeyIntFlag();//清除按键响应的Flag
			IsKeyPressed=buf==KeyReleaseDetectMask?0:1; //更新按键状态	
			GPIO_SetExtIntMode(SideKeyGPIOG,SideKeyGPIOx,buf==KeyReleaseDetectMask?GPIO_Int_Falling:GPIO_Int_Rising);//如果当前按键是松开状态则设置为下降沿，否则设置为上升沿
			GPIO_EnableInt(SideKeyGPIOG,GPIOMask(SideKeyGPIOx)); //使能中断功能
			}
		}	
	//如果按键释放等待计时器在计时的话，则重置定时器
  if(IsKeyPressed&&(KeyTimer[1]&0x80))KeyTimer[1]=0x80;
	//长按3秒的时间到
	buf=!IsBurnMode?0x80+(unsigned char)LongPressTime:0x80+(unsigned char)LongPressTimeForTac; //动态计算长按事件结束的时间
	if(IsKeyPressed&&KeyTimer[0]==buf)
		{
    //处理多击+长按事件
    if(Keyevent.ShortPressCount>0)ClickAndHoldEventHandler(Keyevent.ShortPressCount);
		else //长按事件
		  {
			Keyevent.ShortPressCount=0;
			Keyevent.HoldStat=HoldEvent_H;//长按事件发生
	    Keyevent.LongPressDetected=1;//长按检测到了  
			}
		KeyTimer[0]=0;//关闭定时器
		}
	//连续短按序列已经结束
	if(!IsKeyPressed&&KeyTimer[1]==(0x80+(unsigned char)ContShortPressWindow))
	  {
		KeyTimer[1]=0;//关闭定时器1
		if(!Keyevent.LongPressDetected)	
		  Keyevent.ShortPressEvent=1;//如果长按事件已经生效，则松开开关时短按事件不生效
		else 
			Keyevent.LongPressDetected=0; //清除长按检测到的结果
		}
	}
//获取侧按键点按次数的获取函数
char getSideKeyShortPressCount(void)
  {
	//有长按和N击+长按事件，或者短按事件未置起，返回0
	if(Keyevent.HoldStat!=HoldEvent_None||!Keyevent.ShortPressEvent)return 0;
  return Keyevent.ShortPressCount;		
	}

//清除短按事件
void ClearShortPressEvent(void)
	{
	//没有按键事件发生，清除
	if(!Keyevent.ShortPressEvent||!Keyevent.ShortPressCount)return;
	//有事件发生的时候clear事件
	Keyevent.ShortPressEvent=0; 
	Keyevent.ShortPressCount=0;
	}	
//获取侧按按键长按2秒事件的函数
bit getSideKeyLongPressEvent(void)
  {
	if(Keyevent.HoldStat!=HoldEvent_H)return 0;
	else Keyevent.HoldStat=HoldEvent_None;
  return 1;
	}
//获取侧按按键一直按下的函数
bit getSideKeyHoldEvent(void)
  {
	return Keyevent.LongPressDetected?1:0;
	}
//获取侧按按键单击事件
bit getSideKey1HEvent(void)
	{
	return Keyevent.HoldStat==HoldEvent_1H?1:0;
	}
//获取按键单击+长按次数的操作
char getSideKeyNClickAndHoldEvent(void)
	{
	//Enum值是特殊设计的，按键次数=enum值-1
	return (char)(Keyevent.HoldStat)-1>0?(char)(Keyevent.HoldStat)-1:0;
	}
