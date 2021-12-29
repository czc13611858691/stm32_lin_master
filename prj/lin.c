#include "sys.h"
#include "lin.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
 
u8 LIN_TX_BUF[]={0x11,0x4a,0x55,0x93,0xe5};
void LIN1_Master_Init(u32 Baud){

  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART1��GPIOAʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//��������
	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART3_RX	  GPIOB.11��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.10  
	
	//USART_DeInit(USART1);
	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,ENABLE);
	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,DISABLE);

	USART_InitStructure.USART_BaudRate = Baud;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART3, &USART_InitStructure); //��ʼ������1
	
	USART_LINBreakDetectLengthConfig(USART3,USART_LINBreakDetectLength_11b);
	USART_LINCmd(USART3,ENABLE);
	
  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
//  
//   //USART ��ʼ������

	USART_ITConfig(USART3, USART_IT_LBD, ENABLE);//��������LIN�ж�
	USART_GetFlagStatus(USART3, USART_FLAG_TC);  //��TC��־λ
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	//USART_ITConfig(USART1, USART_IT_TC,ENABLE);//�������ڷ����ж�
//	
  USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���1 

}


/****************************************************************************************
** ��������: LINCheckSum
** ��������: ���㲢����LINУ��ֵ
** ��    ��:  u8 *buf����Ҫ���������
			  u8 lens�����鳤��
** �� �� ֵ:   u8 ckm: ������
** ��  ����: 2015��11��17��
**---------------------------------------------------------------------------------------
** �� �� ��: WEN
** �ա�  ��:
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
** ��������: LinMASTER_SendDataStream
** ��������: LIN��ģʽ�£����ͱ���ͷ��PID
** ��    ��: *buf:�����ַ��
			 lens:���ݳ���,����У���ֽ�
			 CheckSumType: У�����ͣ�=1����ǿ��ENHANGCED��=0��������CLASSIC
** �� �� ֵ: ��
** ��  ����: 2015��11��17��
**---------------------------------------------------------------------------------------
** �� �� ��: WEN
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void LIN1_MASTER_SendDataStream(u8 *buf, u8 lens, u8 CheckSumType)
{
	  u8 i = 0;
    if(CheckSumType)
        *(buf + lens) = LINCheckSum(buf, lens); //������ǿ��У����,��PIDһ��У��
    else
        *(buf + lens) = LINCheckSum(buf + 1, lens-1); //�����׼��У���룬������PID

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
void USART3_IRQHandler(void)                	//����1�жϷ������
{
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
    if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
    {
        LIN_RX_BUF[0] = USART_ReceiveData(USART3); //ȡ�����ӵ�
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
						//Lin1Slave_SendDataCks(TstLin1Slave_SendBuff, TstLin1Slave_SendBufflens);//���þ���У��
						//Lin1Slave_SendDataCksEn(TstLin1Slave_PID, TstLin1Slave_SendBuff, TstLin1Slave_SendBufflens); //������ǿУ��
					
				}
				if(LIN_LEN > 64)LIN_LEN = 0;
		}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 

