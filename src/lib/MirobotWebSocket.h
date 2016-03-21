#ifndef __MirobotWebSocket_h__
#define __MirobotWebSocket_h__

#include "Arduino.h"
#include "./lib/ArduinoWebSockets/WebSocketsServer.h"

typedef void (* msgHandler) (char *);

void beginWebSocket();
void handleWebSocket();
void setMsgHandler(msgHandler);
void broadcastMsg(char *);

#endif
