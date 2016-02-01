#ifndef CmdProcessor_h
#define CmdProcessor_h

#include "Arduino.h"

class Mirobot;

#include "Mirobot.h"

#define INPUT_BUFFER_LENGTH 180

typedef enum {EXPECT_ATTR, ATTR, DELIM, VAL} parseState_t;
typedef enum {WAITING, WEBSOCKET_INIT, WEBSOCKET_RESPOND, WEBSOCKET_READY, HTTP_INIT, HTTP_RESPOND, HTTP_READY} httpState_t;
typedef enum {RAW, HTTP, WEBSOCKET} socketMode_t;
typedef enum {AT_CLOSED, AT_START, AT_OPENING, AT_READY, AT_RECEIVING, AT_CLOSING, AT_FLUSHING} atState_t;

typedef void (* fp_main) (void *, char *);
typedef boolean (* fp_ready) (void *);

class CmdProcessor {
  public:
    CmdProcessor();
    void addUserCmd(char* cmd, fp_main fn, fp_ready ready);
    void setup(Stream &s, Mirobot &m);
    void process();
    void collideNotify(const char msg[]);
    void followNotify(int state);
  private:
    boolean processLine();
    void extractAttr(const char attr[4], char *json, char *output, char len);
    void processCmd(char &cmd, char &arg, char &id);
    void runCmd(char &cmd, char &arg, char &id);
    void sendResponse(const char state[], const char msg[], char &id);
    Stream* _s;
    Mirobot* _m;
    httpState_t httpState;
    socketMode_t socketMode;
    char webSocketKey[61];
    char newLineCount;
    char input_buffer[INPUT_BUFFER_LENGTH];
    byte input_buffer_pos;
    boolean in_process;
    atState_t at_cmd_state;
    char current_id[11];
    unsigned long last_char;
    boolean processWSFrame();
    boolean processJSON();
    boolean processHeaders();
    void startAtCmdReset();
    void handleAtCmds(char incomingByte);
    void resetCheck();
    long resetTimeout;
};

#endif
