#ifndef __CmdManager_h__
#define __CmdManager_h__

#include "Arduino.h"

class Mirobot;
class CmdManager;

#include "Mirobot.h"
#include "./lib/ArduinoJson/ArduinoJson.h"

#define INPUT_BUFFER_LENGTH 500
#define CMD_COUNT 30

typedef enum {JSON_EXPECT_JSON_ATTR, JSON_ATTR, JSON_JSON_DELIM, JSON_VAL} jsonParseState_t;

typedef void (Mirobot::*MirobotMemFn)(ArduinoJson::JsonObject &, ArduinoJson::JsonObject &);
typedef void (* fp) (void *, char *);
typedef boolean (* fp_ready) (void *);

struct Cmd {
  const char *cmd;
  MirobotMemFn func;
  bool immediate;
};

class CmdManager {
  public:
    CmdManager();
    void addCmd(const char cmd[], MirobotMemFn func, bool immediate);
    void addStream(Stream &s);
    void process();
    void notify(const char[], ArduinoJson::JsonObject &);
    void setMirobot(Mirobot &);
    void sendComplete();
    boolean in_process;
  private:
    boolean processLine();
    void processCmd(const char &cmd, const char &arg, const char &id);
    void runCmd(char &cmd, char &arg, char &id);
    void sendResponse(const char state[], ArduinoJson::JsonObject &, const char &id);
    Stream* _s;
    char webSocketKey[61];
    char input_buffer[INPUT_BUFFER_LENGTH];
    byte input_buffer_pos;
    char current_id[11];
    unsigned long last_char;
    boolean processJSON();
    Mirobot* _m;
    Cmd _cmds[CMD_COUNT];
    int cmd_counter;
};


#endif
