#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

void display_run(void);           // scheduler task, every 100 ms
void display_refresh_now(void);   // redraw the whole screen now (after a mode change)

#endif /* INC_DISPLAY_H_ */