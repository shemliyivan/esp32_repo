#pragma once

#include "esp_err.h"

typedef enum {
    WIFI_STATE_OFF,           
    WIFI_STATE_STA_CONNECTING,
    WIFI_STATE_STA_ERROR,     
    WIFI_STATE_STA_GOT_IP,    
    WIFI_STATE_INTERNET_OK,   
    WIFI_STATE_AP_STARTED,    
    WIFI_STATE_AP_CONNECTED   
} wifi_app_state_t;

extern volatile wifi_app_state_t g_wifi_state;

void wifi_init_global(void);      
void wifi_start_ap(void);         
void wifi_start_sta(void);        
void wifi_stop_mode(void);