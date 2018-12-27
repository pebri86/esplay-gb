#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "display.h"

#define NES_FRAME_WIDTH 256
#define NES_FRAME_HEIGHT 240

#define GB_FRAME_WIDTH 160
#define GB_FRAME_HEIGHT 144

#define U16x2toU32(m,l) ((((uint32_t)(l>>8|(l&0xFF)<<8))<<16)|(m>>8|(m&0xFF)<<8))
#define AVERAGE(a, b) ( ((((a) ^ (b)) & 0xf7deU) >> 1) + ((a) & (b)) )

#define LINE_BUFFERS (2)
#define LINE_COUNT (1)

uint16_t* line[LINE_BUFFERS];
extern uint16_t myPalette[];


void set_display_brightness(int percent)
{
#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ILI)
    backlight_percentage_set(percent);
#endif

#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ST)
    st_backlight_percentage_set(percent);
#endif
}

static uint16_t averageSamples(const uint8_t * data[], int dx, int dy)
{
    uint16_t a,b;
    int y = dy*NES_FRAME_HEIGHT/LCD_HEIGHT;
    int x = dx*NES_FRAME_WIDTH/LCD_WIDTH;
    a = AVERAGE(myPalette[(unsigned char) (data[y][x])],myPalette[(unsigned char) (data[y][x + 1])]);
    b = AVERAGE(myPalette[(unsigned char) (data[y + 1][x])],myPalette[(unsigned char) (data[y + 1][x + 1])]);
    return AVERAGE(a,b);
}

void write_nes_frame(const uint8_t * data[])
{
    short x,y;
    uint16_t a,b;
    int sending_line=-1;
    int calc_line=0;
    for (y=0; y<LCD_HEIGHT; y++) {
        for (x=0; x<LCD_WIDTH; x++) {
            if (data == NULL)
            {
                line[calc_line][x] = 0;
            }
            else
            {
                a = averageSamples(data, x, y);
                b = averageSamples(data, x, y);
                line[calc_line][x]=U16x2toU32(a,b);
            }
        }
        if (sending_line!=-1) send_line_finish();
        sending_line=calc_line;
        calc_line=(calc_line==1)?0:1;
        send_lines(y, LCD_WIDTH, line[sending_line]);
    }
    send_line_finish();
}

//Averages four pixels into one
int getAvgPix(uint16_t* bufs, int pitch, int x, int y) {
    int col;
    if (x<0 || x>=160) return 0;
    //16-bit: E79C
    //15-bit: 739C
    col=(bufs[x+(y*(pitch>>1))]&0xE79C)>>2;
    col+=(bufs[(x+1)+(y*(pitch>>1))]&0xE79C)>>2;
    col+=(bufs[x+((y+1)*(pitch>>1))]&0xE79C)>>2;
    col+=(bufs[(x+1)+((y+1)*(pitch>>1))]&0xE79C)>>2;
    return col&0xffff;
}

void write_gb_frame(const uint16_t * data)
{
    short x,y;
    int sending_line=-1;
    int calc_line=0;
    for (y=0; y<LCD_HEIGHT; y++) {
        for (x=0; x<LCD_WIDTH; x++) {
            if (data == NULL)
            {
                line[calc_line][x] = 0;
            }
            else
            {
                uint16_t sample = getAvgPix(data, 160*2, x*GB_FRAME_WIDTH/LCD_WIDTH, y*GB_FRAME_HEIGHT/LCD_HEIGHT);
                line[calc_line][x]=((sample >> 8) | ((sample) << 8));
            }
        }
        if (sending_line!=-1) send_line_finish();
        sending_line=calc_line;
        calc_line=(calc_line==1)?0:1;
        send_lines(y, LCD_WIDTH, line[sending_line]);
    }
    send_line_finish();
}

void display_init()
{
    // Line buffers
    const size_t lineSize = LCD_WIDTH * LINE_COUNT * sizeof(uint16_t);
    for (int x = 0; x < LINE_BUFFERS; x++)
    {
        line[x] = heap_caps_malloc(lineSize, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (!line[x]) abort();
    }
    // Initialize the LCD
    disp_spi_init();
#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ST)
    st7735r_init();
#endif

#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ILI)
    ili9341_init();
#endif
}
