/****************************************************************************/
/** \file ModeControl.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件是顶层应用层文件，负责实现解析按键事件并按照事件执行模式
状态机驱动实现激光手电内所有的挡位切换和操作逻辑。同时该文件实现了根据挡位的特性进
行高级输出电流合成并将电流参数传递给输出通道模块的电流合成操作。

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "LEDMgmt.h"
#include "SideKey.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "SysConfig.h"
#include "ADCCfg.h"
#include "TempControl.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "ModeControl.h"
#include "VersionCheck.h"
#include "SOS.h"
#include "BreathMode.h"
#include "Beacon.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

#define LockLowPowerIndTimeOut 20   //锁定模式下临时点亮的超时时间(秒)
#define BurnModeTimeOut 60 				  //烧灼模式无操作自动退出的时间(秒)
#define RampAdjustDividingFactor 6  //无极调光模式下控制调光速度的分频比例，越大则调光速度越慢
#define HoldSwitchDelay 6 				  // 长按换挡延迟(1单位=0.125秒)

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

//全局变量(挡位)
ModeStrDef *CurrentMode; //挡位结构体指针
xdata ModeIdxDef LastMode; //挡位记忆存储
xdata ModeIdxDef LastModeBeforeTurbo; //上一个进入极亮的挡位
xdata SysConfigDef SysCfg; //系统配置	

//全局变量(状态位)
bit IsSystemLocked;		 //系统是否已锁定
bit IsEnableIdleLED;	 //是否开启待机提示
bit IsBurnMode;        //是否进入到了烧灼模式	
bit IsEnable2SMode;    //是否开启双锂模式	
	
//全局软件计时变量
xdata unsigned char HoldChangeGearTIM; //挡位模式下长按换挡
xdata unsigned char DisplayLockedTIM; //锁定和战术模式进入退出显示

/****************************************************************************/
/*	Local/global constant definitions('const')
****************************************************************************/

