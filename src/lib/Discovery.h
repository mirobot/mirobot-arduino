#ifndef __discovery_h__
#define __discovery_h__

#include "Marceau.h"
extern "C" {
#include "lwip/inet.h"
#include "espconn.h"
}

#define DISCOVERY_HOST "local.mirobot.io"

void send_discovery_request(uint32_t, char *, char *);

#endif
