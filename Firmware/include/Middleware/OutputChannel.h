#ifndef _OCH_
#define _OCH_

//输出欠压参数配置
#define BoostChipUVLO 2.45f     //驱动内部boost芯片所能维持运行的最小UVLO电压(V)，低于此电压后系统强制关闭

//外部参考
extern xdata int Current; //电流值
extern xdata int CurrentBuf; //存储当前已经上传的电流值 

//函数
bit GetIfOutputEnabled(void);
void OutputChannel_Init(void);
void OutputChannel_Calc(void);
void OutputChannel_TestRun(void);

#endif
