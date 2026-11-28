#ifndef INC_FSM_AUTOMATIC_H_
#define INC_FSM_AUTOMATIC_H_

void fsm_automatic_init(void);    // called once when entering AUTO
void fsm_automatic_run(void);     // called every 10 ms while in AUTO

#endif /* INC_FSM_AUTOMATIC_H_ */