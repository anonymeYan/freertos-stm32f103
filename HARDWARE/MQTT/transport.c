#include "transport.h"
#include "usart1.h"


/*订阅消息发送数据函数*/
int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
    int rc = 0;
    
    usart1_Sendnbyte( buf, buflen);
    
    return rc;
}








