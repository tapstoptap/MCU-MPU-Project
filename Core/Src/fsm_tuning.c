#include "fsm_tuning.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"

#define REPEAT_START_TICKS   100   // hold 1 s before auto-repeat starts (100 x 10 ms)
#define REPEAT_EVERY_TICKS   20    // then +1 / -1 every 200 ms

static uint8_t  blink_on = 0;
static uint16_t hold_inc = 0;
static uint16_t hold_dec = 0;

/* which edit value belongs to the selected field */
static uint8_t *field_value(tune_field_t f)
{
    if (f == FIELD_GREEN) return &g_edit_green;
    if (f == FIELD_AMBER) return &g_edit_amber;
    return &g_edit_red;
}

/* 1 once per press, and again every 200 ms while the key is held longer than 1 s */
static uint8_t key_with_repeat(uint8_t btn, uint16_t *hold)
{
    uint8_t fire = button_is_pressed(btn);

    if (button_is_held(btn)) {
        (*hold)++;
        if (*hold >= REPEAT_START_TICKS) {
            fire  = 1;
            *hold = REPEAT_START_TICKS - REPEAT_EVERY_TICKS;   // next repeat in 200 ms
        }
    } else {
        *hold = 0;
    }
    return fire;
}

void fsm_tuning_init(void)
{
    /* edit a COPY: the values in use only change when the user saves */
    g_edit_red   = g_time_red;
    g_edit_green = g_time_green;
    g_edit_amber = g_time_amber;
    g_tune_field = FIELD_RED;
    g_tune_state = TUNE_EDIT;
    hold_inc = 0;
    hold_dec = 0;

    /* lights: both roads flashing AMBER (caution) */
    timer_stop(TMR_PHASE);
    g_phase  = PHASE_FLASH;
    blink_on = 1;
    phase_output(PHASE_FLASH);
    timer_set(TMR_BLINK, BLINK_PERIOD_MS);
}

uint8_t fsm_tuning_run(void)
{
    /* 1. flashing AMBER on both roads, in every tuning state */
    if (timer_is_expired(TMR_BLINK)) {
        blink_on = !blink_on;
        tl_color_t c = blink_on ? TL_AMBER : TL_OFF;
        traffic_set_both(c, c);
        timer_set(TMR_BLINK, BLINK_PERIOD_MS);
    }

    /* 2. always read the keys, so presses during a message are thrown away */
    uint8_t next = button_is_pressed(BTN_NEXT);
    uint8_t inc  = key_with_repeat(BTN_INC, &hold_inc);
    uint8_t dec  = key_with_repeat(BTN_DEC, &hold_dec);
    uint8_t ok   = button_is_pressed(BTN_OK);

    /* 3. tuning state machine */
    switch (g_tune_state) {
    case TUNE_EDIT: {
        uint8_t *v = field_value(g_tune_field);

        if (inc && *v < TIME_MAX_S) (*v)++;           // clamp at 99
        if (dec && *v > TIME_MIN_S) (*v)--;           // clamp at 1
        if (next) g_tune_field = (tune_field_t)((g_tune_field + 1) % 3);

        if (ok) {
            if (times_are_valid(g_edit_red, g_edit_green, g_edit_amber)) {
                g_time_red   = g_edit_red;            // apply new values
                g_time_green = g_edit_green;
                g_time_amber = g_edit_amber;
                g_tune_state = TUNE_SAVED;
                timer_set(TMR_MSG, SAVED_MSG_MS);
            } else {
                g_tune_state = TUNE_ERROR;            // keep old values, show why
                timer_set(TMR_MSG, ERROR_MSG_MS);
            }
        }
        break;
    }

    case TUNE_SAVED:                                  // "SAVED!" for 1 s, then leave
        hold_inc = 0;
        hold_dec = 0;
        if (timer_is_expired(TMR_MSG)) return 1;
        break;

    case TUNE_ERROR:                                  // error for 2 s, then edit again
        hold_inc = 0;
        hold_dec = 0;
        if (timer_is_expired(TMR_MSG)) g_tune_state = TUNE_EDIT;
        break;

    default:
        g_tune_state = TUNE_EDIT;
        break;
    }
    return 0;
}