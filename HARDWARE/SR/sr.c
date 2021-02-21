#include "led.h"
#include "lee.h"
#include "delay.h"
#include "sys.h"	
#include "package.h"
#include "DL_LN3X.h"
#include "USART3.h"			 	 
#include "string.h"	
#include "sr.h"
#include <stdio.h>  

#define MYID_H 0x0C
#define MYID_L 0x09

extern void uartRevieveByte(u8 data);

	u8 temperature,testt;  	    
	u8 humidity;  
//newPkg(num)��һ����,�����չ������һ�����ṹ��,�������ݲ��ֳ�����num
//ʹ���������������RAM������һ������3�����ݵİ�
//newPkg(3) redPkg={	
//	.length = 7,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x0000,
//	.data = {0,10,20}
//};
extern u16 CellV_Buff[8];
extern u16 sys_data[40];
extern u16 chargersta;
extern u16 charge_Addr,charge_on,BSMnum,cdzt,chargeXO,ZIresetm;

newPkg(3) redPkg={7,0x90,0x32,0x09,0x00,{0,10,20}};

char buf1[7]={0x05,0x90,0x21,0x00,0x00,0x01,0xFF};//��ȡģ��ĵ�ַָ��,ע����ǰ���0xFE���ӵ��������棬��ΪsendPkg���������Զ��ᷢ0xfe
char buf2[7]={0x05,0x90,0x21,0x00,0x00,0x02,0xFF};//��ȡģ�������IDָ��
char buf3[7]={0x05,0x90,0x21,0x00,0x00,0x03,0xFF};//��ȡģ����ŵ�ָ��
char buf4[7]={0x05,0x90,0x21,0x00,0x00,0x04,0xFF};//��ȡģ��Ĳ�����ָ��



//---------------����Ϊ�ĵ�ַ���ò���------------- �򿪸ĵ�ַʱ   ������main.c�ļ���� ��119�п��Ź�IWDG_Init(4,625);
#define SETID 0   //0 ��������  1�޸ĵ�ַΪ               ����ID��������
newPkg(3) DZpkg1={7,0x90,0x21,0x00,0x00,{0x11,/*������Ϊ��ַ*/0x01,0xB0}}; //��ַ��  B000��BFFF
newPkg(3) DZpkg2={7,0x90,0x21,0x00,0x00,{0x12,0x11,0x00}};
newPkg(2) DZpkg3={6,0x90,0x21,0x00,0x00,{0x13,0x11}};
newPkg(1) DZpkg4={5,0x90,0x21,0x00,0x00,{0x10}};   //res
newPkg(3) DZTEST={7,0x90,0xAA,0xAA,0xAA,{0xCC,0x00,0xAA}};


//	.length = 5,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x000f,
//	.data = {0}
//};
//newPkg(1) THPkg={5,0xa0,0x32,0x0cd0,{0}};
newPkg(4) THPkg={8,0x99,0x99,0x00,0x00,{0}};//ע�����ʽ
newPkg(16) SCell={20,0x99,0x99,0x00,0x00,{0}};//ע�����ʽ

//sPkg* Rpkg;

u8 reclen=0;  
u8 mt=0;


void loopAll()
{ 
	u16 i;
//	u8 add_l,add_h;
//	static volatile u8 chager_on; //��翪����
	static volatile u8 utim=0; 
//	u16 Vocc=261;
//	u16 stax=chargersta;
    ;       //
//	u8 add_id[]={0x02,0x08};
//	add_h=(u8)sys_data[26];
//	add_l=(u8)(sys_data[26]>>8);

#if SETID
	/*������д��ַ�õ����*/
			sendPkg((sPkg*)(&DZpkg1));
			delay_ms(500);
			sendPkg((sPkg*)(&DZpkg2));
			delay_ms(500);
			sendPkg((sPkg*)(&DZpkg3));
			delay_ms(500);
			sendPkg((sPkg*)(&DZpkg4));
			delay_ms(5000);
			//����Ϊ���Ͳ���
			for(i=0;i<8;i++)
			{
			 DZTEST.data[1]=i;
			 sendPkg((sPkg*)(&DZTEST));
			 delay_ms(500);
			}
#else


	///////////////////////////////////////�������ݣ���������ַ����0ʱ
	if(charge_Addr&&utim==6)
	{
		THPkg.remote_addrH=(u8)charge_Addr;  //���ű�ΪҪ���͵�zigbee��ַ
		THPkg.remote_addrL=(charge_Addr>>8)+0XC0;
		SCell.remote_addrH=(u8)charge_Addr;
		SCell.remote_addrL=(charge_Addr>>8)+0XC0;
		Send2num(0x90,0x01,charge_on);   //����
		delay_ms(4);
	}
//		THPkg.remote_addrH=add_id[tim];
//		SCell.remote_addrH=add_id[tim];
	if(charge_Addr)
	{
		switch(utim)
		{
			case 0:Send2num(0xA0,sys_data[0],sys_data[34]);//��ѹ
					delay_ms(4);break;
			case 1:Send2num(0xA1,sys_data[1],sys_data[36]);//����
					delay_ms(4);break;
			case 2:Send2num(0xA2,sys_data[2],sys_data[35]);//SOC+ �¶���Ϣ
					delay_ms(4);break;
			case 3:Send2num(0xA3,sys_data[3],sys_data[4]);//�¶�1 �¶�2
					delay_ms(4);break;
			case 4:Send2num(0xA4,ZIresetm,0);//��λ  ����
					delay_ms(4);break;
			case 5:SendCellbuf(0xA8);//��ֻ��Ϣ
					delay_ms(4);break;
			default: break;
		}
	}
	if(utim>=6)utim=0;else utim++;


		if(USART1_RX_STA&0X8000)			//���յ�һ��������
		 {

		   reclen=USART1_RX_STA&0X7FFF;	//�õ����ݳ���
			 
			 for(i=0;i<reclen;i++)
			 uartRevieveByte(USART1_RX_BUF[i]);

			 USART1_RX_STA=0;	 
		 }

//		for(i = 0;i<10;i++)
//			{
				delay_ms(10);
				loopReceive();
		 
	#endif	 
//			}		 
 									
//	}

}


