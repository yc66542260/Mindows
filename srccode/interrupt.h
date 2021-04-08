
#ifndef  INTERRUPT_H
#define  INTERRUPT_H


#include "global.h"


#define UARTINTTXMASK               0x2     /* 串口发送中断 */


/*********************************** 函数声明 *************************************/
extern void INT_UartPrint(void);


#endif

