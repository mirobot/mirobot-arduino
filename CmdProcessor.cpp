#include "CmdProcessor.h"

CmdProcessor::CmdProcessor(){
  socketType = NO_SOCKET;
  in_process = false;
}

void CmdProcessor::setup(Stream &s, Mirobot &m){
  _s = &s;
  Serial.begin(115200);
  sendResponse("boot", "Starting Mirobot", *current_id);
  _m = &m;
}

void CmdProcessor::process(){
  // check if the previous command is ready
  if(in_process && _m->ready()){
    in_process = false;
    sendResponse("complete", "", *current_id);
  }
  if (_s->available() > 0){
    char incomingByte = _s->read();
    if(incomingByte == '\r' || incomingByte == '\n' || incomingByte == '\0'){
      if(input_buffer_pos > 0){
        // we have a message in the buffer so set this to end the string
        input_buffer[input_buffer_pos] = '\0';
        // store the command in the command table
        processLine();
        //reset the buffer because we've processed this line
        input_buffer_pos = 0;
      }
    }else{
      // add the character to the buffer
      input_buffer[input_buffer_pos] = incomingByte;
      input_buffer_pos++;
    }
  }
}

void CmdProcessor::processLine(){
  char cmd[10], arg[10], id[10];
  //do a simple check to see if it looks like json
  if(input_buffer[0] == '{' && input_buffer[input_buffer_pos - 1] == '}'){
    extractAttr("cmd", input_buffer, cmd);
    extractAttr("arg", input_buffer, arg);
    extractAttr("id", input_buffer, id);
    processCmd(*cmd, *arg, *id);
  }else{
    //this is where we can handle the websocket handshake
  }
}

void CmdProcessor::processCmd(char &cmd, char &arg, char &id){
  if(in_process){
    // the previous command hasn't finished, send an error
    sendResponse("error", "Previous command not finished", id);
  }else{
    strcpy(current_id, &id);
    in_process = true;
    if(!strcmp(&cmd, "forward")){
      _m->forward(atoi(&arg));
    }else if(!strcmp(&cmd, "back")){
      _m->back(atoi(&arg));
    }else if(!strcmp(&cmd, "right")){
      _m->right(atoi(&arg));
    }else if(!strcmp(&cmd, "left")){
      _m->left(atoi(&arg));
    }else if(!strcmp(&cmd, "penup")){
      _m->penup();
    }else if(!strcmp(&cmd, "pendown")){
      _m->pendown();
    }else{
      // the command isn't recognised, send an error
      sendResponse("error", "Command not recognised", id);
      return;
    }
    sendResponse("accepted", "", *current_id);
  }
}

void CmdProcessor::sendResponse(const char state[], const char msg[], char &id){
  _s->print("{\"status\":\"");
  _s->print(state);
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
  _s->println("}");
}

//This is a very naive JSON parser which will extract the value of a specific attribute
void CmdProcessor::extractAttr(const char attr[], char *json, char *output){
  parseState_t parseState = EXPECT_ATTR;
  char attrPos = 0;
  boolean match = false;
  
  while(*json != '\0'){
    switch(parseState){
      case EXPECT_ATTR:
        if(*json == '"'){
          parseState = ATTR;
          attrPos = 0;
        }
        break;
      case ATTR:
        if(*json == '"'){
          parseState = EXPECT_VAL;
        }else{
          //check the attribute
          if(attr[attrPos] == *json){
            attrPos++;
            if(attr[attrPos] == '\0'){
              match = true;
            }
          }
        }
        break;
      case EXPECT_VAL:
        if(*json == '"'){
          parseState = VAL;
        }
        break;
      case VAL:
        if(*json == '"'){
          if(match){
            *output = '\0';
            return;
          }else{
            parseState = EXPECT_ATTR;
          }
        }else{
          //copy the attribute if it's a match
          if(match){
            *output = *json;
            *output++;
          }
        }
        break;
    }
        
    *json++;
  }
}