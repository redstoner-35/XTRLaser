#ifndef _SysCfg_
#define _SysCfg_

//�ڲ�define
#define IsLocked_MSK 0x01  //�Ƿ����� bit1
#define IsEnableIdleLED_MSK 0x02 //�Ƿ�����Դҹ�� bit2
#define IsEnable2SMode_MSK 0x04  //�Ƿ���2Sģʽ bit3

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

//������Flash����
#define	DataFlashLen 0x3FF  //CMS8S6990��Ƭ������������1KByte��Ѱַ��Χ��0-3FF
#define SysCfgGroupLen (DataFlashLen/sizeof(SysROMImg))-1   //���õ�������ϳ���

//����	
void ReadSysConfig(void);
void SaveSysConfig(bit IsForceSave);	
void LoadMinimumRampCurrentToRAM(void);	
	
#endif
