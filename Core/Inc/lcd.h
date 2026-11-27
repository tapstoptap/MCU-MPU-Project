#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>

#define LCD_ROWS  2
#define LCD_COLS  16

uint8_t lcd_init(void);                          // find address + init, 1 = OK (blocks ~60 ms)
uint8_t lcd_is_ready(void);                      // 1 = LCD found and last I2C write OK
uint8_t lcd_get_address(void);                   // 7-bit address, 0 = not found
void    lcd_clear(void);                         // blocks ~2 ms: use only on mode change
void    lcd_set_cursor(uint8_t row, uint8_t col);
void    lcd_print(const char *s);                // stops at the end of the line
void    lcd_print_int(int32_t value);
void    lcd_print_line(uint8_t row, const char *s); // whole line, padded with spaces
void    lcd_backlight(uint8_t on);
void    lcd_print_x10(int32_t value_x10);           // 285 -> "28.5", -5 -> "-0.5"

#endif /* INC_LCD_H_ */