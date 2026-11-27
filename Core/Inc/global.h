#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>
#include "button.h"

/* ================= MODES ================= */
typedef enum {
    MODE_AUTO = 0,        // lights cycle by themselves
    MODE_MANUAL,          // user decides when to switch (BTN_2)
    MODE_TUNING           // user edits the durations
} sys_mode_t;

/* ================= LIGHT PHASES (both roads together) =================
   There is NO phase with two GREEN lights -> both GREEN can never be requested */
typedef enum {
    PHASE_ALL_RED = 0,    // road 1 RED    road 2 RED    (safety clearance)
    PHASE_R1_G2,          // road 1 RED    road 2 GREEN
    PHASE_R1_A2,          // road 1 RED    road 2 AMBER
    PHASE_G1_R2,          // road 1 GREEN  road 2 RED
    PHASE_A1_R2,          // road 1 AMBER  road 2 RED
    PHASE_FLASH           // both AMBER, flashing (TUNING mode)
} phase_t;

/* ================= TUNING ================= */
typedef enum { FIELD_RED = 0, FIELD_GREEN, FIELD_AMBER } tune_field_t;
typedef enum { TUNE_EDIT = 0, TUNE_SAVED, TUNE_ERROR } tune_state_t;

/* ================= BUTTON ROLES ================= */
#define BTN_MODE   BTN_IDX_1      // all modes: AUTO -> MANUAL -> TUNING -> AUTO
#define BTN_NEXT   BTN_IDX_2      // MANUAL: next phase   | TUNING: next field (R -> G -> A)
#define BTN_INC    BTN_IDX_3      // TUNING: +1 s (hold = repeat)
#define BTN_DEC    BTN_IDX_4      // TUNING: -1 s (hold = repeat)
#define BTN_OK     BTN_IDX_USER   // TUNING: save

/* ================= SOFTWARE TIMER IDs (0..4) ================= */
#define TMR_PHASE  0              // light phase: AUTO durations + MANUAL amber
#define TMR_BLINK  1              // flashing amber in TUNING
#define TMR_IDLE   2              // no-button timeout in MANUAL / TUNING
#define TMR_MSG    3              // SAVED / ERROR message in TUNING

/* ================= LIMITS & TIMES ================= */
#define TIME_MIN_S        1       // every duration: 1..99 s
#define TIME_MAX_S        99
#define DEFAULT_RED_S     5       // default: R = G + A
#define DEFAULT_GREEN_S   3
#define DEFAULT_AMBER_S   2
#define ALL_RED_TIME_S    2       // clearance when AUTO starts
#define BLINK_PERIOD_MS   500
#define IDLE_TIMEOUT_MS   60000   // 60 s without any button -> back to AUTO
#define SAVED_MSG_MS      1000
#define ERROR_MSG_MS      2000

/* ================= SHARED VARIABLES ================= */
extern sys_mode_t   g_mode;
extern phase_t      g_phase;
extern uint8_t      g_time_red, g_time_green, g_time_amber;   // in use, always valid
extern uint8_t      g_edit_red, g_edit_green, g_edit_amber;   // being edited in TUNING
extern tune_field_t g_tune_field;
extern tune_state_t g_tune_state;

void    phase_output(phase_t p);                         // drive both roads for a phase
uint8_t phase_is_amber(phase_t p);                       // 1 = R1_A2 or A1_R2
uint8_t times_are_valid(uint8_t r, uint8_t g, uint8_t a); // 1 = in range and R = G + A

#endif /* INC_GLOBAL_H_ */