#include "lcd.h"
#include "main.h"
#include "i2c.h"                      // hi2c1

/* PCF8574 backpack pins: P0=RS P1=RW P2=EN P3=Backlight P4..P7=D4..D7 */
#define LCD_RS            0x01
#define LCD_EN            0x04
#define LCD_BL            0x08
#define LCD_I2C_TIMEOUT   20          // ms

/* possible LCD addresses, 0x21 first (this board's LCD), fallback to others (0x38 = DHT20, never used) */
static const uint8_t lcd_candidates[] = {
    0x21, 0x27, 0x3F, 0x20, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E
};

static uint8_t lcd_addr = 0;          // 7-bit address, 0 = not found
static uint8_t lcd_ok   = 0;          // 0 = not found or I2C error
static uint8_t lcd_bl   = LCD_BL;     // backlight bit
static uint8_t cur_col  = 0;

/* send raw bytes to the PCF8574; any error marks the LCD as not ready */
static void lcd_write_raw(uint8_t *buf, uint16_t len)
{
    if (!lcd_ok) return;
    if (HAL_I2C_Master_Transmit(&hi2c1, lcd_addr << 1, buf, len, LCD_I2C_TIMEOUT) != HAL_OK) {
        lcd_ok = 0;
        i2c1_recover();
    }
}

/* only the high 4 bits, RS = 0 (used during the init sequence) */
static void lcd_send_nibble(uint8_t high_nibble)
{
    uint8_t d = (high_nibble & 0xF0) | lcd_bl;
    uint8_t buf[2] = { d | LCD_EN, d };
    lcd_write_raw(buf, 2);
}

/* one full byte as two nibbles; rs = 0 command, LCD_RS data */
static void lcd_send_byte(uint8_t value, uint8_t rs)
{
    uint8_t hi = (value & 0xF0) | rs | lcd_bl;
    uint8_t lo = ((value << 4) & 0xF0) | rs | lcd_bl;
    uint8_t buf[4] = { hi | LCD_EN, hi, lo | LCD_EN, lo };
    lcd_write_raw(buf, 4);
}

static void lcd_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}

uint8_t lcd_init(void)
{
    lcd_addr = 0;
    lcd_ok   = 0;
    cur_col  = 0;
    i2c1_recover();

    /* 1. find the LCD address */
    for (uint8_t i = 0; i < sizeof(lcd_candidates); i++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, lcd_candidates[i] << 1, 2, 10) == HAL_OK) {
            lcd_addr = lcd_candidates[i];
            break;
        }
    }
    if (lcd_addr == 0) return 0;                 // no LCD -> all lcd_ functions do nothing
    lcd_ok = 1;

    /* 2. HD44780 4-bit init sequence */
    uint8_t idle = lcd_bl;
    lcd_write_raw(&idle, 1);                     // all outputs low, EN = 0
    HAL_Delay(50);                               // LCD power-up time
    lcd_send_nibble(0x30); HAL_Delay(5);
    lcd_send_nibble(0x30); HAL_Delay(1);
    lcd_send_nibble(0x30); HAL_Delay(1);
    lcd_send_nibble(0x20); HAL_Delay(1);         // switch to 4-bit mode

    lcd_cmd(0x28);                               // 4-bit, 2 lines, 5x8 font
    lcd_cmd(0x08);                               // display off
    lcd_clear();
    lcd_cmd(0x06);                               // cursor moves right, no shift
    lcd_cmd(0x0C);                               // display on, cursor off, blink off

    return lcd_ok;
}

uint8_t lcd_is_ready(void)    { return lcd_ok; }
uint8_t lcd_get_address(void) { return lcd_addr; }

void lcd_clear(void)
{
    lcd_cmd(0x01);
    HAL_Delay(2);                                // clear needs ~1.6 ms
    cur_col = 0;
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    if (row >= LCD_ROWS) row = LCD_ROWS - 1;     // clamp invalid values
    if (col >= LCD_COLS) col = LCD_COLS - 1;
    lcd_cmd(0x80 | ((row == 0 ? 0x00 : 0x40) + col));
    cur_col = col;
}

void lcd_print(const char *s)
{
    if (!lcd_ok || s == 0) return;

    uint8_t  buf[LCD_COLS * 4];                  // whole string in ONE I2C transfer
    uint16_t n = 0;

    while (*s && cur_col < LCD_COLS) {           // never write past the end of the line
        uint8_t c = (uint8_t)*s++;
        if (c < 0x20) c = ' ';                   // control chars -> space
        uint8_t hi = (c & 0xF0) | LCD_RS | lcd_bl;
        uint8_t lo = ((c << 4) & 0xF0) | LCD_RS | lcd_bl;
        buf[n++] = hi | LCD_EN;  buf[n++] = hi;
        buf[n++] = lo | LCD_EN;  buf[n++] = lo;
        cur_col++;
    }
    if (n > 0) lcd_write_raw(buf, n);
}

void lcd_print_int(int32_t value)
{
    char buf[12];
    int8_t i = 11;
    buf[i] = '\0';

    uint32_t u = (value < 0) ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
    do {
        buf[--i] = '0' + (u % 10);
        u /= 10;
    } while (u > 0);
    if (value < 0) buf[--i] = '-';

    lcd_print(&buf[i]);
}

void lcd_print_line(uint8_t row, const char *s)
{
    char line[LCD_COLS + 1];
    uint8_t i = 0;

    if (s) {
        while (s[i] && i < LCD_COLS) { line[i] = s[i]; i++; }
    }
    while (i < LCD_COLS) line[i++] = ' ';       // pad -> old text is erased, no clear needed
    line[LCD_COLS] = '\0';

    lcd_set_cursor(row, 0);
    lcd_print(line);
}

void lcd_backlight(uint8_t on)
{
    lcd_bl = on ? LCD_BL : 0;
    uint8_t d = lcd_bl;
    lcd_write_raw(&d, 1);
}

void lcd_print_x10(int32_t value_x10)
{
    uint32_t u = (value_x10 < 0) ? (uint32_t)(-(int64_t)value_x10) : (uint32_t)value_x10;
    if (value_x10 < 0) {                         // sign first, so -0.5 is not shown as 0.5
        lcd_print("-");
    }
    lcd_print_int((int32_t)(u / 10));
    char d[3] = { '.', (char)('0' + (u % 10)), '\0' };
    lcd_print(d);
}