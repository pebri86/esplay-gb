/**
 * @file lv_templ.h
 *
 */

#ifndef ILI9341_H
#define ILI9341_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/	
#if CONFIG_USE_LVGL_LIBRARY
#include "../lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define ILI9341_HOR_RES	320
#define ILI9341_VER_RES	320

#define ILI9341_RST     CONFIG_HW_LCD_RESET_GPIO
#define ILI9341_BCKL    CONFIG_HW_LCD_BL_GPIO

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
int is_backlight_initialized();
void backlight_percentage_set(int value);
void ili9341_init(void);

#if CONFIG_USE_LVGL_LIBRARY
void ili9341_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
void ili9341_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map);
#endif

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9341_H*/
