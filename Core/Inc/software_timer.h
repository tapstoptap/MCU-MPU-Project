#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

#define TIMER_COUNT     5     // number of software timers, ID 0..4
#define TIMER_TICK_MS   10    // timer_run() is called every 10 ms

void    timer_init(void);
void    timer_set(uint8_t id, uint32_t duration_ms);  // start / restart a timer
void    timer_stop(uint8_t id);                       // stop and clear its flag
uint8_t timer_is_expired(uint8_t id);                 // 1 = time is up (stays 1 until set/stop)
void     timer_run(void);                                // call every 10 ms (scheduler task)
uint32_t timer_get_remaining_ms(uint8_t id);             // time left in ms (0 = stopped/expired)

#endif /* INC_SOFTWARE_TIMER_H_ */