//���������Ҫ�ڹ����в��ϱ�����,���᳢��һ�ν��հ�,
//������ճɹ��ͽ���recievePkg����,���ٴγ���,ֱ���ղ����µİ�/////////////////��������cdzt��chargeXO
void loopReceive(void)
{
	sPkg* pkg;
	pkg = getNextPkg();
	while(pkg != NULL)
	{		
	 //if( pkg->data[0]== 0x01) LED0=!LED0;
        recievePkg(pkg);
		pkg = getNextPkg();	
	}
}

u8 Rnumbuf[8]={0};

//�յ�һ�����������������,����������ݰ���Ŀ�Ķ˿�ѡ����Ӧ�ĳ�����д���
void recievePkg(sPkg* pkg)
{
	if((pkg->src_port==0xB0)&&charge_Addr)
	{
		cdzt=pkg->data[0];
		chargeXO=pkg->data[2];
	}
//	if(THPkg.remote_addrH=pkg->remote_addrH)
//	{ 
//		switch(pkg->src_port)
//		{
//			case 0xB0:
//				RestoreNum(pkg,2);
//				sys_data[0]=Rnumbuf[0];
//				sys_data[34]=(u8)Rnumbuf[1];
//				break;
//			case 0xA1:
//				RestoreNum(pkg,2);
//				sys_data[1]=Rnumbuf[0];
//				sys_data[36]=(u8)Rnumbuf[1];
//				break;
//			case 0xA2:
//				RestoreNum(pkg,2);
//				sys_data[2]=Rnumbuf[0];
//				sys_data[35]=(u8)Rnumbuf[1];
//				break;
//			case 0xA3:
//				RestoreNum(pkg,2);
//				sys_data[3]=Rnumbuf[0];
//				sys_data[4]=Rnumbuf[1];
//				break;
//			default:
//				break;		
//		}
//	}	
}

void RestoreNum(sPkg* pkg,u8 num)//  pkg ��Ҫ��ԭ�İ�   num ��Ҫ��ԭ�����ݸ���
{ 
	u8 i;
	for(i=0;i<num;i++)
	{ 
		Rnumbuf[i]=(pkg->data[2*i+1])<<8;
		Rnumbuf[i]+=(pkg->data[2*i]);
	}
}


void initAll()
{

//  delay_init();	    	 //��ʱ������ʼ��	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
    USART1_init(115200);//����3��ʼ��Ϊ9600
//	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ� 

//	while(DHT11_Init())	//DHT11��ʼ��	
//	{

//	}	
	
	
}

void Send2num(u8 dis_port,u16 massage1,u16 massage2)
{
		THPkg.dis_port = dis_port;
		THPkg.data[0]=(u8)massage1;
	  THPkg.data[1]=(u8)(massage1>>8);
		THPkg.data[2]=(u8)massage2;
	  THPkg.data[3]=(u8)(massage2>>8);
	  sendPkg((sPkg*)(&THPkg));
}

void SendCellbuf(u8 dis_port)
{
	u8 i;
	SCell.dis_port = dis_port;
	for(i=0;i<8;i++)
	{
		SCell.dis_port = dis_port;
		SCell.data[2*i]=(u8)CellV_Buff[i];
		SCell.data[2*i+1]=(u8)((CellV_Buff[i])>>8);
	}
	sendPkg((sPkg*)(&SCell));
}
