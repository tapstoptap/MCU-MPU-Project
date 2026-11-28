#ifndef INC_FSM_MANUAL_H_
#define INC_FSM_MANUAL_H_

void fsm_manual_init(void);       // called once when entering MANUAL
void fsm_manual_run(void);        // called every 10 ms while in MANUAL

#endif /* INC_FSM_MANUAL_H_ */