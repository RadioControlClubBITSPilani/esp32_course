#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "esp_log.h"

#include "led.h"
#include "wifi.h"

#define LED1 32
#define LED2 33

#define SSID "your_wifi_ssid"
#define password "123456"

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char buf[2048];
    int received = 0;

    if (total_len >= 2048) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }

    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("BLINKY", "Request Body: %s", buf);

    cJSON *root = cJSON_Parse(buf);
    char* led = cJSON_GetObjectItem(root, "led")->valuestring;
    char* state = cJSON_GetObjectItem(root, "state")->valuestring;

    TaskHandle_t handle = xTaskGetHandle(led);
    if (strcmp(state, "pause") == 0){
	vTaskSuspend(handle);
    } else if (strcmp(state, "resume") == 0) {
	vTaskResume(handle);
    }
    ESP_LOGI("BLINKY", "Led %s set to %s", led, state);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

void app_main(void){
	gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
	gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	int status = connect_wifi(SSID, password);
	if (status != WIFI_SUCCESS){
		printf("Failed to connect to wifi");
		return;
	}

	led_params *led1 = malloc(sizeof(led_params));
	led1->GPIO = LED1;
	led1->state = 0;
	led1->duration = 200;

	led_params *led2 = malloc(sizeof(led_params));
	led2->GPIO = LED2;
	led2->state = 0;
	led2->duration = 300;
	
	xTaskCreate(&blink, "Blinky1", 2048, led1, 5, NULL);
	xTaskCreate(&blink, "Blinky2", 2048, led2, 5, NULL);

	httpd_uri_t led_post = {
	    .uri      = "/led",
	    .method   = HTTP_POST,
	    .handler  = post_handler,
	};

	/* Generate default configuration */
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	/* Empty handle to esp_http_server */
	httpd_handle_t server = NULL;
	
	/* Start the httpd server */
	if (httpd_start(&server, &config) == ESP_OK) {
	    /* Register URI handlers */
	    httpd_register_uri_handler(server, &led_post);
	}

	free(led1);
	free(led2);
}
