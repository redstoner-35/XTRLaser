#include "ADCCfg.h"
#include "LEDMgmt.h"
#include "delay.h"
#include "ModeControl.h"
#include "TempControl.h"
#include "BattDisplay.h"
#include "OutputChannel.h"
#include "PWMCfg.h"
#include "LowVoltProt.h"
#include "SelfTest.h"
#include "FastOp.h"

//内部变量
static xdata int TempIntegral;
static xdata int TempProtBuf;
static xdata unsigned char StepUpLockTIM; //计时器

//内部状态位
static bit IsNearThermalFoldBack; //标记位，是否接近于退出极亮温度
static bit IsThermalStepDown; //标记位，是否降档
static bit IsTempLIMActive;  //温控是否已经启动
static bit IsSystemShutDown; //是否触发温控强制关机

//外部状态位
bit IsPauseStepDownCalc; //是否暂停温控的计算流程（该bit=1不会强制复位整个温控系统，但是会暂停计算）
bit IsDisableTurbo;  //禁止再度进入到极亮档
bit IsForceLeaveTurbo; //是否强制离开极亮档

//内部宏定义
#define LeaveTurboTemperature ForceOffTemp-10   //退出极亮温度为关机保护温度-10

//获取系统是否触发降档
bit QueryIsThermalStepDown(void)
	{
	//当前处于电量显示状态不允许打断降档提示
	if(VshowFSMState!=BattVdis_Waiting)return 0; 
	//返回提示标志位
	return IsThermalStepDown;
	}

//换挡的时候根据当前恒温的电流重新PI值
void RecalcPILoop(int LastCurrent)	
	{
	int buf,ModeCur;
	//目标挡位不需要计算,复位比例缓存
	if(!CurrentMode->IsNeedStepDown)TempProtBuf=0;
	//需要复位，执行对应处理
	else
		{	
		//获取当前挡位电流
		ModeCur=QueryCurrentGearILED();
		//计算P值缓存
		buf=TempProtBuf+(TempIntegral/IntegralFactor); //计算电流扣减值
		if(IsNegative16(buf))buf=0; //电流扣减值不能小于0
		buf=LastCurrent-buf; //旧挡位电流减去扣减值得到实际电流(mA)
		TempProtBuf=ModeCur-LastCurrent; //P值缓存等于新挡位的电流-旧挡位实际电流(mA)
		if(IsNegative16(TempProtBuf))TempProtBuf=0; //不允许比例缓存小于0
		}
	//清除积分器缓存
	TempIntegral=0;
	}
	
//输出当前温控的限流值
int ThermalILIMCalc(void)
	{
	int result;
	//判断温控是否需要进行计算
	if(!IsTempLIMActive)
		{
		result=Current; 				//温控被关闭，电流限制进来多少返回去多少
		IsThermalStepDown=0;  	//指示温控已被关闭
		}
	//开始温控计算
	else
		{
		result=TempProtBuf+(TempIntegral/IntegralFactor); //根据缓存计算结果
		if(IsNegative16(result))result=0; //不允许负值出现
		result=Current-result; //计算限流值结果
		if(result<MinumumILED) //已经调到底了，禁止PID继续累加
			{
		  TempProtBuf=Current-MinumumILED; //将比例输出结果限幅为最小电流
		  TempIntegral=0;
		  result=MinumumILED; //电流限制不允许小于最低电流
			}
    //判断温控是否已经触发			
		if(result<(Current-200))IsThermalStepDown=1;	//温控已经让输出电流下调200mA，提示温控触发
		}
	//返回结果	                               
	return result; 
	}

