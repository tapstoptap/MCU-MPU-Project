# Team conventions

## Branches

| Branch | Purpose |
|---|---|
| `main` | Submitted, working versions only. Tagged `v1.0-final` at submission |
| `dev` | Shared development branch; merge features here first |
| `feature/<name>` | One branch per piece of work, e.g. `feature/scheduler`, `feature/dht20` |

Workflow: branch from `dev` → commit → push → open a pull request into `dev` → another member reviews → merge.

## Commits

- Every member commits with their own GitHub account (commit history is graded).
- One meaningful change per commit.
- Message format: `<module>: <what changed>`, lowercase, imperative.
  - `scheduler: add one-shot task support`
  - `button: debounce with 3 consecutive reads`
  - `fsm_tuning: reject green longer than red`
- Do not commit build output (`Debug/`, `*.elf`, …); see `.gitignore`.

## Naming

| Item | Rule | Example |
|---|---|---|
| Files | lowercase, underscores | `fsm_automatic.c`, `software_timer.h` |
| Functions | `<module>_<action>()` | `button_read()`, `traffic_set_state()` |
| Constants / enum values | UPPER_CASE | `AUTO_RED1_GREEN2`, `TICK_MS` |
| Global variables | lowercase, declared in `global.h/.c` | `time_red`, `status` |
| `.ioc` User Labels | `TL<road>_<A/B>`, `BTN_<n>` | `TL1_A`, `BTN_3` |

## Layering

1. Only supporting modules and `main.c` may call `HAL_*` functions.
2. `fsm_*.c` files use module functions only (`traffic_set`, `isButtonPressed`, `lcd_print`, …).
3. No `HAL_Delay` after the scheduler is in place — everything runs as a task.
