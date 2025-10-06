/****************************************************************************/
/** \file PWM.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件负责实现系统对外输出PWM的配置以实现PWMDAC功能控制激光器
的电流和亮度

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "PinDefs.h"
#include "GPIO.h"
#include "PWMCfg.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/
#define PWM_Enable() 	PWMFBKC=0x00;PWMCNTE=0x01 //使能通道0的计数器，PWM开始运行(PWM使能操作)
#define SysFreq 48000000 //系统时钟频率(单位Hz)
#define PWMFreq 6000 //PWM频率(单位Hz)	
#define PWMStepConstant (SysFreq/PWMFreq)-1 //PWM周期自动定义

#if (PWMStepConstant > 0xFFFE)
  //自动检测PWM的数值是否合法
	#error "PWM Frequency is too low which causing PWM Counter to overflow!"
#endif
/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/
xdata float CCDACTargetDuty;	//目标PWM占空比(%)
bit IsNeedToUploadPWM; 				//是否需要更新PWM

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable and SFR definitions('static and sfr')
****************************************************************************/
static bit IsPWMLoading; 				 //PWM正在加载中
static bit IsNeedToEnableOutput; //是否需要启用输出

sbit PWMOut=PWMIOP^PWMIOx;
/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

static void UploadPWMValue(void)	//上传PWM值
	{
	PWMLOADEN=0x01; //加载通道0的PWM值
	while(PWMLOADEN&0x01); //等待加载结束
	}
/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/

void PWM_DeInit(void)	//关闭PWM定时器
	{
	//配置为普通GPIO
	GPIO_SetMUXMode(PWMIOG,PWMIOx,GPIO_AF_GPIO);
	//关闭PWM模块
	PWMCNTCLR=0x01; //复位PWM0的计数器
	PWMOE=0x00;
	PWMCNTE=0x00;		//关闭PWM计数器
	PWM45PSC=0x00;
	PWM01PSC=0x00;  //关闭PWM分频器时钟
	}

void PWM_Init(void)	//PWM定时器初始化
	{
	GPIOCfgDef PWMInitCfg;
	//设置结构体
	PWMInitCfg.Mode=GPIO_Out_PP;
  PWMInitCfg.Slew=GPIO_Fast_Slew;		
	PWMInitCfg.DRVCurrent=GPIO_High_Current; //推FanPWM，不需要很高的上升斜率
	//配置GPIO
  PWMOut=0; 				//PWM的输出在非PWM模式下始终输出0
	GPIO_ConfigGPIOMode(PWMIOG,GPIOMask(PWMIOx),&PWMInitCfg); 
	//配置PWM发生器
	PWMCON=0x00; 		//PWM通道为六通道独立模式，向下计数，关闭非对称计数功能	
	PWMOE=0x01; 		//打开PWM输出通道0
	PWM01PSC=0x01;  //打开预分频器和计数器时钟 
  PWM0DIV=0xff;   //令Fpwmcnt=Fsys=48MHz(不分频)
  PWMPINV=0x00; 	//所有通道均设置为正常输出模式
	PWMCNTM=0x01; 	//通道0和4配置为自动加载模式
	PWMCNTCLR=0x01; //初始化PWM的时候复位通道0和4的定时器
	PWMDTE=0x00; 		//关闭死区时间
	PWMMASKD=0x00; 
	PWMMASKE=0x01; 	//PWM掩码功能启用，默认状态下禁止通道0和4输出
	//配置周期数据
	PWMP0H=(PWMStepConstant>>8)&0xFF;
	PWMP0L=PWMStepConstant&0xFF;	
	//配置占空比数据
  PWMD0H=0;
	PWMD0L=0;	
	//初始化变量
	CCDACTargetDuty=0;
	IsPWMLoading=0; 
	IsNeedToUploadPWM=0;
	//启用PWM
	PWM_Enable();
	UploadPWMValue();	
	//PWM初始化完毕，将引脚启用为复用功能
  GPIO_SetMUXMode(PWMIOG,PWMIOx,GPIO_AF_PWMCH0);
	}

void PWM_OutputCtrlHandler(void)	//根据PWM结构体内的配置进行输出
	{
	int value;
	float buf;
	//当前系统未请求加载
	if(!IsNeedToUploadPWM)return; //不需要加载
	//当次加载已开始，进行结束监测
	else if(IsPWMLoading) 
		{
	  if(PWMLOADEN&0x01)return;//加载寄存器复位为0，表示加载成功
	  //加载结束
		if(IsNeedToEnableOutput)PWMMASKE&=0xFE;
		else PWMMASKE|=0x01;   //更新PWMMASKE寄存器根据输出状态启用对应的通道
		IsNeedToUploadPWM=0;
		IsPWMLoading=0;  //正在加载状态为清除
		}
	//当次加载已被请求开始，进行加载处理
	else
		{
		//PWM占空比参数限制
		if(CCDACTargetDuty>100)CCDACTargetDuty=100;
		if(CCDACTargetDuty<0)CCDACTargetDuty=0;
		//配置寄存器装载CC DAC的PWM设置数值
		buf=CCDACTargetDuty*(float)PWMStepConstant;
		buf/=(float)100;
		value=(int)buf;
		IsNeedToEnableOutput=value?1:0;	//根据算出的寄存器加载值判定是否需要启用输出
		PWMD0H=(value>>8)&0xFF;
		PWMD0L=value&0xFF;			
		//PWM寄存器数值已装入，应用数值		
		IsPWMLoading=1; //标记加载过程进行中
		PWMLOADEN|=0x01; //开始加载
		}
	}
