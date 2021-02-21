#include "delay.h"
#include "usart1.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
#include "timer.h"
#include "lee.h"

unsigned char Message_length;
int OneNet_Connect=0;      //�����ж�wifiģ���Ƿ�������OneNetƽ̨�ı�־

char Rx_Buff[200];
int  Rx_count=0;   //����ESP8266�жϽ������ݵĶ���
int ok_flag=0;
char Message_Buf[20];  //���ڴ�������ָ��
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u16 USART_RX_STA=0;       //����״̬���	  
extern u8  AT_Mode;

//ͨ�ö�ʱ��5�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2����
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz 
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM5_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//TIM5ʱ��ʹ��    
	
	//��ʱ��TIM4��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM5�ж�,��������ж�
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

void TIM5_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		USART_RX_STA|=1<<15;	//��ǽ������
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //���TIM5�����жϱ�־    
		TIM_Cmd(TIM5, DISABLE);  //�ر�TIM45
	}	    
}

void USART1_IRQHandler(void)                	//����1�жϷ������
	{
	u8 Res;	
	static int i;
//		if(AT_Mode==0)        
//		Rx_count=0;
		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
  {
	    	Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������	
				Rx_Buff[Rx_count]=Res;
    if(AT_Mode==1)			                         //����wifiģ�鷢ATָ��ʱ��ģʽ
		{
			if(strstr(Rx_Buff,"OK")||strstr(Rx_Buff,">"))    //�ж�ESP8266�Ƿ񷵻�OK
			{
				 memset(Rx_Buff, 0, sizeof(Rx_Buff));  //���Rx_Buff����
         ok_flag=1;			 
         Rx_count=0;				
			}
			else                                   //���������������
			{
						Rx_count++;	
					if(Rx_count==150)                   //���������������� �����������
						{
								Rx_count=0;
					    memset(Rx_Buff, 0, sizeof(Rx_Buff));
						}
			}
		}
			else if(AT_Mode==0)               //���տ���ָ��ģʽ
			{
					 	
					if(Rx_count==150)                   //���������������� �����������
						{
								Rx_count=0;                  //���´�����[0]��ʼ��������
					    memset(Rx_Buff, 0, sizeof(Rx_Buff));  //��յ�ǰ������������
						}
					    ok_flag=1;                         //�ñ�־���ڷ�ATָ������в��õ� ������SendCmd()�����в鿴������ ����ATָ������״̬
						
						if(Message_length>0)                   //��ʾ���Կ�ʼ��������ָ��
						 {
						    Message_Buf[i]=Rx_Buff[Rx_count];   //��������ָ������
							  i++;
						    Message_length--;                  //��һ��ָ�ʣ��������һ,�жϲ���ָ���Ƿ�������
						 }
						 
						if(Rx_count>3&&Rx_Buff[Rx_count-2]==0&&Rx_Buff[Rx_count-1]==0&&Rx_Buff[Rx_count]>0)   
						//�����ǰ���յ������ݴ���0������������ݵ�ǰ��������Ϊ00 ����ǰ���ݾ��ǲ���ָ��ĳ��ȡ�
						{
							 memset(Message_Buf, 0, sizeof(Message_Buf)); //��մ�������ָ������飬׼�������µĲ���ָ��
							
						   Message_length=Rx_Buff[Rx_count];      //�����յ������ݴ�Ϊ����ָ��ȡ�
							 i=0;                                   //���i
						}
						Rx_count++;                               //׼���洢��һ������ 
						
			 }
           
			USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		 }	 
		
		 
} 



//��ʼ��IO ����1
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void USART1_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //����3ʱ��ʹ��

 	USART_DeInit(USART1);  //��λ����1
		 //USART1_TX   PA9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PB10

	//USART1_RX	  PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PB11
	
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART1, &USART_InitStructure); //��ʼ������	1
  

	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�  

	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	
	TIM5_Int_Init(99,7199);		//10ms�ж�
	USART_RX_STA=0;		//����
	TIM_Cmd(TIM5,DISABLE);			//�رն�ʱ��5

}

////����1,printf ����
////ȷ��һ�η������ݲ�����USART1_MAX_SEND_LEN�ֽ�
//void u1_printf(char* fmt,...)  
//{  
//	u16 i,j; 
//	va_list ap; 
//	va_start(ap,fmt);
//	vsprintf((char*)USART1_TX_BUF,fmt,ap);
//	va_end(ap);
//	i=strlen((const char*)USART1_TX_BUF);		//�˴η������ݵĳ���
//	for(j=0;j<i;j++)							//ѭ����������
//	{
//	  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
//		USART_SendData(USART1,USART1_TX_BUF[j]); 
//	} 
//}
//�������ݶ���һ����������,��һ�����ݵĵ�һ��ʹ�������������
//���������򿪷��ͻ�������ж�
u8 uartSendFirstByte(u8 data)
{
	if((USART1->SR&0X40)==0)
	{//û�п�����жϲ�����ô��		

		USART1->DR=0xfe;
		while((USART1->SR&0X40)==0);//�ȴ����ͽ���
		return done;
	}
	else
	{
		return fail;
	}
}
 


//void USART1_Putc(unsigned char c)
//{
//	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET );
//	USART_SendData(USART2, c);
//	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET );
//}
////����1 �����ַ���
//void usart1_send_nbyte(unsigned char *str, int len)
//{
//	int i;
//	for(i = 0; i < len; i++)
//	{
//	USART1_Putc(*str++);
//	}
//}


void USART1_Putc(u8 c)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET );
	
  USART_SendData(USART1, c);	   
//	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET );		
		
	}	 
 

void usart1_Sendnbyte(u8 *str,int len)
{
	int t;
	for(t=0;t<len;t++)		//ѭ����������
	{		    		
		USART1_Putc(*str++);
	}	 

}




//����n���ֽ�����
//buff:�������׵�ַ
//len�����͵��ֽ���
void usart1_SendArray(u8 *buf,u8 len)
{
	u8 t;	
//	delay_ms(5);
  	for(t=0;t<len;t++)		//ѭ����������
	{		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	 
//delay_us(5);		
		USART_SendData(USART1,buf[t]);
	}	 
 while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}




void USART_OUT(USART_TypeDef* USARTx, char *Data){ 
	const char *s;
//    int d;
//    char buf[16];
//    va_list ap;
//    va_start(ap, Data);

	while(*Data!=0){				                          //�ж��Ƿ񵽴��ַ���������
		if(*Data==0x5c)
			{									  //'\'
			switch (*++Data){
				case 'r':							          //�س���
					USART_SendData(USARTx, 0x0d);	   

					Data++;
					break;
				case 'n':							          //���з�
					USART_SendData(USARTx, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				USART_SendData(USARTx, *Data++);
				    break;
			} 		 
		}
		
//		else if(*Data=='%'){									  //
//			switch (*++Data){				
//				case 's':										  //�ַ���
//                	s = va_arg(ap, const char *);
//                	for ( ; *s; s++) {
//                    	USART_SendData(USARTx,*s);
//						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
//                	}
//					Data++;
//                	break;
//            	case 'd':										  //ʮ����
//                	d = va_arg(ap, int);
//                	itoa1(d, buf, 10);
//                	for (s = buf; *s; s++) {
//                    	USART_SendData(USARTx,*s);
//						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
//                	}
//					Data++;
//                	break;
//				default:
//					Data++;
//				    break;
//			}		 
//		}
		else USART_SendData(USARTx, *Data++);
		
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}



