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
  const char* cmd;
  const char* id;
  int cmd_num, i;
  StaticJsonBuffer<INPUT_BUFFER_LENGTH> incomingBuffer;
  StaticJsonBuffer<INPUT_BUFFER_LENGTH> outgoingBuffer;
  JsonObject& outMsg = outgoingBuffer.createObject();
  
  if(input_buffer_pos > 0 && input_buffer[0] == '{' && input_buffer[input_buffer_pos - 1] == '}'){
    JsonObject& inMsg = incomingBuffer.parseObject(input_buffer);
    if(inMsg.success()){
      // Extract the command
      if(!inMsg.containsKey("cmd")) return false;
      cmd = inMsg["cmd"];

      // Extract the ID
      if(inMsg.containsKey("id")){
        id = inMsg["id"];
      }else{
        id = "";
      }

      // Find the command
      cmd_num = -1;
      for(i = 0; i < cmd_counter; i++){
        if(!strcmp(cmd, _cmds[i].cmd)){
          cmd_num = i;
          break;
        }
      }

      // Process the command
      if(cmd_num >= 0){
        if(_cmds[cmd_num].immediate){
          (_m->*(_cmds[cmd_num].func))(inMsg, outMsg);
          sendResponse("complete", outMsg, *id);
        }else{
          if(in_process){
            // the previous command hasn't finished, send an error
            outMsg["msg"] = "Previous command not finished";
            sendResponse("error", outMsg, *id);
          }else{
            (_m->*(_cmds[cmd_num].func))(inMsg, outMsg);
            strcpy(current_id, (char*)id);
            in_process = true;
            sendResponse("accepted", outMsg, *id);
          }
        }
      }else{
        // the command isn't recognised, send an error
        outMsg["msg"] = "Command not recognised";
        sendResponse("error", outMsg, *id);
      }
      
      return true;
    }else{
      //Error parsing
      outMsg["msg"] = "JSON parse error";
      sendResponse("error", outMsg, (const char &)"");
    }
  }
  return false;
}

void CmdManager::sendComplete(){
  if(in_process){
    in_process = false;
    StaticJsonBuffer<60> outBuffer;
    JsonObject& outMsg = outBuffer.createObject();
    sendResponse("complete", outMsg, *current_id);
  }
}

void CmdManager::sendResponse(const char status[], ArduinoJson::JsonObject &outMsg, const char &id){
  if(strlen(&id)){
    outMsg["id"] = &id;
  }
  outMsg["status"] = status;
  outMsg.printTo(*_s);
  _s->println("");
}

void CmdManager::notify(const char id[], ArduinoJson::JsonObject &outMsg){
  sendResponse("notify", outMsg, *id);
}
