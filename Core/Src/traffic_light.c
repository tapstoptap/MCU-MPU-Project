#include "traffic_light.h"
#include "main.h"

/* 2-bit signal mapping for the physical traffic light modules:
   value = A*2 + B  ->  (0,0):0 OFF | (0,1):1 AMBER | (1,0):2 GREEN | (1,1):3 RED */
#define TL_CODE_OFF     0   // A=0, B=0 -> OFF
#define TL_CODE_AMBER   1   // A=0, B=1 -> YELLOW / AMBER
#define TL_CODE_GREEN   2   // A=1, B=0 -> GREEN
#define TL_CODE_RED     3   // A=1, B=1 -> RED

static const uint8_t tl_code[4] = { TL_CODE_OFF, TL_CODE_RED, TL_CODE_AMBER, TL_CODE_GREEN };
static tl_color_t tl_state[2] = { TL_OFF, TL_OFF };

static void tl_write_pins(uint8_t road, uint8_t code)
{
    GPIO_PinState a = (GPIO_PinState)((code >> 1) & 1);
    GPIO_PinState b = (GPIO_PinState)(code & 1);

    GPIO_TypeDef *port_a = (road == TL_ROAD_1) ? TL1_A_GPIO_Port : TL2_A_GPIO_Port;
    uint16_t      pin_a  = (road == TL_ROAD_1) ? TL1_A_Pin       : TL2_A_Pin;
    GPIO_TypeDef *port_b = (road == TL_ROAD_1) ? TL1_B_GPIO_Port : TL2_B_GPIO_Port;
    uint16_t      pin_b  = (road == TL_ROAD_1) ? TL1_B_Pin       : TL2_B_Pin;

    /* When setting RED (1,1), write B=1 before A=1 so OFF (0,0) -> RED (1,1)
       passes through AMBER (0,1) instead of transiently outputting GREEN (1,0) */
    if (a == GPIO_PIN_SET && b == GPIO_PIN_SET) {
        HAL_GPIO_WritePin(port_b, pin_b, b);
        HAL_GPIO_WritePin(port_a, pin_a, a);
    } else {
        HAL_GPIO_WritePin(port_a, pin_a, a);
        HAL_GPIO_WritePin(port_b, pin_b, b);
    }
}

uint8_t traffic_set(uint8_t road, tl_color_t color)
{
    if (road != TL_ROAD_1 && road != TL_ROAD_2) return 0;     // invalid road
    if (color > TL_GREEN) color = TL_OFF;                     // invalid colour -> OFF

    /* SAFETY: never allow both roads GREEN at the same time */
    uint8_t other = (road == TL_ROAD_1) ? TL_ROAD_2 : TL_ROAD_1;
    if (color == TL_GREEN && tl_state[other - 1] == TL_GREEN) {
        tl_write_pins(road, tl_code[TL_RED]);                 // refuse: show RED instead
        tl_state[road - 1] = TL_RED;
        return 0;
    }

    tl_write_pins(road, tl_code[color]);
    tl_state[road - 1] = color;
    return 1;
}

tl_color_t traffic_get(uint8_t road)
{
    if (road != TL_ROAD_1 && road != TL_ROAD_2) return TL_OFF;
    return tl_state[road - 1];
}

void traffic_init(void)
{
    tl_state[0] = TL_OFF;
    tl_state[1] = TL_OFF;
    tl_write_pins(TL_ROAD_1, tl_code[TL_OFF]);
    tl_write_pins(TL_ROAD_2, tl_code[TL_OFF]);
}

void traffic_set_both(tl_color_t road1, tl_color_t road2)
{
    if (road1 == TL_GREEN && road2 == TL_GREEN) {   // forbidden request -> safest state
        road1 = TL_RED;
        road2 = TL_RED;
    }

    /* always clear a currently-GREEN road first, and set a newly-GREEN road last */
    if (road1 == TL_GREEN || tl_state[1] == TL_GREEN) {
        traffic_set(TL_ROAD_2, road2);
        traffic_set(TL_ROAD_1, road1);
    } else {
        traffic_set(TL_ROAD_1, road1);
        traffic_set(TL_ROAD_2, road2);
    }
}