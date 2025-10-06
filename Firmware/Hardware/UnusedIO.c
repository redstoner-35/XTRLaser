/****************************************************************************/
/** \file UnusedIO.c
/** \Author redstoner_35
/** \Project Xtern Ripper Laser Edition 
/** \Description 这个文件负责实现对单片机未使用的IO进行屏蔽处理，降低IO浮空产生
的异常待机功耗

**	History: Initial Release
**	
*****************************************************************************/
/****************************************************************************/
/*	include files
*****************************************************************************/
#include "GPIO.h"
#include "cms8s6990.h"

/****************************************************************************/
/*	Local pre-processor symbols/macros('#define')
****************************************************************************/
#define P0UnusedPIN 0x3C          //P0.2-P0.5
#define P1UnusedPIN (0x01<<4)			//P1.4
#define P2UnusedPIN 0x30          //P2.4-P2.5
#define P3UnusedPIN (0x01<<2)     //P3.2

/****************************************************************************/
/*	Global variable definitions(declared in header file with 'extern')
****************************************************************************/

/****************************************************************************/
/*	Local type definitions('typedef')
****************************************************************************/

/****************************************************************************/
/*	Local variable  definitions('static')
****************************************************************************/

/****************************************************************************/
/*	Local function prototypes('static')
****************************************************************************/

/****************************************************************************/
/*	Function implementation - global ('extern') and local('static')
****************************************************************************/
void MaskUnusedIO(void)
	{
	//Mask掉无用的IO强制输出0	
	P0TRIS|=P0UnusedPIN;
	P1TRIS|=P1UnusedPIN;
	P2TRIS|=P2UnusedPIN;
	P3TRIS|=P3UnusedPIN;
	//所有无用的IO强制输出0
	P0&=~P0UnusedPIN;
	P1&=~P1UnusedPIN;
	P2&=~P2UnusedPIN;
	P3&=~P3UnusedPIN;
	//所有对外IO关闭上下拉电阻
	P0UP&=~P0UnusedPIN;
	P0RD&=~P0UnusedPIN;
	
	P1UP&=~P1UnusedPIN;
	P1RD&=~P1UnusedPIN;
		
	P2UP&=~P2UnusedPIN;
	P2RD&=~P2UnusedPIN;
		
	P3UP&=~P3UnusedPIN;
	P3RD&=~P3UnusedPIN;
	}
