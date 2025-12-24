#ifndef TCPWIFI_H_
#define TCPWIFI_H

#include "bsp.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

static void wifiEventHandler(void* arg, esp_event_base_t eventBase, 
                            int32_t eventId, void* eventData);

static void ipEventHandler(void* arg, esp_event_base_t eventBase,
                            int32_t eventId, void* eventData);

esp_err_t connectWifi();

esp_err_t startTCPClient(int *socket_ptr);

int wifi();

#endif