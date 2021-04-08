
#ifndef  DEVICE_H
#define  DEVICE_H


#include "global.h"


/*********************************** º¯ÊýÉùÃ÷ *************************************/
extern void DEV_UartInit(void);
extern void DEV_PutChar(U8 ucChar);
extern void DEV_TimerInit(void);
extern void DEV_SetGpioMode(void);
extern U8 DEV_Num2Asc(U8 ucHex);
extern U8 DEV_Asc2Num(U8 ucAsc);


#endif

