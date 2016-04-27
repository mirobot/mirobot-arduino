#ifndef __SerialWebSocket_h__
#define __SerialWebSocket_h__

#include "Arduino.h"
#include "./lib/ArduinoJson/ArduinoJson.h"

typedef enum {
  SERWS_WAITING,
  SERWS_HEADERS,
  SERWS_RESPOND,
  SERWS_READY
} wsState_t;

typedef enum {
  SERWS_HEADERS_PROCESSED,
  SERWS_HEADERS_NOT_PROCESSED,
  SERWS_FRAME_PROCESSED,
  SERWS_FRAME_NOT_PROCESSED,
  SERWS_FRAME_EMPTY,
  SERWS_FRAME_ERROR,
  SERWS_NOT_PROCESSED
} processState_t;

class SerialWebSocket {
  public:
    SerialWebSocket(Stream &s);
    processState_t process(char *, int);
    void send(ArduinoJson::JsonObject &);
  private:
    processState_t processWSFrame(char *, int);
    processState_t processHeaders(char *, int);
    void sendHandshake();
    wsState_t wsState = SERWS_WAITING;
    char webSocketKey[61];
    Stream* _s;
    char newLineCount;
};

#endif
