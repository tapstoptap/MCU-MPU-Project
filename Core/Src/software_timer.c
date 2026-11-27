#include "software_timer.h"

static uint32_t timer_counter[TIMER_COUNT];   // ticks left
static uint8_t  timer_flag[TIMER_COUNT];      // 1 = expired

void timer_init(void)
{
    for (uint8_t i = 0; i < TIMER_COUNT; i++) {
        timer_counter[i] = 0;
        timer_flag[i]    = 0;
    }
}

void timer_set(uint8_t id, uint32_t duration_ms)
{
    if (id >= TIMER_COUNT) return;                       // invalid ID -> ignore

    /* convert ms to ticks, round up (e.g. 15 ms -> 2 ticks) */
    uint32_t ticks = duration_ms / TIMER_TICK_MS;
    if (duration_ms % TIMER_TICK_MS != 0) ticks++;

    if (ticks == 0) {                                    // 0 ms -> expired immediately
        timer_counter[id] = 0;
        timer_flag[id]    = 1;
    } else {
        timer_counter[id] = ticks;
        timer_flag[id]    = 0;
    }
}

void timer_stop(uint8_t id)
{
    if (id >= TIMER_COUNT) return;
    timer_counter[id] = 0;
    timer_flag[id]    = 0;
}

uint8_t timer_is_expired(uint8_t id)
{
    if (id >= TIMER_COUNT) return 0;
    return timer_flag[id];
}

void timer_run(void)
{
    for (uint8_t i = 0; i < TIMER_COUNT; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] == 0) timer_flag[i] = 1;
        }
    }
}

uint32_t timer_get_remaining_ms(uint8_t id)
{
    if (id >= TIMER_COUNT) return 0;
    return timer_counter[id] * TIMER_TICK_MS;
}