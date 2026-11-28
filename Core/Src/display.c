#include "display.h"
#include "global.h"
#include "lcd.h"
#include "dht20.h"
#include "software_timer.h"
#include "traffic_light.h"
#include <stdio.h>
#include <string.h>

#define LCD_RETRY_RUNS   20       // LCD missing: try lcd_init() every 20 x 100 ms = 2 s
#define LINE_BUF         32       // build text in a bigger buffer, display_line() cuts it to 16

static char    shown[LCD_ROWS][LCD_COLS + 1];   // what is on the LCD right now
static uint8_t lcd_retry = 0;

/* write a line only if it is different from what is already shown (no flicker, less I2C) */
static void display_line(uint8_t row, const char *text)
{
    char line[LCD_COLS + 1];
    snprintf(line, sizeof(line), "%-16.16s", text);  // exactly 16 chars: pad with spaces or cut
    if (strcmp(line, shown[row]) == 0) return;
    lcd_print_line(row, line);
    strcpy(shown[row], line);
}

/* seconds left in the current phase, rounded up, max 99 */
static int phase_seconds_left(int extra_s)
{
    int s = (int)((timer_get_remaining_ms(TMR_PHASE) + 999) / 1000) + extra_s;
    return (s > 99) ? 99 : s;
}

static const char *color_name(tl_color_t c)
{
    switch (c) {
    case TL_RED:   return "RED";
    case TL_AMBER: return "AMBER";
    case TL_GREEN: return "GREEN";
    default:       return "OFF";
    }
}

/* "T:28.5°C H:65.2%"  (\xDF = degree sign) */
static void format_environment(char *out)
{
    if (!dht20_is_valid()) {
        snprintf(out, LINE_BUF, "T:--.-\xDF" "C H:--.-%%");
        return;
    }
    int t  = dht20_get_temp_x10();
    int h  = dht20_get_humi_x10();
    int ta = (t < 0) ? -t : t;
    snprintf(out, LINE_BUF, "T:%s%d.%d\xDF" "C H:%d.%d%%",
             (t < 0) ? "-" : "", ta / 10, ta % 10, h / 10, h % 10);
}

/* AUTO:   "AUTO 1:R05 2:G03" + environment */
static void draw_auto(char *l0, char *l1)
{
    char c1 = 'R', c2 = 'R';
    int  s1, s2;
    int  t = phase_seconds_left(0);

    switch (g_phase) {
    case PHASE_R1_G2: c1 = 'R'; s1 = phase_seconds_left(g_time_amber); c2 = 'G'; s2 = t; break;
    case PHASE_R1_A2: c1 = 'R'; s1 = t;                                c2 = 'A'; s2 = t; break;
    case PHASE_G1_R2: c1 = 'G'; s1 = t; c2 = 'R'; s2 = phase_seconds_left(g_time_amber); break;
    case PHASE_A1_R2: c1 = 'A'; s1 = t;                                c2 = 'R'; s2 = t; break;
    default:          c1 = 'R'; s1 = t;                                c2 = 'R'; s2 = t; break;
    }
    snprintf(l0, LINE_BUF, "AUTO 1:%c%02d 2:%c%02d", c1, s1, c2, s2);
    format_environment(l1);
}

/* MANUAL: "MANUAL  B2=Next" / "1:RED   2:GREEN" */
static void draw_manual(char *l0, char *l1)
{
    if (phase_is_amber(g_phase)) snprintf(l0, LINE_BUF, "MANUAL  (amber)");
    else                         snprintf(l0, LINE_BUF, "MANUAL  B2=Next");
    snprintf(l1, LINE_BUF, "1:%-5s 2:%-5s",
             color_name(traffic_get(TL_ROAD_1)), color_name(traffic_get(TL_ROAD_2)));
}

/* TUNING: "TUNE RED  : 05s" / "R05 G03 A02 U=OK" */
static void draw_tuning(char *l0, char *l1)
{
    static const char *names[3] = { "RED", "GREEN", "AMBER" };
    uint8_t v = (g_tune_field == FIELD_GREEN) ? g_edit_green :
                (g_tune_field == FIELD_AMBER) ? g_edit_amber : g_edit_red;

    switch (g_tune_state) {
    case TUNE_SAVED:
        snprintf(l0, LINE_BUF, "SAVED!");
        snprintf(l1, LINE_BUF, "R%02d G%02d A%02d", g_edit_red, g_edit_green, g_edit_amber);
        break;
    case TUNE_ERROR:
        snprintf(l0, LINE_BUF, "ERR: R != G+A");
        snprintf(l1, LINE_BUF, "R=%02d G+A=%02d", g_edit_red, g_edit_green + g_edit_amber);
        break;
    default:
        snprintf(l0, LINE_BUF, "TUNE %-5s: %02ds", names[g_tune_field % 3], v);
        snprintf(l1, LINE_BUF, "R%02d G%02d A%02d U=OK", g_edit_red, g_edit_green, g_edit_amber);
        break;
    }
}

void display_run(void)
{
    /* LCD missing or unplugged: try to reconnect every 2 s, then redraw everything */
    if (!lcd_is_ready()) {
        if (++lcd_retry < LCD_RETRY_RUNS) return;
        lcd_retry = 0;
        if (!lcd_init()) return;
        memset(shown, 0, sizeof(shown));
    }

    char l0[LINE_BUF];
    char l1[LINE_BUF];

    switch (g_mode) {
    case MODE_MANUAL: draw_manual(l0, l1); break;
    case MODE_TUNING: draw_tuning(l0, l1); break;
    default:          draw_auto(l0, l1);   break;
    }
    display_line(0, l0);
    display_line(1, l1);
}

void display_refresh_now(void)
{
    memset(shown, 0, sizeof(shown));      // forget the cache -> both lines are rewritten
    display_run();
}