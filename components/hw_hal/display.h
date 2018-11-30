#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include <stdint.h>

//*****************************************************************************
//
// Make sure all of the definitions in this header have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

#define LCD_WIDTH       160
#define LCD_HEIGHT      128

void display_init();
void send_lines(int ypos, uint16_t *linedata);
void send_line_finish();

#ifdef __cplusplus
}
#endif

#endif //  _DISPLAY_H_
