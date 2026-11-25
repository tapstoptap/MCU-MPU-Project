#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS    10   // max number of tasks in the list at the same time
#define SCH_TICK_MS      10   // 1 tick = 10 ms (TIM2 interrupt)
#define SCH_NO_TASK_ID   0    // returned by SCH_Add_Task when it fails

typedef struct {
    void     (*pTask)(void);  // pointer to the task function
    uint32_t Delay;           // ticks to wait, RELATIVE to the task before it
    uint32_t Period;          // period in ticks, 0 = one-shot task
    uint8_t  RunMe;           // 1 = task is ready to run
    uint32_t TaskID;          // unique ID (never 0)
} sTask;

void     SCH_Init(void);
uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Update(void);          // call in the TIM2 interrupt (every 10 ms)
void     SCH_Dispatch_Tasks(void);  // call in while(1)
uint8_t  SCH_Get_Task_Count(void);  // number of tasks in the list (for testing)

#endif /* INC_SCHEDULER_H_ */