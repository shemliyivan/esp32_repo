#include "my_wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_http_server.h"
#include "ping/ping_sock.h"

static const char *TAG = "MY_WIFI";

#define STA_SSID "SSID"
#define STA_PASS "Pass"
#define AP_SSID  "ESP32_CONFIG_AP"
#define AP_PASS  "12345678"
#define MAX_RETRY 5

volatile wifi_app_state_t g_wifi_state = WIFI_STATE_OFF;

static int s_retry_num = 0;
static httpd_handle_t s_server = NULL;

static esp_err_t root_get_handler(httpd_req_t *req) {
    const char *resp = "<h1>ESP32 Config</h1><p>Mode: AP/STA</p>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static void start_webserver(void) {
    if (s_server) return;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
        httpd_register_uri_handler(s_server, &root);
    }
}

static void stop_webserver(void) {
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
}

static void ping_success_cb(esp_ping_handle_t hdl, void *args) {
    g_wifi_state = WIFI_STATE_INTERNET_OK;
    ESP_LOGI(TAG, "Internet Access OK");
}

static void check_internet(void) {
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
    config.count = 3;
    esp_ping_callbacks_t cbs = { .on_ping_success = ping_success_cb };
    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);
}

static void init_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    ESP_LOGI(TAG, "SNTP Initialized");
}

static void event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        g_wifi_state = WIFI_STATE_STA_CONNECTING; // Yellow
    } 
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            g_wifi_state = WIFI_STATE_STA_CONNECTING;
            ESP_LOGI(TAG, "Retry connecting...");
        } else {
            g_wifi_state = WIFI_STATE_STA_ERROR; // Red
            ESP_LOGE(TAG, "Connection Failed");
        }
    } 
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        g_wifi_state = WIFI_STATE_STA_GOT_IP; // Green Blink
        ESP_LOGI(TAG, "Got IP. Starting services...");
        init_sntp();
        check_internet(); // Перехід в Green Solid якщо успішно
    }
    // === AP LOGIC ===
    else if (base == WIFI_EVENT && id == WIFI_EVENT_AP_START) {
        g_wifi_state = WIFI_STATE_AP_STARTED; // Blue Blink
        start_webserver();
    }
    else if (base == WIFI_EVENT && id == WIFI_EVENT_AP_STACONNECTED) {
        g_wifi_state = WIFI_STATE_AP_CONNECTED; // Blue Solid
        ESP_LOGI(TAG, "Client connected to AP");
    }
    else if (base == WIFI_EVENT && id == WIFI_EVENT_AP_STADISCONNECTED) {
        g_wifi_state = WIFI_STATE_AP_STARTED; // Blue Blink
    }
}

void wifi_init_global(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));
}

void wifi_stop_mode(void) {
    esp_wifi_stop();
    stop_webserver();
    esp_sntp_stop();
    g_wifi_state = WIFI_STATE_OFF;
}

void wifi_start_sta(void) {
    wifi_stop_mode();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    wifi_config_t conf = {
        .sta = {
            .ssid = STA_SSID,
            .password = STA_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_config(WIFI_IF_STA, &conf);
    esp_wifi_start();
    s_retry_num = 0;
}

void wifi_start_ap(void) {
    wifi_stop_mode();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t conf = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASS,
            .ssid_len = strlen(AP_SSID),
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    esp_wifi_set_config(WIFI_IF_AP, &conf);
    esp_wifi_start();
}