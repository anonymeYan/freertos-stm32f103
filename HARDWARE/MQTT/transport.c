#include "transport.h"
#include "usart1.h"


/*������Ϣ�������ݺ���*/
int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
    int rc = 0;
    
    usart1_Sendnbyte( buf, buflen);
    
    return rc;
}








