#ifndef __LIN_H
#define __LIN_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	
extern u8 *tx;
extern u8 rev_flag;
extern u8 LIN_TX_BUF[];
extern u8  LIN_RX_BUF[]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
//extern u16 LIN_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void LIN1_Master_Init(u32 Baud);
void LIN1_MASTER_SendDataStream(u8 *buf, u8 lens, u8 CheckSumType);
void LIN1_MASTER_GetDataStream(u8 PID);
#endif


