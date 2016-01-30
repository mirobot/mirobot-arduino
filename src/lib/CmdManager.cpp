#include "CmdManager.h"
#include "sha1.h"
#include "Base64.h"

CmdManager::CmdManager(){
  in_process = false;
  current_id[0] = 0;
}

void CmdManager::addStream(Stream &s){
  _s = &s;
}
void CmdManager::setMirobot(Mirobot &m){
  _m = &m;
}
void CmdManager::addCmd(const char cmd[], MirobotMemFn func, bool immediate){
  if (cmd_counter == CMD_COUNT) {
    _s->println("Too many commands defined");
    return;
  }
  _cmds[cmd_counter].cmd = cmd;
  _cmds[cmd_counter].func = func;
  _cmds[cmd_counter].immediate = immediate;
  cmd_counter++;
}

void CmdManager::process(){
  // check if the previous command is ready
  /*
  if(in_process && _m->ready() && at_cmd_state == CLOSED){
    in_process = false;
    sendResponse("complete", "", *current_id);
  }
  */

  // process incoming data
  if (_s->available() > 0){
    last_char = millis();
    char incomingByte = _s->read();
    if((incomingByte == '\r' || incomingByte == '\n') && processLine()){
      // It's been successfully processed as a line
      input_buffer_pos = 0;
    }else{
      // Not a line to process so store for processing
      input_buffer[input_buffer_pos++] = incomingByte;
      input_buffer[input_buffer_pos] = 0;
    }
  }else{
    //reset the input buffer if nothing is received for 1/2 second to avoid things getting messed up
    if(millis() - last_char >= 500){
      input_buffer_pos = 0;
    }
  }
}

boolean CmdManager::processLine(){
  //do a simple check to see if it looks like json
  if(input_buffer_pos > 0 && input_buffer[0] == '{' && input_buffer[input_buffer_pos - 1] == '}'){
    processJSON();
    return true;
  }
  return false;
}

boolean CmdManager::processJSON(){
  char cmd[20], arg[11], id[11];
  if(input_buffer_pos > 0 && input_buffer[0] == '{' && input_buffer[input_buffer_pos - 1] == '}'){
    extractAttr("cmd", input_buffer, cmd, 20);
    extractAttr("arg", input_buffer, arg, 10);
    extractAttr("id", input_buffer, id, 10);
    processCmd(*cmd, *arg, *id);
    return true;
  }
  return false;
}

void CmdManager::processCmd(char &cmd, char &arg, char &id){
  int cmd_num, i;
  char msg[41] = {0,};
  
  cmd_num = -1;
  for(i = 0; i < cmd_counter; i++){
    if(!strcmp(&cmd, _cmds[i].cmd)){
      cmd_num = i;
      break;
    }
  }

  if(cmd_num >= 0){
    if(_cmds[cmd_num].immediate){
      (_m->*(_cmds[cmd_num].func))(arg, *msg);
      sendResponse("complete", msg, id);
    }else{
      if(in_process){
        // the previous command hasn't finished, send an error
        sendResponse("error", "Previous command not finished", id);
      }else{
        (_m->*(_cmds[cmd_num].func))(arg, *msg);
        strcpy(current_id, &id);
        in_process = true;
        sendResponse("accepted", msg, id);
      }
    }
  }else{
    // the command isn't recognised, send an error
    sendResponse("error", "Command not recognised", id);
  }
}

void CmdManager::sendComplete(){
  if(in_process){
    in_process = false;
    sendResponse("complete", "", *current_id);
  }
}

void CmdManager::sendResponse(const char status[], const char msg[], char &id){
  //Calculate the length of the message for websockets and chunked encoding
  unsigned char len = 13 + strlen(status);
  if(strcmp(msg, "")){ len += 10 + strlen(msg); }
  if(strcmp(&id, "")){ len += 9  + strlen(&id); }
  
  _s->print("{\"status\":\"");
  _s->print(status);
  _s->print("\"");

  if(strcmp(msg, "")){
    _s->print(", \"msg\":\"");
    _s->print(msg);
    _s->print("\"");
  }
  if(strcmp(&id, "")){
    _s->print(", \"id\":\"");
    _s->print(&id);
    _s->print("\"");
  }
  _s->print("}");
  _s->print("\r\n");
}

void CmdManager::collideNotify(const char state[]){
  sendResponse("notify", state, (char &)"collide");
}

void CmdManager::followNotify(int state){
  char sensorState[6];
  sprintf(sensorState, "%d", state);
  sendResponse("notify", sensorState, (char &)"follow");
}

//This is a very naive JSON parser which will extract the value of a specific attribute
//It puts a string into the output buffer. Examples of extracting attr:
//  {"attr":"one"} => "one"
//  {"attr":1}     => "1"
//  {"attr":[1, 2]} => "1,2"
//  {"attr":["one",2] "one,2"
void CmdManager::extractAttr(const char attr[], char *json, char *output, char len){
  jsonParseState_t parseState = JSON_EXPECT_JSON_ATTR;
  char attrPos = 0;
  boolean match = false;
  boolean in_brackets = false;
  boolean in_quotes = false;
  
  while(*json != '\0'){
    switch(parseState){
      case JSON_EXPECT_JSON_ATTR:
        if(*json == '"'){
          parseState = JSON_ATTR;
          attrPos = 0;
        }
        break;
      case JSON_ATTR:
        if(*json == '"'){
          *json++;
          while((*json == ':' || *json == ' ') && *json != '\0'){
            *json++;
          }
          parseState = JSON_VAL;
        }else{
          //check the attribute
          if(attr[attrPos] == *json){
            attrPos++;
            if(attr[attrPos] == '\0'){
              match = true;
            }
          }
          break;
        }
      case JSON_VAL:
        if(!in_quotes && !in_brackets && *json == '['){
          in_brackets = true;
        }else if(!in_quotes && *json == '"'){
          in_quotes = true;
        }else if(!in_quotes && in_brackets && *json == ']'){
          in_brackets = false;
        }else if(in_quotes && *json == '"'){
          in_quotes = false;
        }else if(!in_quotes && in_brackets && *json == ' '){
        }else if(!in_quotes && !in_brackets && (*json == ',' || *json == ' ' || *json == '}')){
          if(match){
            *output = '\0';
            return;
          }else{
            parseState = JSON_EXPECT_JSON_ATTR;
          }
        }else{
          //copy the attribute if it's a match
          if(match){
            *output = *json;
            *output++;
            if(!--len){
              *output = '\0';
              return;
            }
          }
        }
        break;
    }
    *json++;
  }
}
