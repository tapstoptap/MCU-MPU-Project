# Two-Road Traffic Light System — XNUCLEO-F103RB

Course project for **CO3009 Microprocessors – Microcontrollers**, Semester 1, 2026–2027,
Faculty of Computer Science and Engineering, HCMUT.

A traffic light controller for a two-road intersection running on the XNUCLEO-F103RB board
(STM32F103RB). All application logic runs on a cooperative O(1) scheduler; no `HAL_Delay`
is used outside of hardware bring-up code.

Project page: https://tapstoptap.github.io/MCU-MPU-Project/

## Features

| Mode | Behaviour |
|---|---|
| Automatic | Red – Green – Amber of both roads cycle alternately using the configured durations |
| Manual | The light state is changed with push buttons |
| Timing adjustment | Red / Green / Amber durations are edited with push buttons and shown on the LCD |
| Environment | DHT20 temperature and humidity are read periodically and shown on the LCD |
| Scheduler | `SCH_Add_Task`, `SCH_Delete_Task`, `SCH_Update` (O(1)), `SCH_Dispatch_Tasks`, one-shot tasks |

## Hardware

| Device | Connection | Notes |
|---|---|---|
| Traffic light module 1 | D3 – D4 | 2 signals → 4 states |
| Traffic light module 2 | D5 – D6 | 2 signals → 4 states |
| Button module 1 | D7 – D8 | 2 buttons, direct GPIO |
| Button module 2 | D9 – D10 | 2 buttons, direct GPIO |
| DHT20 sensor, LCD | I2C slot (red) | I2C1, PB8 = SCL, PB9 = SDA |
| On-board LEDs / USER button | PA5, PC9 / PC13 | Built into the board |

### Pin mapping

Read from the Arduino header schematic in the course guide (Figure 2), reused in report section 3.1.

| Arduino pin | STM32 pin | GPIO mode | User Label |
|---|---|---|---|
| D3 | PB3 | Output Push-Pull | `TL1_A` |
| D4 | PB5 | Output Push-Pull | `TL1_B` |
| D5 | PB4 | Output Push-Pull | `TL2_A` |
| D6 | PB10 | Output Push-Pull | `TL2_B` |
| D7 | PA8 | Input | `BTN_1` |
| D8 | PA9 | Input | `BTN_2` |
| D9 | PC7 | Input | `BTN_3` |
| D10 | PB6 | Input | `BTN_4` |
| SCL | PB8 | I2C1 SCL | DHT20 + LCD |
| SDA | PB9 | I2C1 SDA | DHT20 + LCD |

Notes:
- D3 (PB3) and D5 (PB4) are JTAG pins by default. Setting **SYS → Debug = Serial Wire**
  releases them as normal GPIO, so this setting is required for traffic light module 1 and 2 to work.
- I2C1 on PB8/PB9 is the remapped I2C1 position; selecting PB8 = I2C1_SCL and PB9 = I2C1_SDA
  in the `.ioc` pinout applies the remap automatically.
- The button input pull configuration (no pull / pull-up) is set after the logic level is
  measured in hardware test 3.3.

### Light signal combinations

| Signal A | Signal B | Light state |
|---|---|---|
| 0 | 0 | … |
| 0 | 1 | … |
| 1 | 0 | … |
| 1 | 1 | … |

## Project structure

```
Core/
├── Inc/
│   ├── global.h            shared state and durations
│   ├── scheduler.h         O(1) cooperative scheduler
│   ├── software_timer.h
│   ├── button.h            debouncing, press / long press
│   ├── traffic_light.h
│   ├── dht20.h
│   ├── lcd.h
│   ├── fsm_automatic.h
│   ├── fsm_manual.h
│   └── fsm_tuning.h
└── Src/                    matching .c files
docs/                       project page (GitHub Pages)
report/                     final report PDF
```

Layering rule: only the supporting modules (`button`, `traffic_light`, `dht20`, `lcd`,
`software_timer`) and `main.c` call HAL functions. The `fsm_*.c` files never touch hardware
directly.

## Build and flash

Requirements: STM32CubeIDE, XNUCLEO-F103RB board, USB cable.

1. Clone the repository:
   ```
   git clone https://github.com/tapstoptap/MCU-MPU-Project.git
   ```
2. In STM32CubeIDE: **File → Import → Existing Projects into Workspace**, select the cloned folder.
3. Open the `.ioc` file and check **System Core → SYS → Debug = Serial Wire**
   (this also frees PB3 and PB4 for the traffic lights).
   If the `Drivers/` folder is missing, use **Project → Generate Code**.
4. Build with **Project → Build Project** (Debug configuration).
5. Connect the board and use **Run → Run** to flash.

> The project must be created for **NUCLEO-F103RB** with Debug set to **Serial Wire**.
> A wrong configuration can lock the board after the first flash.

## How to operate

_To be completed after the state machines are finalized (button roles, mode switching, LCD screens)._

## Team

| Name | Student ID | GitHub |
|---|---|---|
| Nguyễn Minh Phúc | 2453005 | [@tapstoptap](https://github.com/tapstoptap) |
| Võ Lê Thành Triệu | 2453297 | [@volethanhtrieu](https://github.com/volethanhtrieu) |
| Nguyễn Minh Quang | 2453052 | [@quangnguyencomeng](https://github.com/quangnguyencomeng) |

## Submission

- Report: `report/CO3009_<StudentID1>_<StudentID2>_..._Report.pdf`
- Submitted tag: `v1.0-final` (commit hash written on the report cover page)
- Deadline: 04/12/2026
