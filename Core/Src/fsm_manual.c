#include "fsm_manual.h"
#include "global.h"
#include "software_timer.h"

static void manual_go(phase_t p)
{
    g_phase = p;
    phase_output(p);
}

void fsm_manual_init(void)
{
    /* lights are NOT changed when entering MANUAL.
       A running AMBER keeps its timer and finishes normally. */
    if (phase_is_amber(g_phase)) return;

    timer_stop(TMR_PHASE);                         // GREEN / ALL_RED phases wait for BTN_2
    if (g_phase != PHASE_R1_G2 && g_phase != PHASE_G1_R2) {
        manual_go(PHASE_ALL_RED);                  // anything else (e.g. FLASH) -> safe ALL_RED
    }
}

void fsm_manual_run(void)
{
    /* always read the key: a press during AMBER is thrown away, not saved for later */
    uint8_t next = button_is_pressed(BTN_NEXT);

    switch (g_phase) {
    case PHASE_R1_G2:                              // road 2 GREEN: wait for the user
        if (next) {
            manual_go(PHASE_R1_A2);
            timer_set(TMR_PHASE, g_time_amber * 1000);
        }
        break;

    case PHASE_R1_A2:                              // road 2 AMBER: runs by itself
        if (timer_is_expired(TMR_PHASE)) {
            manual_go(PHASE_G1_R2);
            timer_stop(TMR_PHASE);
        }
        break;

    case PHASE_G1_R2:                              // road 1 GREEN: wait for the user
        if (next) {
            manual_go(PHASE_A1_R2);
            timer_set(TMR_PHASE, g_time_amber * 1000);
        }
        break;

    case PHASE_A1_R2:                              // road 1 AMBER: runs by itself
        if (timer_is_expired(TMR_PHASE)) {
            manual_go(PHASE_R1_G2);
            timer_stop(TMR_PHASE);
        }
        break;

    default:                                       // ALL_RED: first press gives road 2 GREEN
        if (next) manual_go(PHASE_R1_G2);
        break;
    }
}