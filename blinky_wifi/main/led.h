#ifndef LED_H
#define LED_H
typedef struct{
	int GPIO;
	int state;
	int duration;
} led_params;

void blink(void *pvParameters);
#endif
