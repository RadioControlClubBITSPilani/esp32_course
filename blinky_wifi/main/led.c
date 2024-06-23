#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "portmacro.h"


void blink(void *pvParameters){
	led_params led = *(led_params *)pvParameters;
	while (1){
		led.state = !led.state;
		gpio_set_level(led.GPIO, led.state);	
		vTaskDelay(led.duration / portTICK_PERIOD_MS);
	}
}
