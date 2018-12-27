/**
 * @file st7735r.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "st7735r.h"
#include "disp_spi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lcd_struct.h"
#include "driver/ledc.h"
#include "driver/rtc_io.h"

/*********************
 *      DEFINES
 *********************/
#define MADCTL_MY       0x80
#define MADCTL_MX       0x40
#define MADCTL_MV       0x20
#define MADCTL_ML       0x10
#define MADCTL_RGB      0x00
#define MADCTL_BGR      0x08
#define MADCTL_MH       0x04

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void st7735r_send_cmd(uint8_t cmd);
static void st7735r_send_data(void * data, uint16_t length);
static void backlight_init();

/**********************
 *  STATIC VARIABLES
 **********************/
static const int DUTY_MAX = 0x1fff;
static const int LCD_BACKLIGHT_ON_VALUE = 1;
static bool isBackLightIntialized = false;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void st7735r_init(void)
{
    lcd_init_cmd_t st_init_cmds[]={
        {0x01, {0}, 0x80}, //SWRESET
        {0x11, {0}, 0x80}, //SLEEPOUT
        {0xB1, {0x01, 0x2C, 0x2D}, 3}, //FRMCTL1 porch padding
        {0xB2, {0x01, 0x2C, 0x2D}, 3}, //FRMCTL2 porch padding
        {0xB3, {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6}, //FRMCTL3 porch padding
        {0xB4, {0x07}, 1}, //INVCTR
        {0xC0, {0xA2, 0x02, 0x84}, 3}, //PWCTR1
        {0xC1, {0xC5}, 1}, //PWCTR2
        {0xC2, {0x0A, 0x00}, 2}, //PWCTR3
        {0xC3, {0x8A, 0x2A}, 2}, //PWCTR4
        {0xC4, {0x8A, 0xEE}, 2}, //PWCTR5
        {0xC5, {0x0E}, 1}, //VMCTR1
        {0x20, {0}, 0}, //INVOFF
        {0x36, {(MADCTL_MV)|(MADCTL_MX)}, 1}, //MADCTL RGB, MV, MX, MY
        {0x3A, {0x05}, 1|0x80}, //COLMOD
        {0x2A, {0x00, 0x02, 0x00, 0x81}, 4}, //CASET
        {0x2B, {0x00, 0x01, 0x00, 0xA0}, 4}, //RASET
        //commented values are for blacktab, good luck!
        //   {0xE0, {0x09, 0x16, 0x09, 0x20, 0x21, 0x1B, 0x13, 0x19, 0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E}, 16},
        //   {0xE1, {0x0B, 0x14, 0x08, 0x1E, 0x22, 0x1D, 0x18, 0x1E, 0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F}, 16},
        {0xE0, {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10}, 16},
        {0xE1, {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10}, 16},
        {0x13, {0}, 0x0}, //NORON
        {0x29, {0}, 0x80}, //DISPON
        {0, {0}, 0xff}
    };

    //Initialize non-SPI GPIOs
    gpio_set_direction(ST7735R_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(ST7735R_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(ST7735R_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(ST7735R_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    printf("st7735r initialization.\n");

    //Send all the commands
    uint16_t cmd = 0;
    while (st_init_cmds[cmd].databytes!=0xff) {
        st7735r_send_cmd(st_init_cmds[cmd].cmd);
        st7735r_send_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes&0x1F);
        if (st_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    //Enable backlight
    backlight_init();
}

int is_st_backlight_initialized()
{
    return isBackLightIntialized;
}

void st_backlight_percentage_set(int value)
{
    int duty = DUTY_MAX * (value * 0.01f);

    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 500);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}


#if CONFIG_USE_LVGL_LIBRARY
void st7735r_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
    uint8_t data[4];

    //Column addresses
    st7735r_send_cmd(0x2A);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    st7735r_send_data(data, 4);

    //Page addresses
    st7735r_send_cmd(0x2B);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    st7735r_send_data(data, 4);

    //Memory write
    st7735r_send_cmd(0x2C);

    uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
    uint16_t color_swap = ((color.full >> 8) & 0xFF) | ((color.full & 0xFF) << 8);  //It's a 8 bit SPI bytes need to be swapped
    uint16_t buf[ST7735R_HOR_RES];

    uint32_t i;
    if(size < ST7735R_HOR_RES) {
        for(i = 0; i < size; i++) buf[i] = color_swap;

    } else {
        for(i = 0; i < ST7735R_HOR_RES; i++) buf[i] = color_swap;
    }

    while(size > ST7735R_HOR_RES) {
        st7735r_send_data(buf, ST7735R_HOR_RES * 2);
        size -= ST7735R_HOR_RES;
    }

    st7735r_send_data(buf, size * 2);   //Send the remaining data
}


void st7735r_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map)
{
    uint8_t data[4];

    //Column addresses
    st7735r_send_cmd(0x2A);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    st7735r_send_data(data, 4);

    //Page addresses
    st7735r_send_cmd(0x2B);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    st7735r_send_data(data, 4);

    //Memory write
    st7735r_send_cmd(0x2C);

    uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);

    //Byte swapping is required
    uint32_t i;
    uint8_t * color_u8 = (uint8_t *) color_map;
    uint8_t color_tmp;
    for(i = 0; i < size * 2; i += 2) {
        color_tmp = color_u8[i + 1];
        color_u8[i + 1] = color_u8[i];
        color_u8[i] = color_tmp;
    }


    while(size > ST7735R_HOR_RES) {

        st7735r_send_data((void*)color_map, ST7735R_HOR_RES * 2);
        size -= ST7735R_HOR_RES;
        color_map += ST7735R_HOR_RES;
    }

    st7735r_send_data((void*)color_map, size * 2);  //Send the remaining data
    lv_flush_ready();
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void st7735r_send_cmd(uint8_t cmd)
{
    disp_spi_send(&cmd, 1, CMD_ON);
}

static void st7735r_send_data(void * data, uint16_t length)
{
    disp_spi_send(data, length, DATA_ON);
}

static void backlight_init()
{
    //configure timer0
    ledc_timer_config_t ledc_timer;
    memset(&ledc_timer, 0, sizeof(ledc_timer));

    ledc_timer.bit_num = LEDC_TIMER_13_BIT; //set timer counter bit number
    ledc_timer.freq_hz = 5000;              //set frequency of pwm
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;   //timer mode,
    ledc_timer.timer_num = LEDC_TIMER_0;    //timer index

    ledc_timer_config(&ledc_timer);

    //set the configuration
    ledc_channel_config_t ledc_channel;
    memset(&ledc_channel, 0, sizeof(ledc_channel));

    //set LEDC channel 0
    ledc_channel.channel = LEDC_CHANNEL_0;
    //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
    ledc_channel.duty = (LCD_BACKLIGHT_ON_VALUE) ? 0 : DUTY_MAX;
    //GPIO number
    ledc_channel.gpio_num = ST7735R_BCKL;
    //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
    ledc_channel.intr_type = LEDC_INTR_FADE_END;
    //set LEDC mode, from ledc_mode_t
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    //set LEDC timer source, if different channel use one timer,
    //the frequency and bit_num of these channels should be the same
    ledc_channel.timer_sel = LEDC_TIMER_0;

    ledc_channel_config(&ledc_channel);

    //initialize fade service.
    ledc_fade_func_install(0);

    // duty range is 0 ~ ((2**bit_num)-1)
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (LCD_BACKLIGHT_ON_VALUE) ? DUTY_MAX : 0, 500);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    isBackLightIntialized = true;

    printf("Backlight initialization done.\n");
}
