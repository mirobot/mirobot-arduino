#ifndef __CmdProcessor_h__
#define __CmdProcessor_h__

#include "Arduino.h"

class Mirobot;
class CmdProcessor;

#include "Mirobot.h"
#include "./lib/ArduinoJson/ArduinoJson.h"

#define CMD_COUNT 31
#define JSON_BUFFER_LENGTH 128
#define OUTPUT_HANDLER_COUNT 2

typedef void (Mirobot::*MirobotMemFn)(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
typedef void (* fp) (void *, char *);
typedef boolean (* fp_ready) (void *);
typedef void (* msgHandler) (ArduinoJson::JsonObject &);

struct Cmd {
  const char *cmd;
  MirobotMemFn func;
  bool immediate;
};

class CmdProcessor {
  public:
    CmdProcessor();
    void addCmd(const char cmd[], MirobotMemFn func, bool immediate);
    bool addOutputHandler(msgHandler);
    void process();
    void notify(const char[], ArduinoJson::JsonObject &);
    void setMirobot(Mirobot &);
    void sendComplete();
    boolean processMsg(char * msg);
    boolean in_process;
  private:
    boolean processLine();
    void processCmd(const char &cmd, const char &arg, const char &id);
    void runCmd(char &cmd, char &arg, char &id);
    void sendResponse(const char state[], ArduinoJson::JsonObject &, const char &id);
    char webSocketKey[61];
    char current_id[11];
    boolean processJSON();
    Mirobot* _m;
    Cmd _cmds[CMD_COUNT];
    int cmd_counter;
    msgHandler outputHandlers[OUTPUT_HANDLER_COUNT];
};


#endif
