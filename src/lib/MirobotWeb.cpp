#ifdef ESP8266
#include "Arduino.h"
#include "MirobotWeb.h"
#include "web_pages.h"

ESP8266WebServer server(80);

class MirobotRequestHandler : public RequestHandler {
  public:
    MirobotRequestHandler(){}

    bool canHandle(HTTPMethod requestMethod, String requestUri) override  {
      return requestMethod == HTTP_GET;
    }

    bool canUpload(String requestUri) override  {
      return false;
    }

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override {
      uint8_t i;
      bool found = false;
      if (!canHandle(requestMethod, requestUri)) return false;
      for(i=0; i<fileCount; i++){
        if(requestUri == files[i].filename || requestUri + "index.html" == files[i].filename){
          server.send_P(200, files[i].mime, files[i].content, files[i].len);
          found = true;
          break;
        }
      }
      if(!found) server.send(404, "text/plain", "Not Found");
      return true;
    }
};

MirobotWeb::MirobotWeb() {
  server.addHandler(new MirobotRequestHandler());
	server.begin();
}

void MirobotWeb::run(){
  server.handleClient();
}

#endif
