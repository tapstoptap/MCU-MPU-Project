#include "button.h"
#include "main.h"                               // pin labels + HAL

#define BUTTON_PRESSED_LEVEL  GPIO_PIN_RESET    // pull-up: pressed = 0 (from Step 3)
#define LONG_PRESS_TICKS      (BUTTON_LONG_PRESS_MS / 10)

static GPIO_TypeDef * const btn_port[BUTTON_COUNT] = {
    BTN_1_GPIO_Port, BTN_2_GPIO_Port, BTN_3_GPIO_Port, BTN_4_GPIO_Port, USER_BTN_GPIO_Port
};
static const uint16_t btn_pin[BUTTON_COUNT] = {
    BTN_1_Pin, BTN_2_Pin, BTN_3_Pin, BTN_4_Pin, USER_BTN_Pin
};

static uint8_t  last_raw[BUTTON_COUNT];      // last raw reading (1 = down)
static uint8_t  same_count[BUTTON_COUNT];    // equal readings in a row
static uint8_t  stable[BUTTON_COUNT];        // debounced state (1 = down)
static uint16_t hold_ticks[BUTTON_COUNT];    // how long it has been down
static uint8_t  flag_pressed[BUTTON_COUNT];
static uint8_t  flag_long[BUTTON_COUNT];

void button_init(void)
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        last_raw[i] = 0;  same_count[i] = 0;  stable[i] = 0;
        hold_ticks[i] = 0;  flag_pressed[i] = 0;  flag_long[i] = 0;
    }
}

void button_read(void)
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        uint8_t raw = (HAL_GPIO_ReadPin(btn_port[i], btn_pin[i]) == BUTTON_PRESSED_LEVEL) ? 1 : 0;

        /* debounce: count equal readings in a row */
        if (raw == last_raw[i]) {
            if (same_count[i] < BUTTON_DEBOUNCE_COUNT) same_count[i]++;
        } else {
            last_raw[i]   = raw;
            same_count[i] = 1;
        }

        /* state really changed */
        if (same_count[i] >= BUTTON_DEBOUNCE_COUNT && stable[i] != raw) {
            stable[i] = raw;
            if (raw == 1) {                 // just pressed
                flag_pressed[i] = 1;
                hold_ticks[i]   = 0;
            }
        }

        /* long press: fires once per hold */
        if (stable[i] == 1) {
            if (hold_ticks[i] < LONG_PRESS_TICKS) {
                hold_ticks[i]++;
                if (hold_ticks[i] == LONG_PRESS_TICKS) flag_long[i] = 1;
            }
        } else {
            hold_ticks[i] = 0;
        }
    }
}

uint8_t button_is_pressed(uint8_t idx)
{
    if (idx >= BUTTON_COUNT) return 0;
    if (flag_pressed[idx]) { flag_pressed[idx] = 0; return 1; }
    return 0;
}

uint8_t button_is_long_pressed(uint8_t idx)
{
    if (idx >= BUTTON_COUNT) return 0;
    if (flag_long[idx]) { flag_long[idx] = 0; return 1; }
    return 0;
}

uint8_t button_is_held(uint8_t idx)
{
    if (idx >= BUTTON_COUNT) return 0;
    return stable[idx];
}

void button_clear_all(void)
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        flag_pressed[i] = 0;
        flag_long[i]    = 0;
    }
}