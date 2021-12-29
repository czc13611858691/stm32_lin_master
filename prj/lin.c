#include "sys.h"
#include "lin.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
 
u8 LIN_TX_BUF[]={0x11,0x4a,0x55,0x93,0xe5};
void LIN1_Master_Init(u32 Baud){

  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART1，GPIOA时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//引脚配置
	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART3_RX	  GPIOB.11初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.10  
	
	//USART_DeInit(USART1);
	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,ENABLE);
	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,DISABLE);

	USART_InitStructure.USART_BaudRate = Baud;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART3, &USART_InitStructure); //初始化串口1
	
	USART_LINBreakDetectLengthConfig(USART3,USART_LINBreakDetectLength_11b);
	USART_LINCmd(USART3,ENABLE);
	
  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
//  
//   //USART 初始化设置

	USART_ITConfig(USART3, USART_IT_LBD, ENABLE);//开启串口LIN中断
	USART_GetFlagStatus(USART3, USART_FLAG_TC);  //清TC标志位
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
	//USART_ITConfig(USART1, USART_IT_TC,ENABLE);//开启串口发送中断
//	
  USART_Cmd(USART3, ENABLE);                    //使能串口1 

}


/****************************************************************************************
** 函数名称: LINCheckSum
** 功能描述: 计算并返回LIN校验值
** 参    数:  u8 *buf：需要计算的数组
			  u8 lens：数组长度
** 返 回 值:   u8 ckm: 计算结果
** 日  　期: 2015年11月17日
**---------------------------------------------------------------------------------------
** 修 改 人: WEN
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
u8 LINCheckSum(u8 *buf, u8 lens)
{
    u8 i, ckm = 0;
    u16 chm1 = 0;
    for(i = 0; i < lens; i++)
    {
        chm1 += *(buf++);
    }
    ckm = chm1 / 256;
    ckm = ckm + chm1 % 256;
    ckm = 0xFF - ckm;
    return ckm;
}

/****************************************************************************************
** 函数名称: LinMASTER_SendDataStream
** 功能描述: LIN主模式下，发送报文头，PID
** 参    数: *buf:数组地址；
			 lens:数据长度,不含校验字节
			 CheckSumType: 校验类型，=1：增强型ENHANGCED；=0：基本型CLASSIC
** 返 回 值: 无
** 日  　期: 2015年11月17日
**---------------------------------------------------------------------------------------
** 修 改 人: WEN
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void LIN1_MASTER_SendDataStream(u8 *buf, u8 lens, u8 CheckSumType)
{
	  u8 i = 0;
    if(CheckSumType)
        *(buf + lens) = LINCheckSum(buf, lens); //计算增强型校验码,连PID一起校验
    else
        *(buf + lens) = LINCheckSum(buf + 1, lens-1); //计算标准型校验码，不计算PID

    USART_SendBreak(USART3);
    USART_SendData(USART3, 0X55);

    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    while(i < lens+1)
    {
        USART_SendData(USART3, buf[i]);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        i++;  
    }
}

void LIN1_MASTER_GetDataStream(u8 PID)
{
	u8 i = 0;

	USART_SendBreak(USART3);
	USART_SendData(USART3, 0X55);

	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, PID);

}
u8 *tx;
u8 rev_flag=0;
u8 LIN_LEN = 0;
u8 LIN_RX_BUF[64];
u16 UART3_cnt;
void USART3_IRQHandler(void)                	//串口1中断服务程序
{
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
    if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
    {
        LIN_RX_BUF[0] = USART_ReceiveData(USART3); //取出来扔掉
        USART_ClearFlag(USART3, USART_FLAG_ORE);
    }
		if(USART_GetITStatus(USART3, USART_IT_LBD) != RESET)
		{
				USART_ClearITPendingBit(USART3, USART_IT_LBD);
				if(LIN_LEN>0)
				{
						UART3_cnt = LIN_LEN;
						LIN_LEN = 0;
						rev_flag=1;
				}
		}
		
		if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
		{
				LIN_RX_BUF[LIN_LEN++] = USART_ReceiveData(USART3);
				USART_ClearITPendingBit(USART3, USART_IT_RXNE);
				if((LIN_RX_BUF[0] == 0x55) && (LIN_LEN == 2))
				{
						//Lin1Slave_SendDataCks(TstLin1Slave_SendBuff, TstLin1Slave_SendBufflens);//采用经典校验
						//Lin1Slave_SendDataCksEn(TstLin1Slave_PID, TstLin1Slave_SendBuff, TstLin1Slave_SendBufflens); //采用增强校验
					
				}
				if(LIN_LEN > 64)LIN_LEN = 0;
		}
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 

