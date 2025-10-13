/****************************************************************************/
/** \file VersionCheck.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description ����ļ��Ƕ���Ӧ�ò��ļ�������ʵ��ϵͳ�����û������̼�����ʱ���
�Է����û�ȷ�ϵ�ǰϵͳ���صĹ̼��汾�Ĺ���

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "ModeControl.h"
#include "VersionCheck.h"
#include "SideKey.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

extern xdata unsigned char CommonSysFSMTIM;
xdata VersionChkFSMDef VChkFSMState=VersionCheck_InAct;

/****************************************************************************/
/*	Local constant definitions('static const')
****************************************************************************/

/*********** �̼�ʱ��� ***********
�̼�ʱ��������̼�������꣬�£�����
��24Сʱ��ʱ��ͷ��ӡ�
**********************************/
static code char TimeStamp[]={"25 10 13-18 31"};

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

static xdata unsigned char VersionIndex=0; //�汾���ַ���index
static xdata unsigned char VersionShowFastStrobeTIM;  //������˸��ʾ��ʱ��

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/	
//������ʾ����
void VersionCheck_Trigger(void)
	{
	//��״̬������Ϊinit״̬��ʼ��ʾ��������״̬��
	if(VChkFSMState==VersionCheck_InAct)
		{
		VChkFSMState=VersionCheck_StartInit;
		CommonSysFSMTIM=15;  //��ʼ����һ����
		}
	}

//��ʾģ��״̬������
char VersionCheckFSM(void)
	{
	unsigned char buf;
	switch(VChkFSMState)
		{
		//��ʾϵͳδ����
		case VersionCheck_InAct:break;
		//��ʼ����ʾϵͳ	
		case VersionCheck_StartInit:	
			//��ʼ��ʾ����0.5�����ұ�ʾ��ʼ����
			if(CommonSysFSMTIM>6)return 1;
		  //�ȴ�ʱ�䵽
      if(CommonSysFSMTIM)break;
			VersionIndex=0;
			VersionShowFastStrobeTIM=0;
		  VChkFSMState=VersionCheck_LoadNextNumber; //�������ֿ�ʼ��ʾ
			break;
		//������һ������
		case VersionCheck_LoadNextNumber:
			if(TimeStamp[VersionIndex]=='-'||TimeStamp[VersionIndex]==' ')
				{
				//��⵽��ܣ�ͣ��4.5��,����ǿո���ͣ��2.5��
				VChkFSMState=VersionCheck_ShowNumberWait;
				if(TimeStamp[VersionIndex]=='-')CommonSysFSMTIM=36;
				else CommonSysFSMTIM=20;
				}
			else //�����ַ�����������
				{
				buf=TimeStamp[VersionIndex]&0x0F; //ASCII��תʵ����ֵ
				if(!buf)VersionShowFastStrobeTIM=50; //Ϊ0�����ÿ�����˸��ʱ����һ��
				else CommonSysFSMTIM=(buf*4)-1; //��0ֵ���������ִ�С������ʾ��ʱ��
				VChkFSMState=VersionCheck_ShowNumber;
				}
			//ָ����һ���ַ�
			VersionIndex++;
			break;
		//��ʾ����
		case VersionCheck_ShowNumber:
			if(!VersionShowFastStrobeTIM)
				{
				//������ʾ��ʱ����ʱ����������1.5��������ת�����ּ��ؽ׶�
				if(!CommonSysFSMTIM)
					{
					CommonSysFSMTIM=12;
					VChkFSMState=VersionCheck_ShowNumberWait;
					break;
					}
				//������ʼ��ʾ
				if((CommonSysFSMTIM%4)&0x7E)return 1;
				}
		  else 
				{
				//0ֵ�����������ѭ���Կ�����˸��ʱ�������ۼ��������̵ܶ���˸
				VersionShowFastStrobeTIM--;
				return 1;
				}
		  break;
    //�ȴ�����֮��ļ��
		case VersionCheck_ShowNumberWait:
			if(CommonSysFSMTIM)break;
		   //������ʾ���ַ��Ѿ������һ����(�¸��ַ���NULL)����ʾ������
	    if(TimeStamp[VersionIndex]=='\0')VChkFSMState=VersionCheck_InAct;
	    //�����ַ���ʾ�������ǻ����ַ�����׼��������һ������
		  else VChkFSMState=VersionCheck_LoadNextNumber;  
		  break;
		}
	//Ĭ��ʹ����Ϩ�𣬷���0
	return 0;
	}