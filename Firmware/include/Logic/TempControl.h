#ifndef _TC_
#define _TC_

//函数
int ThermalILIMCalc(void); //根据温控模块计算电流限制
void ThermalMgmtProcess(void); //温控管理函数
void RecalcPILoop(int LastCurrent); //换挡的时候重新计算PI环路
void ThermalPILoopCalc(void); //温控PI环路的计算

//外部Flag
extern bit IsPauseStepDownCalc; //是否暂停温控的计算流程（该bit=1不会强制复位整个温控系统，但是会暂停计算）
extern bit IsDisableTurbo; //关闭极亮进入
extern bit IsForceLeaveTurbo; //强制退出极亮

#endif
