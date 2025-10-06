/****************************************************************************/
/** \file SysConfig.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ�Ϊ�в��豸�����ļ�������ʵ��ϵͳ����ʧ�����ݵĽṹ���鵵
�洢���ϵ�ʱ�����ݶ��룬ʵ�ַ���ʧ�Դ洢

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "cms8s6990.h"
#include "ModeControl.h"
#include "SysConfig.h"
#include "Flash.h"
#include "SideKey.h"
#include "delay.h"
#include "LEDMgmt.h"
#include "SysReset.h"
#include "OutputChannel.h"
#include "ADCCfg.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

//������Flash����
#define	DataFlashLen 0x3FF  //CMS8S6990��Ƭ������������1KByte��Ѱַ��Χ��0-3FF
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   //���õ�������ϳ���

//�ڲ�bit field�Ĵ洢Mask
#define IsLocked_MSK 0x01  //�Ƿ����� bit1
#define IsEnableIdleLED_MSK 0x02 //�Ƿ�����Դҹ�� bit2
#define IsEnable2SMode_MSK 0x04  //�Ƿ���2Sģʽ bit3

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

//�洢��������
typedef struct
	{
	int RampCurrent;
  unsigned char BitfieldMem1;
	}SysStorDef;
	
typedef union
	{
	SysStorDef Data;
	char ByteBuf[sizeof(SysStorDef)];
	}SysDataUnion;

typedef struct
	{
	SysDataUnion SysConfig;
	char CheckSum;
	}SysROMImageDef;

typedef union
	{
	SysROMImageDef Data;
	char ByteBuf[sizeof(SysROMImageDef)];
	}SysROMImg;

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/
static xdata unsigned int CurrentIdx;
static xdata u8 CurrentCRC;

/****************************************************************************/
/*	Function implementation - local('static')
****************************************************************************/
static u8 PEC8Check(char *DIN,char Len)	//CRC-8���� 
	{
	unsigned char crcbuf=0xFF;
	unsigned char i;
	do
		{
		//��������
		crcbuf^=*DIN++;
		//����
		i=8;
			do
			{
			if(crcbuf&0x80)crcbuf=(crcbuf<<1)^0x07;//���λΪ1������֮��Ͷ���ʽXOR
			else crcbuf<<=1;//���λΪ0��ֻ��λ��XOR
			}
		while(--i);
		}
	while(--Len);
	//������
	return crcbuf;
	}

//��EEPROM��Ѱ������һ��ϵͳ����
static int SearchSysConfig(SysROMImg *ROMData)	
	{
	unsigned char i;
	int Len=0;
	//����flash����ʼ��ȡ
	SetFlashState(1);
	do
		{		
		for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]); //��ROM�ڶ�ȡ����
		if(ROMData->Data.CheckSum!=PEC8Check(ROMData->Data.SysConfig.ByteBuf,sizeof(SysStorDef)))break; //�ҵ���û�б�д��CRCУ�鲻���ĵط�����������
		Len++;
		}
	while(Len<SysCfgGroupLen);
	//��ȡ��һ����ȷ������
	if(Len>0)Len--;
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Read,i+(Len*sizeof(SysROMImg)),&ROMData->ByteBuf[i]);
	//��ȡ������������һ�������ݵ�index
	return Len;
	}

//׼����ʼ��ϵͳ����
static void PrepareFactoryDefaultCfg(void)
	{
	LoadMinimumRampCurrentToRAM();	
	IsSystemLocked=0;
	IsEnableIdleLED=1;
  //��������ʱ�ĵ�ص�ѹ�����Ƿ���˫�ģʽ
	SystemTelemHandler();
	#ifndef USING_LD_NURM11T
	if(Data.RawBattVolt>4.35)IsEnable2SMode=1;
	else IsEnable2SMode=0;
	#else
	IsEnable2SMode=0;	 //���LDֻ�ܵ��
	#endif
	}	
	
//���Լ���û��������ò���	
static void ResetSysConfigToDefault(void)
	{
	unsigned char delay;
	//���ϵͳ��������״̬�����������ó�������
  if(IsSystemLocked)return;
	//׼����ʼ��ϵͳ����
  PrepareFactoryDefaultCfg();
	//�������ݲ���ʾ״̬
	SaveSysConfig(0); //д����д��Ĭ��ֵ
	SetFlashState(0); //����Flash
	//����ָʾ��׼����ʾ
	delay=100;
	LEDMode=LED_AmberBlinkFast; //LEDģʽ����Ϊ��ɫ����
	do
		{
		delay_ms(10);
		LEDControlHandler();
		//�ɿ�������ʼ��ʱ
		if(GetSideKeyRawGPIOState())delay--;
		}
	while(delay);
	//����ϵͳ����
	TriggerSoftwareReset();
	}	
	
