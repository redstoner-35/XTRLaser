#ifndef _BattDs_
#define _BattDs_

//状态变量enum
typedef enum
	{
	Battery_Plenty, //电池电量充足
	Battery_Mid, //电池电量较为充足
	Battery_Low, //电池电量不足
	Battery_VeryLow //电池电量严重不足
	}BattStatusDef;

typedef enum
	{
  BattVdis_Waiting, //等待显示阶段
	BattVdis_PrepareDis, //准备显示
	BattVdis_DelayBeforeDisplay, //延迟一段时间
	BattVdis_Show10V, //显示十位
	BattVdis_Gap10to1V, //十位和个位之间的等待
	BattVdis_Show1V, //显示个位
	BattVdis_Gap1to0_1V, //个位和十分位之间的等待
	BattVdis_Show0_1V, //显示小数点后一位(0.1V)
	BattVdis_WaitShowChargeLvl, //等待一段时间后显示当前电量
	BattVdis_ShowChargeLvl, //显示电池电量的等待
	BattVdis_WaitShowTempState,
	BattVdis_ShowTempState	
	}BattVshowFSMDef; //电池电量显示处理

//外部参考
extern bit IsBatteryAlert; //电池低电警告发生
extern bit IsBatteryFault; //电池低电量故障发生
extern BattStatusDef BattState; //电池状态
extern xdata int CellVoltage; //滤波之后的电池组等效单节电压（不反应电池组在不平衡状态下的实际单节电压）
extern xdata BattVshowFSMDef VshowFSMState; //状态机状态	

//触发报告提示类函数	
void Trigger2SModeEnterInfo(void);//触发进入2S模式的快闪两次提示
void TriggerTShowDisplay(void); //启动温度显示	
void TriggerVshowDisplay(void); //启动电池电压显示	
	
//函数
bit IsKeyFSMCanEnable(void); //内部处理函数，负责在进行上电首次电量播报期间禁止按键状态机响应
void ShowEntered2SModeProc(void); //处理进入2S模式快闪2次的显示流程
void BattDisplayTIM(void); //电池电量显示函数处理
void DisplayVBattAtStart(bit IsPOR); //在启动时显示电池电压
void BatteryTelemHandler(void);  //电池测量和指示灯控制
bit LowPowerStrobe(void); //低电量提示闪
	
#endif
