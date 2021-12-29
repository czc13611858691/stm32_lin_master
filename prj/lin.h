#ifndef __LIN_H
#define __LIN_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	
extern u8 *tx;
extern u8 rev_flag;
extern u8 LIN_TX_BUF[];
extern u8  LIN_RX_BUF[]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
//extern u16 LIN_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void LIN1_Master_Init(u32 Baud);
void LIN1_MASTER_SendDataStream(u8 *buf, u8 lens, u8 CheckSumType);
void LIN1_MASTER_GetDataStream(u8 PID);
#endif


