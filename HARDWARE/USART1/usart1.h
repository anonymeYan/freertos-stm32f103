#ifndef __usart1_H
#define __usart1_H
#include "stdio.h"	
#include "sys.h" 
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			3  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����



extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
extern int ok_flag;
//����봮���жϽ��գ��벻Ҫע�����º궨��
void USART1_IRQHandler(void);
void USART1_init(u32 bound);
void ShowRXbuf(void);

void USART1_Putc(u8 c);

void usart1_Sendnbyte(u8 *str,int len);

void usart1_SendArray(u8 *buf,u8 len);

void USART_OUT(USART_TypeDef* USARTx, char *Data);








#endif






