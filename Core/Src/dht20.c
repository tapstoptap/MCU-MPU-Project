#include "dht20.h"
#include "main.h"
#include "i2c.h"                       // hi2c1

#define DHT20_I2C_TIMEOUT      20      // ms
#define DHT20_BUSY_TIMEOUT_MS  1000    // forget a measurement that was never read

static uint8_t  dht_found      = 0;
static uint8_t  dht_need_init  = 1;
static uint8_t  dht_measuring  = 0;
static uint32_t dht_start_tick = 0;
static uint8_t  dht_valid      = 0;
static uint8_t  dht_fail       = 0;
static int16_t  dht_temp_x10   = 0;
static uint16_t dht_humi_x10   = 0;
static uint32_t dht_ok_count   = 0;
static uint32_t dht_err_count  = 0;

/* ---------- low level ---------- */
static HAL_StatusTypeDef dht20_write(uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Master_Transmit(&hi2c1, DHT20_ADDR << 1, buf, len, DHT20_I2C_TIMEOUT);
}

static HAL_StatusTypeDef dht20_read(uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Master_Receive(&hi2c1, DHT20_ADDR << 1, buf, len, DHT20_I2C_TIMEOUT);
}

/* CRC-8, polynomial 0x31, init 0xFF (from the datasheet) */
static uint8_t dht20_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

/* one failure: after DHT20_MAX_FAIL in a row the value is no longer valid */
static void dht20_fail(void)
{
    dht_err_count++;
    if (dht_fail < DHT20_MAX_FAIL) dht_fail++;
    if (dht_fail >= DHT20_MAX_FAIL) dht_valid = 0;
}

/* register reset from the datasheet sample code */
static void dht20_reset_reg(uint8_t reg)
{
    uint8_t w[3] = { reg, 0x00, 0x00 };
    uint8_t r[3] = { 0, 0, 0 };
    dht20_write(w, 3);  HAL_Delay(5);
    dht20_read(r, 3);   HAL_Delay(10);
    w[0] = 0xB0 | reg;  w[1] = r[1];  w[2] = r[2];
    dht20_write(w, 3);
}

/* read status; if calibration bits are not set, initialise the sensor registers */
static uint8_t dht20_check_and_init(void)
{
    uint8_t cmd = 0x71;
    uint8_t status = 0;
    if (dht20_write(&cmd, 1) != HAL_OK) return 0;
    if (dht20_read(&status, 1) != HAL_OK) return 0;

    if ((status & 0x18) != 0x18) {          // only after power-up / replug (blocks ~50 ms)
        dht20_reset_reg(0x1B);
        dht20_reset_reg(0x1C);
        dht20_reset_reg(0x1E);
        HAL_Delay(10);
    }
    return 1;
}

/* ---------- public ---------- */
uint8_t dht20_init(void)
{
    HAL_Delay(100);                          // sensor needs 100 ms after power-up
    dht_measuring = 0;
    dht_found = (HAL_I2C_IsDeviceReady(&hi2c1, DHT20_ADDR << 1, 2, 10) == HAL_OK);
    if (!dht_found) { dht_need_init = 1; return 0; }

    if (!dht20_check_and_init()) { dht_found = 0; dht_need_init = 1; return 0; }
    dht_need_init = 0;
    return 1;
}

dht20_status_t dht20_start_measure(void)
{
    /* a measurement is already running */
    if (dht_measuring) {
        if (HAL_GetTick() - dht_start_tick < DHT20_BUSY_TIMEOUT_MS) return DHT20_ERR_BUSY;
        dht_measuring = 0;                   // it was never read -> forget it and record failure
        dht_need_init = 1;
        dht20_fail();
    }

    /* (re)connect if the sensor was missing or unplugged */
    if (!dht_found || dht_need_init) {
        i2c1_recover();
        if (HAL_I2C_IsDeviceReady(&hi2c1, DHT20_ADDR << 1, 2, 10) != HAL_OK) {
            dht_found = 0;
            dht20_fail();
            return DHT20_ERR_NOT_FOUND;
        }
        dht_found = 1;
        if (!dht20_check_and_init()) { dht20_fail(); i2c1_recover(); return DHT20_ERR_I2C; }
        dht_need_init = 0;
    }

    uint8_t cmd[3] = { 0xAC, 0x33, 0x00 };   // trigger measurement
    if (dht20_write(cmd, 3) != HAL_OK) {
        dht_need_init = 1;
        dht20_fail();
        i2c1_recover();
        return DHT20_ERR_I2C;
    }
    dht_measuring  = 1;
    dht_start_tick = HAL_GetTick();
    return DHT20_OK;
}

dht20_status_t dht20_read_measure(void)
{
    if (!dht_measuring) return DHT20_ERR_NOT_STARTED;

    uint8_t d[7];
    if (dht20_read(d, 7) != HAL_OK) {
        dht_measuring = 0;
        dht_need_init = 1;
        dht20_fail();
        i2c1_recover();
        return DHT20_ERR_I2C;
    }
    if (d[0] & 0x80) return DHT20_ERR_BUSY;  // not finished: keep waiting, try again later

    dht_measuring = 0;
    if (dht20_crc8(d, 6) != d[6]) { dht20_fail(); return DHT20_ERR_CRC; }

    /* 20-bit raw values */
    uint32_t raw_h = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
    uint32_t raw_t = (((uint32_t)d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

    /* RH = raw/2^20*100, T = raw/2^20*200 - 50 (x10, integer math, no float) */
    uint32_t h = (uint32_t)(((uint64_t)raw_h * 1000) >> 20);
    if (h > 1000) h = 1000;
    dht_humi_x10 = (uint16_t)h;
    dht_temp_x10 = (int16_t)((int32_t)(((uint64_t)raw_t * 2000) >> 20) - 500);

    dht_valid = 1;
    dht_fail  = 0;
    dht_ok_count++;
    return DHT20_OK;
}

uint8_t  dht20_is_valid(void)      { return dht_valid; }
int16_t  dht20_get_temp_x10(void)  { return dht_temp_x10; }
uint16_t dht20_get_humi_x10(void)  { return dht_humi_x10; }
uint32_t dht20_get_ok_count(void)  { return dht_ok_count; }
uint32_t dht20_get_err_count(void) { return dht_err_count; }