//��ʾϵͳ���ݴ��ڴ���
static void ShowEPROMCorrupted(void)
	{
	unsigned char delay=0xFF;
	//��ȡ������ϣ�����flash	
	SetFlashState(0);
	//����LEDģʽ
	LEDMode=LED_RedBlink; //LEDģʽ����Ϊ��ɫ����
	while(--delay)
		{
		delay_ms(10);
		LEDControlHandler();
		}
	//ʱ�䵽����ϵͳreboot
	TriggerSoftwareReset();
	}
	
/****************************************************************************/
/*	Function implementation - global ('extern')
****************************************************************************/
	
void ReadSysConfig(void)	//��ȡϵͳ����������
	{
	xdata SysROMImg ROMData;
	//��ȡ����
	CurrentIdx=SearchSysConfig(&ROMData);
	//���ж������ݵ�У��
	if(ROMData.Data.CheckSum==PEC8Check(ROMData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)))
		{
		//У��ɹ�����������
		IsEnableIdleLED=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnableIdleLED_MSK?1:0;
		IsEnable2SMode=ROMData.Data.SysConfig.Data.BitfieldMem1&IsEnable2SMode_MSK?1:0;
		IsSystemLocked=ROMData.Data.SysConfig.Data.BitfieldMem1&IsLocked_MSK?1:0;
		SysCfg.RampCurrent=ROMData.Data.SysConfig.Data.RampCurrent;
		//�洢��ǰ��indexֵ
		CurrentCRC=ROMData.Data.CheckSum;
		CurrentIdx++; //��ǰλ�������ݣ���Ҫ��index+1�ƶ���δд���λ��
		
		//�û����°������������ò�����
		if(!GetSideKeyRawGPIOState())ResetSysConfigToDefault();
		}
	//У��ʧ���ؽ�����
	else 
		{
		PrepareFactoryDefaultCfg(); 
		IsSystemLocked=1;  //ϵͳˢд�̼����״��ϵ磬ʹϵͳ��������״̬
		SaveSysConfig(1);  //�ؽ����ݺ������������
		ShowEPROMCorrupted(); //��ʾEEPROM��
		}
	//��ȡ������ϣ�����flash	
	SetFlashState(0);
	}

//�ָ����޼�����ģʽ����͵���
void LoadMinimumRampCurrentToRAM(void)	
	{
	bool Result;
	ModeStrDef *Mode=FindTargetMode(Mode_Ramp,&Result);
	if(Result)SysCfg.RampCurrent=Mode->MinCurrent; //�ҵ���λ�������޼�����ĵ�λ
	else SysCfg.RampCurrent=200; //Ĭ�ϻָ�Ϊ200mA
	}	
	
//�����޼���������
void SaveSysConfig(bit IsForceSave)
	{
	unsigned char i,BFBuf=0;
	xdata SysROMImg SavedData;
	//����flash��CRCУ��ģ����Ҫ��ȡFlash������Ҫ������
	SetFlashState(1);
  //��ʼ�������ݹ���
	if(IsSystemLocked)BFBuf|=IsLocked_MSK;										 //�Ƿ�����
	if(IsEnableIdleLED)BFBuf|=IsEnableIdleLED_MSK;             //�Ƿ�����Դҹ��
	if(IsEnable2SMode)BFBuf|=IsEnable2SMode_MSK;               //�Ƿ���2Sģʽ
		
	SavedData.Data.SysConfig.Data.BitfieldMem1=BFBuf;
	SavedData.Data.SysConfig.Data.RampCurrent=SysCfg.RampCurrent;
	SavedData.Data.CheckSum=PEC8Check(SavedData.Data.SysConfig.ByteBuf,sizeof(SysStorDef)); //����CRC
	//�������ݱȶ�
	if(!IsForceSave&&SavedData.Data.CheckSum==CurrentCRC)
		{
		SetFlashState(0);//��ȡ������ϣ�����flash	
	  return; //�������������������ͬ	
		}
	//������Ҫ���棬��ʼ����Ƿ���Ҫ����
	if(IsForceSave||CurrentIdx>=SysCfgGroupLen) 
		{
		//�����Ѿ�д���ˣ�������0��1������ȫ����
		Flash_Operation(DataFlash_Erase,0x200,&i);  //����2=512-1023
		Flash_Operation(DataFlash_Erase,0,&i);      //����1=0-511
		//�ӵ�0��λ�ÿ�ʼд��
		CurrentIdx=0;
		}
	//д������
	for(i=0;i<sizeof(SysROMImageDef);i++)Flash_Operation(DataFlash_Write,i+(CurrentIdx*sizeof(SysROMImg)),&SavedData.ByteBuf[i]);	
	CurrentIdx++; //��index�ѱ�д�룬���д���¸�idx
	CurrentCRC=SavedData.Data.CheckSum; //���汾��index��CRC8
	SetFlashState(0);//д�������ϣ�����flash	
	}	

