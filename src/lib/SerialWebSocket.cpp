#include "SerialWebSocket.h"
#include "sha1.h"
#include "Base64.h"

const char _webSocketKey[] PROGMEM = "------------------------258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const char _getMsg[] PROGMEM = "GET /websocket HTTP/1.1";
const char _keyHeader[] PROGMEM = "Sec-WebSocket-Key: ";

SerialWebSocket::SerialWebSocket(Stream &s){
  strcpy_P(webSocketKey, _webSocketKey);
  _s = &s;
}

processState_t SerialWebSocket::process(char * buffer, int len){
  processState_t headerRes = processHeaders(buffer, len);
  if(headerRes == SERWS_HEADERS_PROCESSED){
    return SERWS_HEADERS_PROCESSED;
  }else if(wsState == SERWS_READY){
    // It's most likely a WebSocket frame
    return processWSFrame(buffer, len);
  }
  return SERWS_NOT_PROCESSED;
}

processState_t SerialWebSocket::processHeaders(char * buffer, int len){
  // Handle newlines so we can send the response
  if(len > 0 && wsState != SERWS_READY && (buffer[len-1] == '\n' || buffer[len-1] == '\r')){
    newLineCount++;
    if(wsState == SERWS_RESPOND && newLineCount == 4){
      // When we receive the header delimiter then respond with the correct headers
      wsState = SERWS_READY;
      sendHandshake();
    }
    return SERWS_HEADERS_PROCESSED;
  }else{
    newLineCount = 0;
  }
  
  if(len >= 23 && !strncmp_P(buffer, _getMsg, 23)){
    // The client is attempting to open a websocket
    wsState = SERWS_HEADERS;
    return SERWS_HEADERS_PROCESSED;
  }else if(len == 43 && !strncmp_P(buffer, _keyHeader, 19)){
    // Grab the security key for the handshake
    strncpy(webSocketKey, &buffer[19], 24);
    wsState = SERWS_RESPOND;
    return SERWS_HEADERS_PROCESSED;
  }
  
  return SERWS_HEADERS_NOT_PROCESSED;
}

void SerialWebSocket::send(ArduinoJson::JsonObject &outMsg){
  _s->write(0x81);
  _s->write(outMsg.measureLength() & B01111111);
  outMsg.printTo(*_s);
}

processState_t SerialWebSocket::processWSFrame(char * buffer, int len){
  boolean fin = false;
  uint8_t opcode = 0;
  boolean mask_set = false;
  uint8_t length = 0;
  uint8_t mask[4] = {0,0,0,0};
  
  if(wsState == SERWS_READY && len > 6){
    // byte 1
    fin = buffer[0] >> 7;
    opcode = buffer[0] & 0x0F;
    
    if(fin != 1 || (opcode != 0x01 && opcode != 0x08)){
      //It's not a websocket frame or it's not final
      return SERWS_FRAME_ERROR;
    }
    
    //byte 2
    mask_set = buffer[1] >> 7;
    length = buffer[1] & 0x7F;
    if(len >= (length + 6)){
      if(length < 125){
        //extract the mask
        if(mask_set){
          for(char i = 0; i<4; i++){
            mask[i] = buffer[i + 2];
          }
        }
        
        if(opcode == 0x08){
          // The socket is closing, we need to send a close frame in response
          _s->write(0x88);
          _s->write(0x02);
          _s->write(0x03);
          _s->write(0xe9);
          return SERWS_FRAME_EMPTY;
        }else{
          //process the message into the buffer without the websocket frame data
          for(uint8_t i=0; i<length; i++){
            buffer[i] = (char)(buffer[i+6] ^ mask[i % 4]);
          }
          buffer[length] = '\0';
          // The frame has been fully processed
          return SERWS_FRAME_PROCESSED;
        }
      }else{
        // Message too long
        return SERWS_FRAME_ERROR;
      }
    }
  }
  return SERWS_FRAME_NOT_PROCESSED;
}

void SerialWebSocket::sendHandshake(){
  uint8_t *hash;
  char result[21];
  char b64Result[30];

  Sha1.init();
  Sha1.print(webSocketKey);
  hash = Sha1.result();

  for (int i=0; i<20; ++i) {
    result[i] = (char)hash[i];
  }
  result[20] = '\0';

  base64_encode(b64Result, result, 20);
  
  _s->print(F("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "));
  _s->print(b64Result);
  _s->print(F("\r\n\r\n"));
  newLineCount = 0;
  wsState = SERWS_READY;
}