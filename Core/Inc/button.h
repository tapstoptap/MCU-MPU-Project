#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>

#define BUTTON_COUNT          5      // BTN_1..BTN_4 + USER
#define BTN_IDX_1             0      // D7  - PA8
#define BTN_IDX_2             1      // D8  - PA9
#define BTN_IDX_3             2      // D9  - PC7
#define BTN_IDX_4             3      // D10 - PB6
#define BTN_IDX_USER          4      // PC13 (blue button on board)

#define BUTTON_DEBOUNCE_COUNT 3      // 3 equal readings x 10 ms = stable
#define BUTTON_LONG_PRESS_MS  1000   // hold time for a long press

void    button_init(void);
void    button_read(void);                    // call every 10 ms (scheduler task)
uint8_t button_is_pressed(uint8_t idx);       // 1 once per press (flag cleared when read)
uint8_t button_is_long_pressed(uint8_t idx);  // 1 once after holding 1 s
uint8_t button_is_held(uint8_t idx);          // 1 while the button is down
void    button_clear_all(void);               // forget unread presses (use when changing mode)

#endif /* INC_BUTTON_H_ */