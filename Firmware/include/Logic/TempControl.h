#ifndef _TC_
#define _TC_

//PI环参数和最小电流限制
#define ILEDRecoveryTime 120 //使用积分器缓慢升档的判断时长，如果积分器持续累加到这个时长，则执行一次调节(单位秒)
#define SlowStepDownTime 60 //使用积分器缓慢降档的判断时长，如果积分器持续累加到这个时长，则执行一次调节(单位秒)
#define IntegralCurrentTrimValue 2000 //积分器针对输出的电流修调的最大值(mA)
#define IntegralFactor 16 //积分系数(每单位=1/8秒，越大时间常数越高，6=每分钟进行40mA的调整)
#define MinumumILED 390 //降档系统所能达到的最低电流(mA)

//常亮电流配置
#define ILEDConstant 750 //降档系统内温控的常亮电流设置(mA)
#define ILEDConstantFoldback 500 //在接近温度极限时的降档系统内的常亮电流设置(mA)

//温度配置
#define ForceOffTemp 65 //过热关机温度
#define ForceDisableTurboTemp 50 //超过此温度无法进入极亮
#define ConstantTemperature 46 //非极亮挡位温控启动后维持的温度
#define ReleaseTemperature 35 //温控释放的温度

/*   积分器满量程自动定义，切勿修改！    */
#define IntegrateFullScale IntegralCurrentTrimValue*IntegralFactor

#if (IntegrateFullScale > 32000)

#error "Error 001:Invalid Integral Configuration,Trim Value or time-factor out of range!"

#endif

#if (IntegrateFullScale <= 0)

#error "Error 002:Invalid Integral Configuration,Trim Value or time-factor must not be zero or less than zero!"

#endif

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
