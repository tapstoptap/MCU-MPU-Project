#include "fsm_mode.h"
#include "global.h"
#include "software_timer.h"
#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "fsm_tuning.h"
#include "display.h"

/* every mode change goes through here */
static void mode_enter(sys_mode_t m)
{
    button_clear_all();              // a press from the old mode must not act in the new one
    timer_stop(TMR_BLINK);           // timers that belong to TUNING only
    timer_stop(TMR_MSG);

    switch (m) {
    case MODE_MANUAL:
        g_mode = MODE_MANUAL;
        timer_set(TMR_IDLE, IDLE_TIMEOUT_MS);
        fsm_manual_init();
        break;

    case MODE_TUNING:
        g_mode = MODE_TUNING;
        timer_set(TMR_IDLE, IDLE_TIMEOUT_MS);
        fsm_tuning_init();
        break;

    default:                         // MODE_AUTO (and any invalid value)
        g_mode = MODE_AUTO;
        timer_stop(TMR_IDLE);
        fsm_automatic_init();
        break;
    }
    display_refresh_now();           // LCD shows the new mode immediately
}

static uint8_t any_button_held(void)
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        if (button_is_held(i)) return 1;
    }
    return 0;
}

void fsm_mode_init(void)
{
    mode_enter(MODE_AUTO);
}

void fsm_mode_run(void)
{
    /* 1. MODE button: AUTO -> MANUAL -> TUNING -> AUTO (from TUNING = cancel, not saved) */
    if (button_is_pressed(BTN_MODE)) {
        if      (g_mode == MODE_AUTO)   mode_enter(MODE_MANUAL);
        else if (g_mode == MODE_MANUAL) mode_enter(MODE_TUNING);
        else                            mode_enter(MODE_AUTO);
        return;                      // one mode change per call
    }

    /* 2. MANUAL / TUNING: 60 s without any button -> back to AUTO */
    if (g_mode != MODE_AUTO) {
        if (any_button_held()) {
            timer_set(TMR_IDLE, IDLE_TIMEOUT_MS);
        } else if (timer_is_expired(TMR_IDLE)) {
            mode_enter(MODE_AUTO);
            return;
        }
    }

    /* 3. run the state machine of the current mode */
    switch (g_mode) {
    case MODE_AUTO:
        fsm_automatic_run();
        break;
    case MODE_MANUAL:
        fsm_manual_run();
        break;
    case MODE_TUNING:
        if (fsm_tuning_run()) mode_enter(MODE_AUTO);  // saved -> AUTO with the new times
        break;
    default:
        mode_enter(MODE_AUTO);                        // corrupted value -> safe
        break;
    }
}   