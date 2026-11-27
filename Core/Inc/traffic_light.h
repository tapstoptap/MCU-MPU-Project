#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

typedef enum { TL_OFF = 0, TL_RED, TL_AMBER, TL_GREEN } tl_color_t;

#define TL_ROAD_1  1
#define TL_ROAD_2  2

void       traffic_init(void);                               // both roads OFF
uint8_t    traffic_set(uint8_t road, tl_color_t color);      // 1 = OK, 0 = refused
tl_color_t traffic_get(uint8_t road);
void       traffic_set_both(tl_color_t road1, tl_color_t road2); // safe order, never 2 GREEN

#endif /* INC_TRAFFIC_LIGHT_H_ */