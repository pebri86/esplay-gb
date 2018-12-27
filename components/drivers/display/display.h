#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include <stdint.h>
#include "disp_spi.h"

//*****************************************************************************
//
// Make sure all of the definitions in this header have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ILI)
#include "ili9341.h"
#define LCD_WIDTH       ILI9341_HOR_RES
#define LCD_HEIGHT      ILI9341_VER_RES
#define DPI             100
#endif

#if (CONFIG_HW_LCD_TYPE == LCD_TYPE_ST)
#include "st7735r.h"
#define LCD_WIDTH       ST7735R_HOR_RES
#define LCD_HEIGHT      ST7735R_VER_RES
#define DPI             20
#endif

void display_init();
void write_nes_frame(const uint8_t * data[]);
void write_gb_frame(const uint16_t * data);
void set_display_brightness(int percent);

#ifdef __cplusplus
}
#endif

#endif /*_DISPLAY_H_*/
