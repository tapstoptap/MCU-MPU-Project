#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include <stdint.h>

#define DHT20_ADDR            0x38    // fixed I2C address (7-bit)
#define DHT20_MEASURE_TIME_MS 80      // sensor needs >= 80 ms after start
#define DHT20_MAX_FAIL        3       // errors in a row before value becomes invalid

typedef enum {
    DHT20_OK = 0,
    DHT20_ERR_NOT_FOUND,      // no answer at 0x38
    DHT20_ERR_I2C,            // I2C transfer failed
    DHT20_ERR_BUSY,           // measurement still running / already started
    DHT20_ERR_CRC,            // data corrupted
    DHT20_ERR_NOT_STARTED     // read called without start
} dht20_status_t;

uint8_t        dht20_init(void);             // 1 = found + ready (blocks ~150 ms, call before scheduler)
dht20_status_t dht20_start_measure(void);    // step 1: send measure command
dht20_status_t dht20_read_measure(void);     // step 2: call >= 80 ms after start
uint8_t        dht20_is_valid(void);         // 1 = a recent valid value exists
int16_t        dht20_get_temp_x10(void);     // 28.5 C -> 285
uint16_t       dht20_get_humi_x10(void);     // 65.2 % -> 652
uint32_t       dht20_get_ok_count(void);
uint32_t       dht20_get_err_count(void);

#endif /* INC_DHT20_H_ */