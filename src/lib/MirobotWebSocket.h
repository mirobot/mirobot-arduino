#ifndef __MirobotWebSocket_h__
#define __MirobotWebSocket_h__

#include "Arduino.h"
#include <WebSocketsServer.h>

typedef void (* msgHandler) (char *);

void beginWebSocket();
void handleWebSocket();
void setMsgHandler(msgHandler);
void broadcastMsg(char *);

#endif
