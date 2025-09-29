#ifndef _ACTBeacon_
#define _ACTBeacon_

//参数
#define ActiveBeaconOFFVolt 2900 //设置当等效单节电池电压欠压后关闭有源夜光避免电池饿死的阈值(mV)

//函数
void ActiveBeacon_LVKill(void);
void ActiveBeacon_Start(void);

#endif