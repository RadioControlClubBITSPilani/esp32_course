#include "wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_netif.h"
#include "esp_wifi_default.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

#define MAX_RETRY 5

static EventGroupHandle_t s_wifi_event_group;
static
const char * TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void * arg, esp_event_base_t event_base,
  int32_t event_id, void * event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < MAX_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAILURE);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t * event = (ip_event_got_ip_t * ) event_data;
    ESP_LOGI(TAG, "got ip:"
      IPSTR, IP2STR( & event -> ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_SUCCESS);
  }
}

int connect_wifi(const char * ssid, const char * password) {
  int status = WIFI_FAILURE;
  // initialise network interface
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init( & config));

  s_wifi_event_group = xEventGroupCreate();
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    ESP_EVENT_ANY_ID, &
    event_handler,
    NULL, &
    instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    IP_EVENT_STA_GOT_IP, &
    event_handler,
    NULL, &
    instance_got_ip));

  wifi_config_t c = {
    .sta = {
      .threshold = {
        .authmode = WIFI_AUTH_WPA2_PSK
      },
      .pmf_cfg = {
        .capable = true,
        .required = false
      }
    }
  };
  snprintf((char * ) c.sta.ssid, sizeof(c.sta.ssid), "%s", ssid);
  snprintf((char * ) c.sta.password, sizeof(c.sta.password), "%s", password);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, & c));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
    WIFI_SUCCESS | WIFI_FAILURE,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
   * happened. */
  if (bits & WIFI_SUCCESS) {
    ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid, password);
    status = WIFI_SUCCESS;
  } else if (bits & WIFI_FAILURE) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, password);
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }

  return status;
}
