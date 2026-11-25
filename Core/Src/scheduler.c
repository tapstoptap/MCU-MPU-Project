#include "scheduler.h"
#include "main.h"            // gives __disable_irq / __get_PRIMASK (CMSIS)

static sTask    SCH_tasks_G[SCH_MAX_TASKS];  // sorted by run time, Delay is relative
static uint8_t  SCH_count   = 0;             // number of tasks in the list
static uint32_t SCH_next_id = 1;             // next ID to give out (never 0)

/* ---------- critical section: stop the TIM2 interrupt while we edit the list ---------- */
static uint32_t sch_lock(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static void sch_unlock(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* ---------- internal helpers (call only with interrupts disabled) ---------- */

/* If the first task has nothing left to wait, mark it ready */
static void sch_mark_head_ready(void)
{
    if (SCH_count > 0 && SCH_tasks_G[0].Delay == 0) {
        SCH_tasks_G[0].RunMe = 1;
    }
}

/* Insert task t (t.Delay = absolute ticks from now) at the right place.
   Returns 1 = OK, 0 = list full */
static uint8_t sch_insert(sTask t)
{
    if (SCH_count >= SCH_MAX_TASKS) return 0;

    uint8_t  i     = 0;
    uint32_t delay = t.Delay;

    /* walk past every task that runs before or at the same time as the new one */
    while (i < SCH_count && delay >= SCH_tasks_G[i].Delay) {
        delay -= SCH_tasks_G[i].Delay;
        i++;
    }

    /* shift tasks right to make room at position i */
    for (uint8_t j = SCH_count; j > i; j--) {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1];
    }

    t.Delay = delay;          // now relative to the task before it
    t.RunMe = 0;
    SCH_tasks_G[i] = t;
    SCH_count++;

    /* the task right after the new one now waits relative to the new one */
    if (i + 1 < SCH_count) {
        SCH_tasks_G[i + 1].Delay -= delay;
    }
    return 1;
}

/* Remove the task at position i and keep the timing of the others correct */
static void sch_remove_at(uint8_t i)
{
    if (i >= SCH_count) return;

    /* give the removed task's waiting time to the task after it */
    if (i + 1 < SCH_count) {
        SCH_tasks_G[i + 1].Delay += SCH_tasks_G[i].Delay;
    }

    /* shift tasks left */
    for (uint8_t j = i; j + 1 < SCH_count; j++) {
        SCH_tasks_G[j] = SCH_tasks_G[j + 1];
    }

    SCH_count--;
    SCH_tasks_G[SCH_count] = (sTask){0};   // clear the empty slot
}

/* ---------- public functions ---------- */

void SCH_Init(void)
{
    uint32_t key = sch_lock();
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i] = (sTask){0};
    }
    SCH_count = 0;
    sch_unlock(key);
}

uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunc == 0) return SCH_NO_TASK_ID;          // no function -> refuse

    uint32_t key = sch_lock();
    uint32_t id  = SCH_NO_TASK_ID;

    sTask t;
    t.pTask  = pFunc;
    t.Delay  = DELAY;
    t.Period = PERIOD;
    t.RunMe  = 0;
    t.TaskID = SCH_next_id;

    if (sch_insert(t)) {
        id = SCH_next_id;
        SCH_next_id++;
        if (SCH_next_id == SCH_NO_TASK_ID) SCH_next_id = 1;   // skip 0 after wrap-around
        sch_mark_head_ready();
    }

    sch_unlock(key);
    return id;                                       // 0 = list full
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == SCH_NO_TASK_ID) return 0;          // invalid ID

    uint32_t key   = sch_lock();
    uint8_t  found = 0;

    for (uint8_t i = 0; i < SCH_count; i++) {
        if (SCH_tasks_G[i].TaskID == taskID) {
            sch_remove_at(i);
            found = 1;
            break;
        }
    }
    sch_mark_head_ready();

    sch_unlock(key);
    return found;                                    // 1 = deleted, 0 = not found
}

/* O(1): only the first task is checked */
void SCH_Update(void)
{
    if (SCH_count == 0) return;
    if (SCH_tasks_G[0].Delay > 0) SCH_tasks_G[0].Delay--;
    if (SCH_tasks_G[0].Delay == 0) SCH_tasks_G[0].RunMe = 1;
}

void SCH_Dispatch_Tasks(void)
{
    while (1) {                                      // run every task that is due now
        uint32_t key = sch_lock();

        if (SCH_count == 0 || SCH_tasks_G[0].RunMe == 0) {
            sch_unlock(key);
            return;                                  // nothing ready
        }

        sTask t = SCH_tasks_G[0];                    // copy the ready task
        sch_remove_at(0);                            // take it off the list

        if (t.Period > 0) {                          // periodic: put back, same ID
            t.Delay = t.Period;
            sch_insert(t);
        }                                            // one-shot: not put back

        sch_mark_head_ready();                       // next task may be due at the same tick
        sch_unlock(key);

        t.pTask();                                   // run with interrupts ON
    }
}

uint8_t SCH_Get_Task_Count(void)
{
    return SCH_count;
}