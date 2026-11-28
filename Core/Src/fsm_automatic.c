#include "fsm_automatic.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"

/* switch to phase p and stay there for 'seconds' */
static void auto_go(phase_t p, uint32_t seconds)
{
    g_phase = p;
    phase_output(p);
    timer_set(TMR_PHASE, seconds * 1000);
}

void fsm_automatic_init(void)
{
    /* never cut a GREEN straight to RED: that road gets its AMBER first */
    if (traffic_get(TL_ROAD_1) == TL_GREEN) {
        auto_go(PHASE_A1_R2, g_time_amber);
    } else if (traffic_get(TL_ROAD_2) == TL_GREEN) {
        auto_go(PHASE_R1_A2, g_time_amber);
    } else {
        auto_go(PHASE_ALL_RED, ALL_RED_TIME_S);     // boot, after TUNING, after an AMBER
    }
}

void fsm_automatic_run(void)
{
    if (!timer_is_expired(TMR_PHASE)) return;      // current phase not finished

    switch (g_phase) {
    case PHASE_ALL_RED: auto_go(PHASE_R1_G2, g_time_green); break;
    case PHASE_R1_G2:   auto_go(PHASE_R1_A2, g_time_amber); break;
    case PHASE_R1_A2:   auto_go(PHASE_G1_R2, g_time_green); break;
    case PHASE_G1_R2:   auto_go(PHASE_A1_R2, g_time_amber); break;
    case PHASE_A1_R2:   auto_go(PHASE_R1_G2, g_time_green); break;
    default:            auto_go(PHASE_ALL_RED, ALL_RED_TIME_S); break;   // unexpected -> safe
    }
}