#ifndef _SideKey_
#define _SideKey_

typedef enum
{
HoldEvent_None=0,
HoldEvent_H=1, //长按
HoldEvent_1H=2, //单击+长按
HoldEvent_2H=3, //双击+长按
HoldEvent_3H=4, //三击+长按
HoldEvent_4H=5, //四击+长按
HoldEvent_5H=6 //5击+长按
}HoldEventDef;

//获取按键事件的函数
bit getSideKeyHoldEvent(void);						//获得侧按按钮一直按住的事件
bit IsKeyEventOccurred(void); 						//检测是否有任意的事件发生
bit getSideKey1HEvent(void); 							//获取侧按按键是否有单击+长按事件
bit getSideKeyLongPressEvent(void); 			//获取侧按按键长按2秒事件
char getSideKeyNClickAndHoldEvent(void); 	//获取侧按按下N次+长按的按键数
char getSideKeyShortPressCount(void);			//获取侧按按键的连击（包括单击）按键次数

//函数(初始化和其余功能)
void SideKeyInit(void);							//初始化按键控制器
bit GetSideKeyRawGPIOState(void); 	//获取侧部按键的GPIO实时状态（没有任何去抖）
void SideKey_SetIntOFF(void);				//关闭侧按的GPIO中断
void MarkAsKeyPressed(void); 				//标记按键按下
void ClearShortPressEvent(void); 		//清除累计的短按事
char GetIfSideKeyTriggerInt(void); 	//获取侧按是否触发中断

//回调处理
void SideKey_TIM_Callback(void);//连按检测计时的回调处理
void SideKey_LogicHandler(void);//逻辑处理

#endif