//温控系统中积分追踪温度变化实现恒亮的处理
static void ThermalIntegralHandler(bool IsStepDown,bool IsEnableFastAdj)
	{
	int Buf;
	//条件定义，如果积分值小于上限且系统需要快速调整，则令积分器以和温度挂钩的可变速率工作
	#define IsEnableQuickItg (abs(TempIntegral)<(IntegrateFullScale-Buf)&&IsEnableFastAdj)
	//计算温度差和积分数值
	if(IsStepDown)Buf=Data.Systemp-(LeaveTurboTemperature-8);
	else Buf=(ReleaseTemperature+5)-Data.Systemp; //降档模式下系统温度误差值为强制极亮的温度-8，升档模式为恢复温度+5
	if(IsNegative16(Buf))Buf=0; //温度差不能为负数
	//进行积分器本次调整值的计算
	if(IsEnableQuickItg)Buf<<=1;      //快速调整开启,令调整值=温差*2
	else Buf=0;
	Buf++;  													//这里需要保证Buf始终为1(快速调整被禁用后调整值将会变为0)确保积分器正常响应
  //应用积分数值到积分缓存
	TempIntegral+=(IsStepDown?Buf:-Buf);
	#undef IsEnableQuickItg            //这个宏定义只是该函数的局部定义，需要在函数末尾禁用掉避免后续意外使用到导致问题
	}
	
static void ThermalIntegralCommitToProtHandler(void)	
	{
	//当前积分器累计的参数小于复位时间，退出
	if(abs(TempIntegral)<(ILEDRecoveryTime*8))return;
	//将积分器内累积的变化成比例应用到比例项并清零积分器
	TempProtBuf+=TempIntegral/IntegralFactor;
	TempIntegral=0;						
	}

//温控PI环计算
void ThermalPILoopCalc(void)	
	{
	int ProtFact,Err,ConstantILED;
	bool IsSwitchToITGTrack;
	//PI环关闭，复位数值
	if(!IsTempLIMActive)
		{
		IsNearThermalFoldBack=0;
		TempIntegral=0;
		TempProtBuf=0;
		IsThermalStepDown=0;
		}
	//进行PI环的计算(仅在输出开启且没有暂停的时候进行)
	else if(!IsPauseStepDownCalc&&GetIfOutputEnabled())
		{			
		//获取恒温温度值和恒亮电流
		ConstantILED=ILEDConstant;
		if(IsNearThermalFoldBack)ConstantILED=ILEDConstantFoldback; //接近温度上限，立即使用额外下调的常亮电流
		else ConstantILED=ILEDConstant;
		//温度误差为正（温度大于恒温值）
		if(Data.Systemp>ConstantTemperature)
			{		
			/**************************************************************
			安全保护机制：马上就要摸到强制掉极亮的温度了，立刻使能标志位下
			调常亮电流强制继续使用P项降档快速拉低电流，这样可以避免温度继
			续上去在正常情况下触发退出极亮的保护机制
			**************************************************************/
			if(Data.Systemp>LeaveTurboTemperature-3)IsNearThermalFoldBack=1;
			//比例项(P)
			Err=Data.Systemp-ConstantTemperature;  //误差值等于目标温度-恒温温度
			StepUpLockTIM=24; //升档之后温度过高则之后停止3秒
				
			//极亮挡位高电流下强制时控降档
			if((CurrentMode->ModeIdx==Mode_Turbo||CurrentMode->ModeIdx==Mode_Burn)&&CurrentBuf>2850)
				{
				//迅速降低输出电流避免过热
				TempProtBuf+=(20*Err); 
				}
			//正常执行比例温控
			else if(Err>2)
				{
				//计算比例项	
				if(CurrentMode->ModeIdx==Mode_Turbo||CurrentMode->ModeIdx==Mode_Burn)
					{
					//极亮和烧灼模式用最高斜率增加降档速度，否则使用低一档的斜率
	        ProtFact=CurrentBuf/1000;
					}
				else ProtFact=CurrentBuf/1300;
				//比例项提交
				if(IsNegative16(ProtFact))ProtFact=0;
				ProtFact++; //保证比例项始终有1确保可以正确降档

			  //当前LED电流已被限制到常亮电流范围内，阻止快速降档，否则使用比例项快速降档
				if(CurrentBuf<ConstantILED)ThermalIntegralCommitToProtHandler();
				else 
					{
					//电流没有达到常亮下限，继续提交电流设置
					if(IsLargerThanThreeU16(Err))ProtFact*=(Err+2); 			//温度误差大于3摄氏度，扩张比例系数
				  TempProtBuf+=(ProtFact*Err);													//向buf提交比例项	
					}
				//限制比例项最大只能达到ILEDMIN
				if(TempProtBuf>(Current-MinumumILED))TempProtBuf=(Current-MinumumILED); 
				StepUpLockTIM=60; //触发比例项降档，停7.5秒
				}
			//积分项(I)
			ThermalIntegralHandler(true,CurrentBuf<ConstantILED?true:false); //电流小于常亮值时使能快速调整
			}
		//温度小于恒温值（温度误差为负）
		else if(Data.Systemp<ConstantTemperature)
			{
			//计算误差并判断电流是否进入积分缓调区域
			Err=ConstantTemperature-Data.Systemp;								//误差等于目标温度值减去系统温度
			if(Err>4)
				IsSwitchToITGTrack=true; 	 //温度存在4度以上的负误差说明系统使用暴力风扇快速冷却，允许积分器快速积分，迅速提升常亮
			else if(CurrentBuf>(ConstantILED-200))
				IsSwitchToITGTrack=false;  //当前系统电流已经回升到接近常亮水平，使用积分器每次+1缓慢跟踪
			else
				IsSwitchToITGTrack=true; 	 //当前系统电流距离标定的常亮还很远，允许积分器快速提升到常亮电流
			//比例项(P)
			if(StepUpLockTIM)StepUpLockTIM--; //当前触发降档还没达到快速升档的时间
			else
				{
				//电流达到回升限制值，开始使用积分器监测缓慢回升
				if(IsSwitchToITGTrack)ThermalIntegralCommitToProtHandler();
				//执行比例升温
				else
					{
					if(IsLargerThanOneU16(Err))TempProtBuf-=Err; //进行升档
					if(IsNegative16(TempProtBuf))TempProtBuf=0;
					}			
				//温度下来了很多，系统已经令电流回升到强制降额前的常亮电流，则复位标记位
				if(IsNearThermalFoldBack)
					{
					//当前执行的电流大于额定常亮，复位foldback标记位
					if(CurrentBuf>ILEDConstant)IsNearThermalFoldBack=0;  
					}
				}
			//比例项数值限幅(不能是负数)
			if(IsNegative16(TempProtBuf))TempProtBuf=0; 
			//积分项(I)
			ThermalIntegralHandler(false,IsSwitchToITGTrack); //电流大于常亮值进入积分模式时使能快速调整
			}
		}
	}

