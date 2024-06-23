#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED1 32
#define LED2 33

typedef struct{
	int GPIO;
	int state;
	int duration;
} led_params;


void blink(void *pvParameters){
	led_params led = *(led_params *)pvParameters;
	while (1){
		led.state = !led.state;
		gpio_set_level(led.GPIO, led.state);	
		// waiting
		vTaskDelay(led.duration / portTICK_PERIOD_MS);
	}
}

void app_main(void){
	gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
	gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

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
}