//挡位结构体
code ModeStrDef ModeSettings[ModeTotalDepth]=
	{
		//关机状态
    {
		Mode_OFF,
		0,
		0,  //电流0mA
		0,  //关机状态阈值为0强制解除警报
		true,
		false,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_OFF,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 
		//出错了
		{
		Mode_Fault,
		0,
		0,  //电流0mA
		0,
		false,
		false,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_OFF,							 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
	  //无极调光		
		{
		Mode_Ramp,
		3000,  //最大 3A电流
		400,   //最小 0.4A电流
		3200,  //3.2V关断
		false, //不能带记忆  
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Ramp,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
    //低亮
		{
		Mode_Low,
		600,  //0.6A电流
		0,   //最小电流没用到，无视
		2850,  //2.85V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Low,				 					//低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Mid,
		Mode_OFF		//模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
    //中亮
		{
		Mode_Mid,
		1200,  //1.2A电流
		0,   //最小电流没用到，无视
		3000,  //3V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Low,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_MHigh,
		Mode_Low	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //中高亮
		{
		Mode_MHigh,
		SingleCellModeICCMAX,  //该挡位电流值根据LD的类型自动配置
		0,   //最小电流没用到，无视
		3100,  //3.1V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Mid,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_High,
		Mode_Mid	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //高亮
		{
		Mode_High,
		2700,  //2.7A电流
		0,   //最小电流没用到，无视
		3200,  //3.2V关断
		true,
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_MHigh,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_Jump,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Low,
		Mode_MHigh	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		}, 	
    //极亮
		{
		Mode_Turbo,
		DualCellTurboCurrent,  //使用系统根据LD类型自动选择的极亮电流
		0,   //最小电流没用到，无视
		3350,  //3.35V关断
		false, //极亮不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Turbo,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_High	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
	  //SOS求救挡位
		{
		Mode_SOS,
		800,  //0.8A电流	
		0,   //最小电流没用到，无视
		2850,  //2.85V关断
		false, //特殊挡位不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_SOS,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Breath,
		Mode_Beacon	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
		//拿来调试光学的对焦挡位
		{
		Mode_Focus,
		200,  //0.2A电流	
		0,   //最小电流没用到，无视
		2850,  //2.85V关断
		false, //特殊挡位不能带记忆
		false,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Focus,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_Low,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)		
		},
		//拿来烧东西的点动模式
		{
		Mode_Burn,
		DualCellTurboCurrent,  //使用系统根据LD类型自动选择的极亮电流
		200,   //在按键松开状态下200mA
		3350,  //3.35V关断
		false, //特殊挡位不能带记忆
		true,
		//配置是否允许进入爆闪
		true,
		//低电量保护设置
		Mode_Focus,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)		
		},
	  //呼吸信标闪挡位
		{
		Mode_Breath,
		3100,  //3.1A电流	
		160,   //呼吸模式最低160mA
		3000,  //3V关断
		false, //特殊挡位不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Breath,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,   //低电量保护机制的类型
		//挡位切换设置
		Mode_Beacon,
		Mode_SOS	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
		//定期快闪信标闪
		{
		Mode_Beacon,
		3100,  //3.1A电流	
		0,   	 //最小电流没用到，无视
		3000,  //3V关断
		false, //特殊挡位不能带记忆
		true,
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_Beacon,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Enable_OFF,   //低电量保护机制的类型
		//挡位切换设置
		Mode_SOS,
		Mode_Breath	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},
		//无电量保护机制的紧急SOS模式
		{
		Mode_SOS_NoProt,
		550,  //0.55A电流	
		0,   //最小电流没用到，无视
		2850,  //2.85V关断
		false, //特殊挡位不能带记忆
		false, //该挡位间歇工作且温度极低，不需要温控也能安全使用
		//配置是否允许进入爆闪
		false,
		//低电量保护设置
		Mode_SOS_NoProt,				 //低电量触发保护之后，如果不执行关机则自动跳转的挡位
		LVPROT_Disable,        //低电量保护机制的类型
		//挡位切换设置
		Mode_OFF,
		Mode_OFF	 //模式挡位切换设置，长按和单击+长按切换到的目标挡位(输入OFF表示不进行切换)
		},		
	};
/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata unsigned char LockINDTimer;  //锁定条件下临时使能激光的计时器
static xdata unsigned int BurnModeTimer;  //点按超时计时器
static xdata unsigned char RampDIVCNT; //无极调光降低调光速度的分频计时器		
static bit IsRampKeyPressed;  //标志位，用户是否按下按键对无极调光进行调节
static bit IsNotifyMaxRampLimitReached; //标记无极调光达到最大电流	
static bit RampEnteredStillHold;    //无极调光进入后按键仍然按住
static bit IsSingleCellEnterTurbo;  //是否1S电池进入极亮
static bit IsDisplayLocked;         //是否开启锁定指示	
	
/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/

//查询挡位的目标电池电压
static int QueryModeRequiredBattVolt(ModeIdxDef TargetMode)	
	{
		unsigned char i;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==TargetMode)
		{
		//找到目标挡位了，返回保护电压值
		return ModeSettings[i].LowVoltThres;
		}
	//整个挡位存储区域找遍了都没有，返回0
	return 0;
	}	
	
//无极调光处理
static void RampAdjHandler(void)
	{	
  int Limit;
	bit IsPress;
  //计算出无极调光上限
	IsPress=getSideKey1HEvent()|getSideKeyHoldEvent();
	Limit=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX;                    				 //双锂模式限制电流最大不能超过1.8A
	Limit=SysCfg.RampCurrentLimit<Limit?SysCfg.RampCurrentLimit:Limit;
	if(Limit<QueryCurrentGearILED()&&IsPress&&SysCfg.RampCurrent>Limit)SysCfg.RampCurrent=Limit; //在电流被限制的情况下用户按下按键尝试调整电流，立即限幅
	//进行亮度调整
	if(getSideKeyHoldEvent()&&!IsRampKeyPressed) //长按增加电流
			{	
			if(RampDIVCNT)RampDIVCNT--;
			else 
				{
				//时间到，开始增加电流
				if(SysCfg.RampCurrent<Limit)SysCfg.RampCurrent++;
				else
					{
					IsNotifyMaxRampLimitReached=1; //标记已达到上限
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到上限
					SysCfg.RampCurrent=Limit; //限制电流最大值	
					IsRampKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=RampAdjustDividingFactor;
				}
			}	
	else if(getSideKey1HEvent()&&!IsRampKeyPressed) //单击+长按减少电流
		 {
			if(RampDIVCNT)RampDIVCNT--;
			else
				{
				if(SysCfg.RampCurrent>CurrentMode->MinCurrent)SysCfg.RampCurrent--; //减少电流	
				else
					{
					IsNotifyMaxRampLimitReached=0;
					SysCfg.RampLimitReachDisplayTIM=4; //熄灭0.5秒指示已经到下限
					SysCfg.RampCurrent=CurrentMode->MinCurrent; //限制电流最小值
					IsRampKeyPressed=1;
					}
				//计时时间到，复位变量
				RampDIVCNT=RampAdjustDividingFactor;
				}
		 }
  else if(!IsPress&&IsRampKeyPressed)
		{
	  IsRampKeyPressed=0; //用户放开按键，允许调节		
		RampDIVCNT=RampAdjustDividingFactor; //复位分频计时器
		}
	//进行数据保存的判断
	if(IsPress)SysCfg.CfgSavedTIM=32; //按键按下说明正在调整，复位计时器
	else if(SysCfg.CfgSavedTIM==1)
		{
		SysCfg.CfgSavedTIM--;
		SaveSysConfig(0);  //一段时间内没操作说明已经调节完毕，保存数据
		}
	}
//进行关机和开机状态执行N击+长按事件处理的函数
static void ProcessNClickAndHoldHandler(void)
	{
  //正常执行处理
	switch(getSideKeyNClickAndHoldEvent())
		{
		case 1:	//单击+长按进入对焦专用挡位
			if(CurrentMode->ModeIdx!=Mode_OFF)break;
			SwitchToGear(Mode_Focus);
			break; 
		case 2:TriggerVshowDisplay();break; //双击+长按查询电量
		case 3:TriggerTShowDisplay();break;//三击+长按查询温度
		case 4:
			  //关机状态下四击+长按开启无极调光并强制锁到最低电流
			  if(CurrentMode->ModeIdx!=Mode_OFF)break;
				if(CellVoltage>2850)
					{
					SwitchToGear(Mode_Ramp);
					RampRestoreLVProtToMax();
					RampEnteredStillHold=1;
					}
				else LEDMode=LED_RedBlinkFifth;	//手电处于关机状态下且电池电量不足，闪烁五次提示进不去	
		    break;
		case 5:
			  //五击+长按进入无任何电量保护机制（除了低于Boost芯片的UVLO后系统会关闭之外）的应急SOS模式
		    if(CurrentMode->ModeIdx!=Mode_OFF)break;

				if(Data.RawBattVolt<BoostChipUVLO)LEDMode=LED_RedBlinkFifth;
		    else SwitchToGear(Mode_SOS_NoProt);
		    break;
		//其余情况什么都不做
		default:break;			
		}
	}	
	
//开启到普通模式
static void PowerToNormalMode(ModeIdxDef Mode)
	{
  ModeIdxDef ModeBuf=Mode;
	do
		{
		if(CellVoltage>QueryModeRequiredBattVolt(ModeBuf)+50)
			{
			//当前电池电压支持运行到该挡位，切换到该挡位并退出
			SwitchToGear(ModeBuf);
			return;
			}
		}
  while(ModeBuf>2);		//从高亮开始，反复往下匹配寻找可以开启的挡位

	//找遍了所有挡位都没找到合适的，提示电量异常，如果系统处于开启状态则立即关闭
	if(CurrentMode->ModeIdx==Mode_OFF)LEDMode=LED_RedBlinkFifth;	
	else ReturnToOFFState();	 
	}
	
//尝试进入极亮的处理
static void TryEnterTurboProcess(char Count)	
	{
	//非双击模式或者系统锁定，退出
  if(Count!=2||IsSystemLocked)return;
	//未开启2S模式，双击切换到1.8A挡位并使能标记位暂时禁用挡位记忆实现伪装的极亮
  if(!IsEnable2SMode)
			 {
		   PowerToNormalMode(Mode_MHigh);
			 if(CurrentMode->ModeIdx==Mode_MHigh)IsSingleCellEnterTurbo=1;
			 }
	//电池电量充足且没有触发关闭极亮的保护，正常开启
	else if(CellVoltage>QueryModeRequiredBattVolt(Mode_Turbo)+50&&!IsDisableTurbo)
			{
			if(CurrentMode->ModeIdx>1)LastModeBeforeTurbo=CurrentMode->ModeIdx; //存下进入极亮之前的挡位
			if(LastMode>2&&LastMode<7)LastMode=CurrentMode->ModeIdx; //离开循环档的时候，更新循环挡位的主记忆
		  SwitchToGear(Mode_Turbo); 
			}
	//电池电池电量不足或者极亮被锁定尝试开到高亮去
	else PowerToNormalMode(Mode_High);	
	}
	
	
//进行模式状态机的表驱动模块处理	
static void ModeSwitchFSMTableDriver(char ClickCount)
	{
	if(CurrentMode->IsEnterTurboStrobe)TryEnterTurboProcess(ClickCount);//读取当前的模式结构体，执行进入极亮或者爆闪的检测	
  if(IsLargerThanOneU8(CurrentMode->ModeIdx)) //大于1的比较									
		{
		//系统在开机状态，且标志位无效之后则执行电量显示启动检测
		ProcessNClickAndHoldHandler();		
		//侧按单击关机
		if(ClickCount==1)ReturnToOFFState();
		//系统在关闭状态下电池电压低于boost芯片的UVLO值后系统就无法工作了，关机
		else if(!GetIfOutputEnabled()&&Data.RawBattVolt<BoostChipUVLO)ReturnToOFFState();		
		}
 	if(HoldChangeGearTIM&0x80)	 
		{
		//当挡位数据库内的状态表使能长按换挡功能且条件满足时，执行顺向换挡
		HoldChangeGearTIM&=0x7F;
    if(!IsEnable2SMode&&CurrentMode->ModeTargetWhenH==Mode_High)SwitchToGear(Mode_Low);       //非双锂电模式如果检测到顺向换挡试图换到高挡位，则换到低档构成循环			
		else if(CurrentMode->ModeTargetWhenH!=Mode_OFF)
			{
		  //正常执行顺向换挡，如果电池电压高于目标要换的挡位则跳过去，否则立即跳到最低构成循环
		  if(CellVoltage>QueryModeRequiredBattVolt(CurrentMode->ModeTargetWhenH)+50)SwitchToGear(CurrentMode->ModeTargetWhenH);	
			else SwitchToGear(Mode_Low);
			}
		}
	
	if(HoldChangeGearTIM&0x20)  
		{
		//当挡位数据库内的状态表使能单击+长按换挡功能且条件满足时，执行逆向换挡
		HoldChangeGearTIM&=0xDF; 
		if(CurrentMode->ModeTargetWhen1H!=Mode_OFF)SwitchToGear(CurrentMode->ModeTargetWhen1H); 
		}
	
	if(CurrentMode->LVConfig)BatteryLowAlertProcess(CurrentMode->LVConfig&0x02,CurrentMode->ModeWhenLVAutoFall); //执行低电量处理
	}	

/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/

//输入指定的Index，从index里面找到目标模式结构体并返回指针
ModeStrDef *FindTargetMode(ModeIdxDef Mode,bool *IsResultOK)
	{
	unsigned char i;
	*IsResultOK=false;
	for(i=0;i<ModeTotalDepth;i++)if(ModeSettings[i].ModeIdx==Mode)
		{
		*IsResultOK=true;
		break;
		}
	//返回对应的index
	return &ModeSettings[i];
	}

//初始化模式状态机
void ModeFSMInit(void)
	{
	bool Result;
  //复位故障码和挡位模式配置系统
	LastMode=Mode_Low;
	LastModeBeforeTurbo=Mode_Low;
	ErrCode=Fault_None; 					//没有故障
	//初始化无极调光
	SysCfg.RampLimitReachDisplayTIM=0;
  ReadSysConfig(); //从EEPROM内读取无极调光配置
	
	CurrentMode=FindTargetMode(Mode_Ramp,&Result);//遍历挡位设置结构体寻找无极调光的挡位并读取配置
	if(Result)
		{
		SysCfg.RampBattThres=CurrentMode->LowVoltThres; //低压检测上限恢复
		SysCfg.RampCurrentLimit=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX;                   			//找到挡位数据中无极调光的挡位，电流上限恢复
		if(SysCfg.RampCurrent<CurrentMode->MinCurrent)SysCfg.RampCurrent=CurrentMode->MinCurrent;
		if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)SysCfg.RampCurrent=SysCfg.RampCurrentLimit;		//读取数据结束后，检查读入的数据是否合法，不合法就直接修正
		CurrentMode=&ModeSettings[0]; 					//记忆重置为第一个档
		}
	//无法找到无极调光数值，挡位数据损毁，报错
  else ReportError(Fault_RampConfigError);
	//复位变量和一部分模块
	DisplayLockedTIM=0;
	IsBurnMode=0;
	IsPauseStepDownCalc=0;                    //每次初始化clear掉暂停温控计算的标志位
	IsNotifyMaxRampLimitReached=0;
	RampEnteredStillHold=0;
	IsDisplayLocked=0;
	
	IsSingleCellEnterTurbo=0;
	RampDIVCNT=RampAdjustDividingFactor; 			//复位分频计数器	
	ResetSOSModule(); 
	BeaconFSM_Reset(); 
  BreathFSM_Reset();	                      //复位SOS和呼吸、信标模式状态机
	}	

//挡位状态机所需的软件定时器处理
void ModeFSMTIMHandler(void)
{
	//无极调光相关的定时器
	if(IsLargerThanOneU8(SysCfg.CfgSavedTIM))SysCfg.CfgSavedTIM--;
	if(SysCfg.RampLimitReachDisplayTIM)
		{
		SysCfg.RampLimitReachDisplayTIM--;
		if(!SysCfg.RampLimitReachDisplayTIM)IsNotifyMaxRampLimitReached=0;
		}
	//锁定模式下临时点动激光的超时计时器（防止激光一直按下）	
	if(LockINDTimer)LockINDTimer--;
	//烧灼模式超时计时器
	if(BurnModeTimer)BurnModeTimer--;
	//锁定操作提示计时器
  if(DisplayLockedTIM)DisplayLockedTIM--;
}

//挡位跳转
void SwitchToGear(ModeIdxDef TargetMode)
	{
	bool IsLastModeNeedStepDown,Result;
	ModeStrDef *ModeBuf;
	//当前挡位已经是目标值，不执行
	if(TargetMode==CurrentMode->ModeIdx)return;
	//记录换档前的结果	
	IsLastModeNeedStepDown=CurrentMode->IsNeedStepDown; //存下是否需要降档
	//开始寻找
	ModeBuf=FindTargetMode(TargetMode,&Result);
	if(!Result)return;                    //找不到对应的挡位，退出
	
	//应用挡位结果并重新计算极亮电流,同时复位特殊挡位状态机
	switch(TargetMode)
		{
		case Mode_SOS_NoProt:
		case Mode_SOS:ResetSOSModule();break;
		case Mode_Breath:BreathFSM_Reset();break;
		case Mode_Beacon:BeaconFSM_Reset();break;
		}
	CurrentMode=ModeBuf;		
	//如果新老挡位都是常亮挡，则重新设置PI环避免电流过调
	if(TargetMode>2&&IsLastModeNeedStepDown)RecalcPILoop(Current); 	
	}

//长按关机函数	
void ReturnToOFFState(void)
	{
	switch(CurrentMode->ModeIdx)
		{
		case Mode_Fault:
		case Mode_OFF:return;  //非法状态，直接打断整个函数的执行

		//其余挡位则执行判断
		default:break;
		}
  //执行挡位记忆并跳回到关机状态
	if(IsSingleCellEnterTurbo)
		{
		//1S模式下双击模拟进入极亮，退出时不覆写挡位记忆
		IsSingleCellEnterTurbo=0;
		SwitchToGear(Mode_OFF);
		}
	else if(CurrentMode->IsModeHasMemory)LastMode=CurrentMode->ModeIdx;
	SwitchToGear(Mode_OFF); 
	}	
	
//长按换挡的间隔命令生成
void HoldSwitchGearCmdHandler(void)
	{
	char buf;
	//当前系统处于特殊挡位状态(烧灼模式长按烧东西和长按换挡冲突)，不执行处理	
	if(CurrentMode->ModeIdx==Mode_Burn)HoldChangeGearTIM=0; 
	//按键松开或者系统处在非正常状态，计时器复位
	else if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())HoldChangeGearTIM=0; 
	//执行换挡程序
	else 
		{
		buf=HoldChangeGearTIM&0x1F; //取出TIM值
		if(!buf&&!(HoldChangeGearTIM&0x40))HoldChangeGearTIM|=getSideKey1HEvent()?0x20:0x80; //令换挡命令位1指示换挡可以继续
		HoldChangeGearTIM&=0xE0; //去除掉原始的TIM值
		if(buf<HoldSwitchDelay&&!(HoldChangeGearTIM&0x40))buf++;
		else buf=0;  //时间到，清零结果
		HoldChangeGearTIM|=buf; //把数值写回去
		}
	}	


//挡位状态机
void ModeSwitchFSM(void)
	{
	char ClickCount;
	ModeIdxDef ModeBeforeFSMSwitch;
	//获取按键状态
	if(!IsKeyFSMCanEnable())return;         //在初次上电阶段如果按键状态机未启用，则跳过函数执行
	ClickCount=getSideKeyShortPressCount();	//读取按键处理函数传过来的参数
		
	//挡位记忆参数检查
	if(LastMode<3||LastMode>6)LastMode=Mode_Low;				//全局常规记忆
		
	//处理FSM的特殊逻辑部分		
  ModeBeforeFSMSwitch=CurrentMode->ModeIdx;		 //存下进入之前的挡位
	if(VChkFSMState==VersionCheck_InAct)switch(ModeBeforeFSMSwitch)	
		{
		//关机状态
		case Mode_OFF:		  
		   //进入锁定模式
       if(IsSystemLocked)
				{
				//五击解锁，绿灯闪三次并且保存状态
				if(ClickCount==5)
					{
					LEDMode=LED_GreenBlinkThird; 
					IsSystemLocked=0;
					if(CellVoltage>2850)DisplayLockedTIM=5;  //电池电压足够时令LD点亮0.5秒指示解锁成功
					SaveSysConfig(0);
					}
				//锁定状态单击+长按，开启低功率的无害指示激光
        else if(getSideKeyNClickAndHoldEvent()==1)
					{
					if(CellVoltage<2900)LockINDTimer=0; //电池电压异常，禁止激光器运行
					else if(!IsDisplayLocked)
						{
						LockINDTimer=8*LockLowPowerIndTimeOut; //加载临时点亮超时计时器
						IsDisplayLocked=1;                  	 //标记激光打开
						}
					}				
				//当前没有单击+长按事件，检测是否有其他事件发生
				else 
					{
					if(IsDisplayLocked)
						{
						//没有单击+长按事件，clear掉标志位并复位计时器
						LockINDTimer=0;
						IsDisplayLocked=0;
						}
				  //有其余按键事件，红色闪五次提示已锁定
					if(IsKeyEventOccurred())LEDMode=LED_RedBlinkFifth;
					}
				//跳过其余处理
				break;
				}
		  
			//系统并未处于按住的状态下，侧按长按开机进入无级调光
			if(!RampEnteredStillHold&&getSideKeyLongPressEvent())
				{
				if(CellVoltage>2850)
					{
					SwitchToGear(Mode_Ramp);
					RampRestoreLVProtToMax();
					RampEnteredStillHold=1;
					}
				else LEDMode=LED_RedBlinkFifth;	//手电处于关机状态下且电池电量不足，闪烁五次提示进不去	
				}				
			//用户从烧灼模式退出，等待用户松开按键才执行进入无极调光模式的操作
			else if(RampEnteredStillHold&&!getSideKeyHoldEvent())RampEnteredStillHold=0;	
				
		  //非特殊模式正常单击开关机，执行一键极亮，爆闪和转换无极调光
			else switch(ClickCount)
				{
				case 1:
					//侧按单击开机，进入循环挡位上一次关闭的模式（仅在开启了记忆的条件下）
					PowerToNormalMode(LastMode);
					break; 
        case 3:
					//关机状态下侧按三击进入SOS
          if(CellVoltage>2750)SwitchToGear(Mode_SOS); //电量正常，进入SOS
				  else LEDMode=LED_RedBlinkFifth;
          break;
        case 4:
          //关机状态且电压充足，系统温度正常时进入烧灼模式
          if(!IsDisableTurbo&&CellVoltage>3350)  
						{
						BurnModeTimer=8*BurnModeTimeOut; //复位烧灼模式计时器
						IsBurnMode=1;                    //标记进入烧灼模式
						SwitchToGear(Mode_Burn);
						}
					//电池电压不足，无法进入烧灼模式
					else LEDMode=LED_RedBlinkFifth;
					break;	
        case 5:
          //已进入锁定状态，不处理
				  if(IsSystemLocked)break;  
			    //关机状态下且系统未锁定，五击进入锁定模式，侧按红色闪三次，主灯点亮1.5秒                
					LEDMode=LED_RedBlinkThird; 
					IsSystemLocked=1;
					SaveSysConfig(0);
				  break;
		    case 6:
					#ifndef USING_LD_NURM11T
					//关机状态下6击，在1S和2S之间切换电池节数
					if(Data.RawBattVolt>4.35)IsEnable2SMode=1; 	//当前安装的电池是2节，始终保持开启2S模式
					else IsEnable2SMode=IsEnable2SMode?0:1; 		//当前安装的电池是1节，允许翻转状态在1S/2S之间切换
					//制造侧部按键闪烁提示用户当前的节数设置
					if(!IsEnable2SMode)MakeFastStrobe(LED_Amber);
					else Trigger2SModeEnterInfo();               //单锂模式黄色闪一次，双锂模式绿色闪两次
					//如果2S模式禁用，则立即对无极调光电流上限进行限幅
				  if(!IsEnable2SMode)
						{
						//1S模式下无极调光电流不能超过1.8A
						if(SysCfg.RampCurrent>SingleCellModeICCMAX)SysCfg.RampCurrent=SingleCellModeICCMAX;
						if(SysCfg.RampCurrentLimit>SingleCellModeICCMAX)SysCfg.RampCurrentLimit=SingleCellModeICCMAX;
						}
					//保存更改后的配置数据
					SaveSysConfig(0);
          #endif						
					break;
				case 7:
					//7击切换有源夜光功能
					IsEnableIdleLED=IsEnableIdleLED?0:1; //翻转状态
					MakeFastStrobe(IsEnableIdleLED?LED_Green:LED_Red);  //快闪提示一次
					SaveSysConfig(0);  //保存数据
				  break;
			  case 8:
					//8击查询固件版本
					VersionCheck_Trigger();
				  break;
				//其余情况什么都不做
        default:break;				
				}		
		  //N击+长按查询电压，温度和安全开机
			ProcessNClickAndHoldHandler();
  		break;
		//出现错误	
		case Mode_Fault:
		  //致命错误，锁死
		  if(IsErrorFatal())break;	 
			//非致命错误状态用户按下按钮清除错误，清除后特殊功能模块会让主灯熄灭
			if(getSideKeyLongPressEvent())ClearError();
		  break;		
    //无极调光状态				
    case Mode_Ramp:
			  if(RampEnteredStillHold)
					{
					SysCfg.RampLimitReachDisplayTIM=0;
					//等待按键放开再处理
					if(!getSideKeyHoldEvent()&&!getSideKey1HEvent())RampEnteredStillHold=0;
					}
				else RampAdjHandler();					    //无极调光处理
		    //执行低电压保护
				RampLowVoltHandler(); 				
		    break;
		//极亮状态
    case Mode_Turbo:
			  if(IsForceLeaveTurbo)PowerToNormalMode(Mode_Low); //温度达到上限值，强制返回到低亮
			  else if(ClickCount==2)
					{
					//双击返回至极亮进入前的挡位（如果是聚焦挡位进入，则跳到低档位）
					if(LastModeBeforeTurbo==Mode_Focus)PowerToNormalMode(Mode_Low);
					else PowerToNormalMode(LastModeBeforeTurbo); 
					LastModeBeforeTurbo=Mode_Low;   //每次使用了极亮进入记忆则复位记忆
					}
		    break;		
    //烧灼模式
    case Mode_Burn:
			  //系统过热达到退出极亮的标准或长时间无操作，系统关闭
        if(IsForceLeaveTurbo||!BurnModeTimer)
					{
					RampEnteredStillHold=1; //标记当前系统处于按下状态，禁止进入无极调光避免用户在按着按键的时候系统因为低电量关闭后误打开无极调光
					ReturnToOFFState();
					}
		    break;
		}
		
	//处理FSM中的表驱动部分
	if(ModeBeforeFSMSwitch==CurrentMode->ModeIdx&&VChkFSMState==VersionCheck_InAct)
		{
		//如果状态机FSM内有操作或者当前处于版本检查状态则跳过表驱动，否则执行表驱动
		ModeSwitchFSMTableDriver(ClickCount); 
		}
	//表驱动事项响应完毕，清除按键状态
	ClearShortPressEvent(); 
	//系统处于单锂模式，强制限制高亮和极亮并返回到低亮		
  if(!IsEnable2SMode&&(CurrentMode->ModeIdx==Mode_High||CurrentMode->ModeIdx==Mode_Turbo))PowerToNormalMode(Mode_Low);
  //非烧灼模式，清除Burn位
	if(IsBurnMode&&CurrentMode->ModeIdx!=Mode_Burn)IsBurnMode=0;
  //应用输出电流
	if(DisplayLockedTIM||(LockINDTimer&&IsDisplayLocked))Current=230; //用户进入或者退出锁定(包括锁定状态下单击+长按开启激光)，用230mA短暂点亮提示一下
	else if(VChkFSMState!=VersionCheck_InAct)Current=VersionCheckFSM()?150:0; //版本提示触发，开始播报
	else if(LowPowerStrobe())Current=30; //触发低压报警，短时间闪烁提示
	else switch(CurrentMode->ModeIdx)
		{
		case Mode_Beacon:
			  //信标模式，电流由状态机设置
		    IsPauseStepDownCalc=1;		//默认系统处于温控计算暂停的状态
				switch(BeaconFSM())
					{
					case 0:Current=0;break; //0表示让电流关闭
					case 2:Current=200;break; //用200mA低亮提示告知用户已进入信标模式
					default:
						IsPauseStepDownCalc=0;     //正常输出的时候温控启动
						Current=IsEnable2SMode?QueryCurrentGearILED():SingleCellModeICCMAX; //其他值调用系统默认电流正常输出
						break;
					} 	
			  break;
		case Mode_Breath:	
			  //呼吸模式，电流由状态机设置
				Current=BreathFSM_Calc();
				IsPauseStepDownCalc=Current>650?0:1;   //呼吸模式下电流大于650mA才进行计算
				break;
		case Mode_Burn:
			  //烧灼模式，按键按下立即使用最高电流，按键松开使用低电流点亮LD进行对焦
			  if(!IsEnable2SMode)Current=getSideKeyHoldEvent()?SingleCellModeICCMAX:CurrentMode->MinCurrent;
				else Current=getSideKeyHoldEvent()?QueryCurrentGearILED():CurrentMode->MinCurrent;
		    //烧灼模式下只有按下按键才打开温控计算
		    if(Current==CurrentMode->MinCurrent)IsPauseStepDownCalc=1; //按键松开，暂停温控计算
				else	
					{
					//按键按下，复位烧灼模式超时计时器并启用温控运算
					BurnModeTimer=8*BurnModeTimeOut;
					IsPauseStepDownCalc=0;
					}
				break; 		
    case Mode_SOS_NoProt:					
		case Mode_SOS:
				//SOS模式电流由状态机控制
			  Current=SOSFSM()?QueryCurrentGearILED():0;
				IsPauseStepDownCalc=!Current?1:0;              //SOS挡位下若灯珠处于熄灭状态，则暂停温控计算
				break; 
		case Mode_Ramp:
			  IsPauseStepDownCalc=0;              //无极调光挡位下计算始终开启
				//无极调光模式取结构体内数据
				if(SysCfg.RampCurrent>SysCfg.RampCurrentLimit)Current=SysCfg.RampCurrentLimit;
				else Current=SysCfg.RampCurrent;	
		    //无极调光模式指示(无极调光模式在抵达上下限后短暂熄灭或者调到25%)
				if(SysCfg.RampLimitReachDisplayTIM)Current=IsNotifyMaxRampLimitReached?Current>>2:30;
		    break;
		
		//其他挡位使用设置值作为目标电流	
		default:
			  if(QueryCurrentGearILED()>1&&Current<450)IsPauseStepDownCalc=1;
			  else IsPauseStepDownCalc=0;              //其余挡位计算始终开启
			  Current=QueryCurrentGearILED();	
		    break;
		}
	//输出通道运算结束后，限制电流值最高不能超过系统的安全限制值
	if(!IsEnable2SMode&&Current>SingleCellModeICCMAX)Current=SingleCellModeICCMAX;
	if(IsEnable2SMode&&Current>DualCellTurboCurrent)Current=DualCellTurboCurrent;
	}