//负责温度使能控制的施密特触发器
static bit TempSchmittTrigger(bit ValueIN,char HighThreshold,char LowThreshold)	
	{
	if(Data.Systemp>HighThreshold)return 1;
	if(Data.Systemp<LowThreshold)return 0;
	//数值保持，没有改变
	return ValueIN;
	}

//温度管理函数
void ThermalMgmtProcess(void)
	{
	bit ThermalStatus;
	//温度传感器正常，执行温度控制
	if(Data.IsNTCOK)
		{
		//手电温度过高时对极亮进行限制
		IsForceLeaveTurbo=TempSchmittTrigger(IsForceLeaveTurbo,LeaveTurboTemperature,ForceDisableTurboTemp-10);	//温度距离关机保护的间距不到10度，立即退出极亮
		IsDisableTurbo=TempSchmittTrigger(IsDisableTurbo,ForceDisableTurboTemp,ForceDisableTurboTemp-10); //温度达到关闭极亮档的阈值，关闭极亮
		//过热关机保护
		IsSystemShutDown=TempSchmittTrigger(IsSystemShutDown,ForceOffTemp,ConstantTemperature-5);
		if(IsSystemShutDown)ReportError(Fault_OverHeat); //报故障
		else if(ErrCode==Fault_OverHeat)ClearError(); //消除掉当前错误
		//PI环使能控制
		if(!CurrentMode->IsNeedStepDown)IsTempLIMActive=0; //当前挡位不需要降档
		else //使用施密特函数决定温控是否激活
			{
			ThermalStatus=TempSchmittTrigger(IsTempLIMActive,ConstantTemperature,ReleaseTemperature); //获取施密特触发器的结果
			if(ThermalStatus)IsTempLIMActive=1;//施密特函数要求激活温控，立即激活
			else if(!ThermalStatus&&!TempProtBuf&&IsNegative16(TempIntegral))IsTempLIMActive=0; //施密特函数要求关闭温控，等待比例缓存为0解除限流后关闭
			}
		}
	//温度传感器故障，返回错误
	else ReportError(Fault_NTCFailed);
	}	
