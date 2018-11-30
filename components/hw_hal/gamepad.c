
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "sdkconfig.h"


// Set buttons with pins
#define A       CONFIG_GAMEPAD_A
#define B       CONFIG_GAMEPAD_B
#define START   CONFIG_GAMEPAD_START
#define SELECT  CONFIG_GAMEPAD_SELECT
#define UP      CONFIG_GAMEPAD_UP
#define DOWN    CONFIG_GAMEPAD_DOWN
#define LEFT    CONFIG_GAMEPAD_LEFT
#define RIGHT   CONFIG_GAMEPAD_RIGHT


int gpdReadInput()
{
	return 0xFFFF ^ ((!gpio_get_level(A) << 13) | (!gpio_get_level(B) << 14) |
			(!gpio_get_level(START) << 3) | (!gpio_get_level(SELECT) << 0) |
			(!gpio_get_level(UP) << 4) | (!gpio_get_level(DOWN) << 6) |
			(!gpio_get_level(LEFT) << 7) | (!gpio_get_level(RIGHT) << 5));
}

void gamepadInit()
{
	//Configure button
	gpio_config_t btn_config;
	btn_config.intr_type = GPIO_INTR_ANYEDGE;        //Enable interrupt on both rising and falling edges
	btn_config.mode = GPIO_MODE_INPUT;               //Set as Input
	btn_config.pin_bit_mask = (uint64_t)             //Bitmask
		((uint64_t) 1 << A) | ((uint64_t) 1 << B) | ((uint64_t) 1 << START) | ((uint64_t) 1 << SELECT) |
		((uint64_t) 1 << UP) | ((uint64_t) 1 << DOWN) | ((uint64_t) 1 << LEFT) | ((uint64_t) 1 << RIGHT);

	btn_config.pull_up_en = GPIO_PULLUP_ENABLE;      //Disable pullup
	btn_config.pull_down_en = GPIO_PULLDOWN_DISABLE; //Enable pulldown
	gpio_config(&btn_config);
}
