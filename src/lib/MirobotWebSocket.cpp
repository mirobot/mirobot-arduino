#ifdef ESP8266

#include "MirobotWebSocket.h"

WebSocketsServer ws = WebSocketsServer(8899);

msgHandler handler = NULL;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      break;
    case WStype_TEXT:
      if(handler) handler((char *)payload);
      break;
  }
}

void beginWebSocket(){
  ws.begin();
  ws.onEvent(webSocketEvent);
}

void handleWebSocket(){
  ws.loop();
}

void setMsgHandler(msgHandler h){
  handler = h;
}

void broadcastMsg(char * msg){
  ws.broadcastTXT(msg, strlen(msg));
}

#endif