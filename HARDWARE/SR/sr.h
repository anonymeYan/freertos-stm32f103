#ifndef __SR_H
#define __SR_H

#include "package.h"

void recievePkg(sPkg* pkg);
void loopReceive(void);
void loopAll(void);
void initAll(void);
void Send2num(u8 dis_port,u16 massage1,u16 massage2);
void SendCellbuf(u8 dis_port);

void RestoreNum(sPkg* pkg,u8 num);
#endif

