#ifndef INC_FSM_MODE_H_
#define INC_FSM_MODE_H_

void fsm_mode_init(void);         // start the system in AUTO
void fsm_mode_run(void);          // scheduler task, every 10 ms

#endif /* INC_FSM_MODE_H_ */