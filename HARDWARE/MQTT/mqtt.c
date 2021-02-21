#include "mqtt.h"
#include "MQTTPacket.h"
#include "ytransport.h"
#include "types.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//MQTT发布消息函数
int do_mqtt_publish(char *pTopic,char *pMessage)
{
	int32_t len,rc;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;//配置部分可变头部的值
	unsigned char buf[100];
	MQTTString topicString = MQTTString_initializer;
	int msglen = strlen(pMessage);//计算发布消息的长度
	int buflen = sizeof(buf);

	data.clientID.cstring = "504865372";//客户端标识，用于区分每个客户端
	data.keepAliveInterval = 600;//保活计时器，定义了服务器收到客户端消息的最大时间间隔
	data.cleansession = 1;//该标志置1服务器必须丢弃之前保持的客户端的信息，将该连接视为“不存在”
	data.username.cstring = "190125";
	data.password.cstring = "GmHLuQA7f7uq8pSCQgbIbzL2XM8=";
	len = MQTTSerialize_connect(buf, buflen, &data); /*1 构造连接报文*/	
	rc = transport_sendPacketBuffer(buf, len);//发送连接请求
	
	

	memset(buf,0,buflen);
  strcat(pTopic,data.clientID.cstring);//合并字符串为真正主题
	topicString.cstring = pTopic;
	len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /*2 构造发布消息的报文*/
	
	rc = transport_sendPacketBuffer(buf,len);//发送消息
	memset(buf,0,buflen);


	return 0;
}

	
	



