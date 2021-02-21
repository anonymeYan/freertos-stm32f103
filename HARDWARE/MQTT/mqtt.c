#include "mqtt.h"
#include "MQTTPacket.h"
#include "ytransport.h"
#include "types.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//MQTT������Ϣ����
int do_mqtt_publish(char *pTopic,char *pMessage)
{
	int32_t len,rc;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//���ò��ֿɱ�ͷ����ֵ
	unsigned char buf[100];
	MQTTString topicString = MQTTString_initializer;
	int msglen = strlen(pMessage);//���㷢����Ϣ�ĳ���
	int buflen = sizeof(buf);

	data.clientID.cstring = "504865372";//�ͻ��˱�ʶ����������ÿ���ͻ���
	data.keepAliveInterval = 600;//�����ʱ���������˷������յ��ͻ�����Ϣ�����ʱ����
	data.cleansession = 1;//�ñ�־��1���������붪��֮ǰ���ֵĿͻ��˵���Ϣ������������Ϊ�������ڡ�
	data.username.cstring = "190125";
	data.password.cstring = "GmHLuQA7f7uq8pSCQgbIbzL2XM8=";
	len = MQTTSerialize_connect(buf, buflen, &data); /*1 �������ӱ���*/	
	rc = transport_sendPacketBuffer(buf, len);//������������
	
	

	memset(buf,0,buflen);
  strcat(pTopic,data.clientID.cstring);//�ϲ��ַ���Ϊ��������
	topicString.cstring = pTopic;
	len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /*2 ���췢����Ϣ�ı���*/
	
	rc = transport_sendPacketBuffer(buf,len);//������Ϣ
	memset(buf,0,buflen);


	return 0;
}

	
	



