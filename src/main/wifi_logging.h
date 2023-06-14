#ifndef _WIFI_LOGGING_H
#define _WIFI_LOGGING_H

/* This is an untracked file where I store the SSID information */
#include "ssid_info.untracked.h"
#define ESP_WIFI_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define ESP_WIFI_MAXIMUM_RETRY  10

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries
 * (see event group defined in wifi_logging.c -- static EventGroupHandle_t s_wifi_event_group */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* global functions */
void wifi_init_all();

#endif /* _WIFI_LOGGING_H */
