#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#define LED 2  //onboard led


void app_main(void)
{
	int led_state = 0;
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	while(1){
		gpio_set_level(LED, led_state);
		led_state = !led_state;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
