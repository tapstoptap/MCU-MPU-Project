#ifndef INC_FSM_TUNING_H_
#define INC_FSM_TUNING_H_

#include <stdint.h>

void    fsm_tuning_init(void);    // called once when entering TUNING
uint8_t fsm_tuning_run(void);     // called every 10 ms; returns 1 when values were SAVED (-> AUTO)

#endif /* INC_FSM_TUNING_H_ */