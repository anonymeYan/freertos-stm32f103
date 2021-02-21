#ifndef _esp8266_H_
#define _esp8266_H_

#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־

extern unsigned char  AT_Mode;     //��ATָ���ģʽ ���ڱ�ʾ����������Wifiģ���ģʽ  �Դ��ڽ������ݽ��в�ͬ�Ĵ���
extern unsigned char  Contral_flag;  //���ڴ�������ָ��  �жϽ���ʲô����
extern char Rx_Buff[200];
extern int  Rx_count;

/******��OneNet�������������ݶԽ�ʱ������ */
#define AT          "AT\r\n"	
#define CWMODE      "AT+CWMODE=3\r\n"		//STA+APģʽ
#define wifi_RST    "AT+RST\r\n"
#define CIFSR       "AT+CIFSR\r\n"
#define CWJAP       "AT+CWJAP=\"thjx3\",\"taihang5012553\"\r\n"	//ssid:  ���룺
#define CIPSTART    "AT+CIPSTART=\"TCP\",\"183.230.40.39\",6002\r\n"	//MQTT������ 183.230.40.39       6002
#define CIPMODE0    "AT+CIPMODE=0\r\n"		//��͸��ģʽ
#define CIPMODE1    "AT+CIPMODE=1\r\n"		//͸��ģʽ
#define CIPSEND     "AT+CIPSEND\r\n"   
#define Out_CIPSEND     "+++" 
#define CIPSTATUS   "AT+CIPSTATUS\r\n"		//����״̬��ѯ





void ESP8266_Init(void);

void ESP8266_Clear(void);

void SendCmd(char* cmd, char* result, int timeOut);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);

void ESP8266_SendData(unsigned char *data, unsigned short len);

void ESP8266Mode_inti(void);

void ESP8266_GPIO_Init(void);

void USART1_Write(unsigned char *cmd, int len);

#endif
