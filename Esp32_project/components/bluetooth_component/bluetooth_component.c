#include "bluetooth_component.h"
#include "esp_log.h"
#include <string.h>

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/bas/ble_svc_bas.h" 
#include "services/dis/ble_svc_dis.h" 

#include "led_types.h" 
#include "led_strip.h"

static const char *TAG_BLE = "BLE_COMP";
static uint8_t ble_addr_type;

extern led_strip_handle_t strip;
extern led_mode_t g_led_mode;

#define CUSTOM_SVC_UUID      0xBB00 
#define LED_CMD_CHR_UUID     0xBB01 
#define LED_STATUS_CHR_UUID  0xBB02 

static int gatt_svr_chr_access_led(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid = ctxt->chr->uuid;

    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        uint8_t data[3];
        ble_hs_mbuf_to_flat(ctxt->om, data, sizeof(data), &len);

        if (ble_uuid_u16(uuid) == LED_CMD_CHR_UUID) {
            g_led_mode = LED_MODE_MQTT_MANUAL;
            
            if (len == 1) { 
                if (data[0] == 0) {
                    led_strip_clear(strip);
                    ESP_LOGI(TAG_BLE, "LED OFF via BLE");
                } else {
                    led_strip_set_pixel(strip, 0, 255, 255, 255);
                    ESP_LOGI(TAG_BLE, "LED ON (White) via BLE");
                }
            } 
            else if (len == 3) {
                led_strip_set_pixel(strip, 0, data[0], data[1], data[2]);
                ESP_LOGI(TAG_BLE, "LED Color set: R:%d G:%d B:%d", data[0], data[1], data[2]);
            }
            led_strip_refresh(strip);
            return 0;
        }
    }

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        if (ble_uuid_u16(uuid) == LED_STATUS_CHR_UUID) {
            uint8_t state = (g_led_mode == LED_MODE_MQTT_MANUAL) ? 1 : 0;
            return os_mbuf_append(ctxt->om, &state, sizeof(state));
        }
        
        if (ble_uuid_u16(uuid) == 0x2A2B) {
            uint8_t dummy_time[10] = {0}; 
            return os_mbuf_append(ctxt->om, dummy_time, sizeof(dummy_time));
        }
    }

    return BLE_ATT_ERR_UNLIKELY;
}

// Таблиця GATT сервісів
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180A),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0x2A29), 
            .access_cb = gatt_svr_chr_access_led, 
            .flags = BLE_GATT_CHR_F_READ,
        }, { 0 } }
    },

    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x1805),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0x2A2B), 
            .access_cb = gatt_svr_chr_access_led,
            .flags = BLE_GATT_CHR_F_READ,
        }, { 0 } }
    },

    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(CUSTOM_SVC_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { 
            {
                .uuid = BLE_UUID16_DECLARE(LED_CMD_CHR_UUID),
                .access_cb = gatt_svr_chr_access_led,
                .flags = BLE_GATT_CHR_F_WRITE,
            }, 
            {
                .uuid = BLE_UUID16_DECLARE(LED_STATUS_CHR_UUID),
                .access_cb = gatt_svr_chr_access_led,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            }, 
            { 0 } 
        },
    },
    { 0 },
};

static void ble_app_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;

    memset(&fields, 0, sizeof(fields));
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
}

static void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

static void ble_host_task(void *param)
{
    ESP_LOGI(TAG_BLE, "NimBLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void bluetooth_component_init(void)
{
    nimble_port_init();
    
    ble_svc_gap_device_name_set("Ivan-ESP32");
    
    ble_svc_gap_init();
    ble_svc_gatt_init();
    
    ble_svc_bas_init();
    ble_svc_dis_init();
    
    ble_svc_dis_model_number_set("ESP32S3-Ivan-Dev");
    ble_svc_dis_manufacturer_name_set("Ivan-Corp");
    
    ble_svc_bas_battery_level_set(98); 

    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(ble_host_task);
}