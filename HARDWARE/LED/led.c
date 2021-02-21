#include "led.h"


void Start_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB,PE�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;				 //
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO
 
	 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		//������������
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);		           //PC0
	
	GPIO_SetBits(GPIOC,GPIO_Pin_10);						 //PC10 �����led
		    GPIO_ResetBits(GPIOC,GPIO_Pin_9);      //������
 GPIO_ResetBits(GPIOC,GPIO_Pin_11);		             //sg1��Ҫ����ź�
 GPIO_ResetBits(GPIOC,GPIO_Pin_12);		            //sg2����
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_8|GPIO_Pin_9;	    		 //LED1-->PE.5 �˿�����, �������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOB,GPIO_Pin_0); 						 //CHR
	
 GPIO_SetBits(GPIOB,GPIO_Pin_1);             //DCHR
 GPIO_ResetBits(GPIOB,GPIO_Pin_8);	           //sg3����
 GPIO_ResetBits(GPIOB,GPIO_Pin_9);	           //sg4�ж�
	
	
}

 
 
 
 

