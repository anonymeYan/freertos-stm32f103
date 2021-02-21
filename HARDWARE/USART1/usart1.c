#include "delay.h"
#include "usart1.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
#include "timer.h"
#include "lee.h"

unsigned char Message_length;
int OneNet_Connect=0;      //用于判断wifi模块是否连接上OneNet平台的标志

char Rx_Buff[200];
int  Rx_count=0;   //用于ESP8266判断接受数据的多少
int ok_flag=0;
char Message_Buf[20];  //用于存贮操作指令
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u16 USART_RX_STA=0;       //接收状态标记	  
extern u8  AT_Mode;

//通用定时器5中断初始化
//这里时钟选择为APB1的2倍，
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//通用定时器中断初始化
//这里始终选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM5_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//TIM5时钟使能    
	
	//定时器TIM4初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //使能指定的TIM5中断,允许更新中断
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

void TIM5_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		USART_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIM5更新中断标志    
		TIM_Cmd(TIM5, DISABLE);  //关闭TIM45
	}	    
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;	
	static int i;
//		if(AT_Mode==0)        
//		Rx_count=0;
		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
  {
	    	Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据	
				Rx_Buff[Rx_count]=Res;
    if(AT_Mode==1)			                         //配置wifi模块发AT指令时的模式
		{
			if(strstr(Rx_Buff,"OK")||strstr(Rx_Buff,">"))    //判断ESP8266是否返回OK
			{
				 memset(Rx_Buff, 0, sizeof(Rx_Buff));  //清空Rx_Buff数组
         ok_flag=1;			 
         Rx_count=0;				
			}
			else                                   //否则继续接受数据
			{
						Rx_count++;	
					if(Rx_count==150)                   //数据溢出，清空数组 方便继续接收
						{
								Rx_count=0;
					    memset(Rx_Buff, 0, sizeof(Rx_Buff));
						}
			}
		}
			else if(AT_Mode==0)               //接收控制指令模式
			{
					 	
					if(Rx_count==150)                   //数据溢出，清空数组 方便继续接收
						{
								Rx_count=0;                  //重新从数组[0]开始接收数据
					    memset(Rx_Buff, 0, sizeof(Rx_Buff));  //清空当前数组所有数据
						}
					    ok_flag=1;                         //该标志是在发AT指令过程中才用的 可以在SendCmd()函数中查看其作用 保持AT指令的完成状态
						
						if(Message_length>0)                   //表示可以开始存贮操作指令
						 {
						    Message_Buf[i]=Rx_Buff[Rx_count];   //存贮操作指令数据
							  i++;
						    Message_length--;                  //存一个指令，剩余数量减一,判断操作指令是否存贮完成
						 }
						 
						if(Rx_count>3&&Rx_Buff[Rx_count-2]==0&&Rx_Buff[Rx_count-1]==0&&Rx_Buff[Rx_count]>0)   
						//如果当前接收到的数据大于0，并且这个数据的前两个数据为00 代表当前数据就是操作指令的长度。
						{
							 memset(Message_Buf, 0, sizeof(Message_Buf)); //清空存贮操作指令的数组，准备存贮新的操作指令
							
						   Message_length=Rx_Buff[Rx_count];      //将接收到的数据存为操作指令长度。
							 i=0;                                   //清空i
						}
						Rx_count++;                               //准备存储下一个数据 
						
			 }
           
			USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		 }	 
		
		 
} 



//初始化IO 串口1
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void USART1_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //串口3时钟使能

 	USART_DeInit(USART1);  //复位串口1
		 //USART1_TX   PA9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PB10

	//USART1_RX	  PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PB11
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART1, &USART_InitStructure); //初始化串口	1
  

	USART_Cmd(USART1, ENABLE);                    //使能串口 
	
	//使能接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断  

	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	
	TIM5_Int_Init(99,7199);		//10ms中断
	USART_RX_STA=0;		//清零
	TIM_Cmd(TIM5,DISABLE);			//关闭定时器5

}

////串口1,printf 函数
////确保一次发送数据不超过USART1_MAX_SEND_LEN字节
//void u1_printf(char* fmt,...)  
//{  
//	u16 i,j; 
//	va_list ap; 
//	va_start(ap,fmt);
//	vsprintf((char*)USART1_TX_BUF,fmt,ap);
//	va_end(ap);
//	i=strlen((const char*)USART1_TX_BUF);		//此次发送数据的长度
//	for(j=0;j<i;j++)							//循环发送数据
//	{
//	  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
//		USART_SendData(USART1,USART1_TX_BUF[j]); 
//	} 
//}
//发送数据都是一连串的数据,这一串数据的第一个使用这个函数发送
//这个函数会打开发送缓冲空闲中断
u8 uartSendFirstByte(u8 data)
{
	if((USART1->SR&0X40)==0)
	{//没有开这个中断才能这么发		

		USART1->DR=0xfe;
		while((USART1->SR&0X40)==0);//等待发送结束
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
////串口1 发送字符串
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
	for(t=0;t<len;t++)		//循环发送数据
	{		    		
		USART1_Putc(*str++);
	}	 

}




//发送n个字节数据
//buff:发送区首地址
//len：发送的字节数
void usart1_SendArray(u8 *buf,u8 len)
{
	u8 t;	
//	delay_ms(5);
  	for(t=0;t<len;t++)		//循环发送数据
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

	while(*Data!=0){				                          //判断是否到达字符串结束符
		if(*Data==0x5c)
			{									  //'\'
			switch (*++Data){
				case 'r':							          //回车符
					USART_SendData(USARTx, 0x0d);	   

					Data++;
					break;
				case 'n':							          //换行符
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
//				case 's':										  //字符串
//                	s = va_arg(ap, const char *);
//                	for ( ; *s; s++) {
//                    	USART_SendData(USARTx,*s);
//						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
//                	}
//					Data++;
//                	break;
//            	case 'd':										  //十进制
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



