#ifdef ESP8266
#include "Discovery.h"

static uint32_t device_ip;
static char device_name[33];
static char device_type[20];

static void ICACHE_FLASH_ATTR discoveryReconCb(void *arg, int8_t err) {}
static void ICACHE_FLASH_ATTR discoveryDisconCb(void *arg) {}
static void ICACHE_FLASH_ATTR discoverySentCb(void *arg) {}
static void ICACHE_FLASH_ATTR discoveryRecvCb(void *arg, char *data, unsigned short len) {}

static void ICACHE_FLASH_ATTR discoveryConnectedCb(void *arg) {
  char httpStr[128];
  static uint8_t macaddr[6];
  struct espconn *conn=(struct espconn *)arg;
  os_sprintf(httpStr, "POST /?address=" IPSTR "&name=%s&type=%s HTTP/1.1\r\nHost: %s\r\n\r\n", (uint8_t)(device_ip & 0xFF), (uint8_t)((device_ip >> 8) & 0xFF), (uint8_t)((device_ip >> 16) & 0xFF), (uint8_t)((device_ip >> 24) & 0xFF), device_name, device_type, DISCOVERY_HOST);
  espconn_sent(conn, (uint8 *)httpStr, strlen(httpStr));
}

static void ICACHE_FLASH_ATTR DNSFoundCb(const char *name, ip_addr_t *ip, void *arg) {
  static esp_tcp tcp;
  struct espconn *conn=(struct espconn *)arg;
  if (ip==NULL) {
    os_printf("DNS lookup failed\n");
    return;
  }
 
  os_printf("Found DNS: %d.%d.%d.%d\n",
  *((uint8 *)&ip->addr), *((uint8 *)&ip->addr + 1),
  *((uint8 *)&ip->addr + 2), *((uint8 *)&ip->addr + 3));
 
  conn->type=ESPCONN_TCP;
  conn->state=ESPCONN_NONE;
  conn->proto.tcp=&tcp;
  conn->proto.tcp->local_port=espconn_port();
  conn->proto.tcp->remote_port=80;
  os_memcpy(conn->proto.tcp->remote_ip, &ip->addr, 4);
  espconn_regist_connectcb(conn, discoveryConnectedCb);
  espconn_regist_disconcb(conn, discoveryDisconCb);
  espconn_regist_reconcb(conn, discoveryReconCb);
  espconn_regist_recvcb(conn, discoveryRecvCb);
  espconn_regist_sentcb(conn, discoverySentCb);
  espconn_connect(conn);
}

void send_discovery_request(uint32_t _ip, char * _name, char * _type){
  static struct espconn conn;
  static ip_addr_t ip;
  device_ip = _ip;
  strcpy(device_name, _name);
  strcpy(device_type, _type);
  os_printf("Fetching DNS\n");
  espconn_gethostbyname(&conn, "local.mirobot.io", &ip, DNSFoundCb);
}

#endif
