#include "global.h"
#include "traffic_light.h"

sys_mode_t   g_mode       = MODE_AUTO;
phase_t      g_phase      = PHASE_ALL_RED;

uint8_t      g_time_red   = DEFAULT_RED_S;
uint8_t      g_time_green = DEFAULT_GREEN_S;
uint8_t      g_time_amber = DEFAULT_AMBER_S;

uint8_t      g_edit_red   = DEFAULT_RED_S;
uint8_t      g_edit_green = DEFAULT_GREEN_S;
uint8_t      g_edit_amber = DEFAULT_AMBER_S;

tune_field_t g_tune_field = FIELD_RED;
tune_state_t g_tune_state = TUNE_EDIT;

void phase_output(phase_t p)
{
    switch (p) {
    case PHASE_R1_G2: traffic_set_both(TL_RED,   TL_GREEN); break;
    case PHASE_R1_A2: traffic_set_both(TL_RED,   TL_AMBER); break;
    case PHASE_G1_R2: traffic_set_both(TL_GREEN, TL_RED);   break;
    case PHASE_A1_R2: traffic_set_both(TL_AMBER, TL_RED);   break;
    case PHASE_FLASH: traffic_set_both(TL_AMBER, TL_AMBER); break;
    default:          traffic_set_both(TL_RED,   TL_RED);   break;   // ALL_RED or unknown -> safe
    }
}

uint8_t phase_is_amber(phase_t p)
{
    return (p == PHASE_R1_A2 || p == PHASE_A1_R2);
}

uint8_t times_are_valid(uint8_t r, uint8_t g, uint8_t a)
{
    if (r < TIME_MIN_S || r > TIME_MAX_S) return 0;
    if (g < TIME_MIN_S || g > TIME_MAX_S) return 0;
    if (a < TIME_MIN_S || a > TIME_MAX_S) return 0;
    return (r == g + a);                  // RED of one road = GREEN + AMBER of the other road
}