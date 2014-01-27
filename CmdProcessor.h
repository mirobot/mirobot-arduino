#ifndef CmdProcessor_h
#define CmdProcessor_h

#include "Arduino.h"

class Mirobot;

#include "Mirobot.h"

#define INPUT_BUFFER_LENGTH 60
#define COMMAND_COUNT  12

#define NO_SOCKET 0
#define RAW_SOCKET 1
#define WEB_SOCKET 2

#define RUNNING 1
#define READY 2
#define HANDSHAKE 3

typedef enum {EXPECT_ATTR, ATTR, EXPECT_VAL, VAL} parseState_t;

typedef void (* fp_main) (void *, char *);
typedef boolean (* fp_ready) (void *);

struct UserCmd {
  const char *cmd;
  byte type;
  fp_main fn_main;
  fp_ready fn_ready;
};

class CmdProcessor {
  public:
    CmdProcessor();
    void addUserCmd(char* cmd, fp_main fn, fp_ready ready);
    void setup(Stream &s, Mirobot &m);
    void process();
    char socketType;
  private:
    void processLine();
    void extractAttr(const char attr[4], char *json, char *output);
    void processCmd(char &cmd, char &arg, char &id);
    void runCmd(char &cmd, char &arg, char &id);
    void sendResponse(const char state[], const char msg[], char &id);
    Stream* _s;
    Mirobot* _m;
    char state;
    char input_buffer[INPUT_BUFFER_LENGTH];
    byte input_buffer_pos;
    UserCmd user_cmds[COMMAND_COUNT];
    byte user_cmd_counter;
    boolean in_process;
    char current_id[10];
};

#endif
