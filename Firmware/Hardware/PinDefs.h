#ifndef PINDEFS
#define PINDEFS

//内部包含
#include "GPIOCfg.h"

/************************************************************************************
系统未使用IO Map
P0.2-0.5 P1.4 P2.4-2.5 P3.2
************************************************************************************/
#define P0UnusedPIN 0x3C
#define P1UnusedPIN (0x01<<4)
#define P2UnusedPIN 0x30
#define P3UnusedPIN (0x01<<2)

/************************************************************************************
以下是系统的常规GPIO引脚，用于控制外部外设切换功能
************************************************************************************/
#define DCDCENIOP GPIO_PORT_1
#define DCDCENIOG 1
#define DCDCENIOx GPIO_PIN_3 //LD的DCDC电源使能引脚(P1.3)

/************************************************************************************
以下是系统的PWM输出引脚，用于对外输出PWM控制风扇速度，DCDC输出电压等等
************************************************************************************/
#define PWMIOP GPIO_PORT_2
#define PWMIOG 2
#define PWMIOx GPIO_PIN_3  //负责控制恒流基准DAC的输出引脚（P2.3）

/************************************************************************************
以下是系统的模拟输入引脚，例如电池电压测量等
************************************************************************************/
#define NTCInputIOG 2
#define NTCInputIOx GPIO_PIN_2 
#define NTCInputAIN 8						//NTC输入(P2.2,AN8)

#define VOUTFBIOG 0
#define VOUTFBIOx GPIO_PIN_0
#define VOUTFBAIN 0						//输出电压反馈引脚(P0.0,AN0)

#define VBATInputIOG 3
#define VBATInputIOx GPIO_PIN_1
#define VBATInputAIN 13					//电池电压检测引脚(P3.1,AN11)

/****************** 以下是负责按键小板部分的引脚(指示灯和按键) ********************/
#define SideKeyGPIOP GPIO_PORT_2
#define SideKeyGPIOG 2
#define SideKeyGPIOx GPIO_PIN_6 	//侧按按键(P2.6)


#define RedLEDIOP GPIO_PORT_3
#define RedLEDIOG 3
#define RedLEDIOx GPIO_PIN_0		//红色指示灯(P3.0)	


#define GreenLEDIOP GPIO_PORT_0
#define GreenLEDIOG 0
#define GreenLEDIOx GPIO_PIN_1		//绿色指示灯(P0.1)

#endif
