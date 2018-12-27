/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

#define DISP_SPI_MOSI CONFIG_HW_LCD_MOSI_GPIO
#define DISP_SPI_CLK  CONFIG_HW_LCD_CLK_GPIO
#define DISP_SPI_CS   CONFIG_HW_LCD_CS_GPIO
#define DISP_SPI_DC   CONFIG_HW_LCD_DC_GPIO
#define CMD_ON        0
#define DATA_ON       1
#define LCD_TYPE_ILI  0
#define LCD_TYPE_ST   1

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void disp_spi_init(void);
void disp_spi_send(uint8_t * data, uint16_t length, int dc);
void send_lines(int ypos, int width, uint16_t *linedata);
void send_line_finish(void);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
