#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LED 2

QueueHandle_t q;

void blinky(void* pvParameter){
	int state = 0;
	int delay;
	while(1){
		if(xQueueReceive(q, (void *)&delay, 0) == pdTRUE){
			printf("Received delay %d from the queue\n", delay);
		}
		state = !state;
		gpio_set_level(LED, state);
		vTaskDelay(delay / portTICK_PERIOD_MS);
	}
}

void reader(void* pvParameter){
	int delay;
	while(1){
		delay = rand() % 1000;
		if(xQueueSend(q, (void *)&delay, 1000) == pdTRUE){
			printf("Sent delay %d to the queue\n", delay);
		}
	}
}

void app_main(void)
{
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	q = xQueueCreate(5, sizeof(int));
	int init_delay = 1000;
	xQueueSend(q, (void *)&init_delay, 0);
	xTaskCreate(blinky, "Blinky", 2048, NULL, 5, NULL);
	xTaskCreate(reader, "Reader", 2048, NULL, 5, NULL);
}

