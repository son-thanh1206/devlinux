# Assignment — Session: 11

**Deadline: 2026-12-06 23:59:00**

## Exercise_1 — A Stopwatch Driven by a Hardware Timer [review-only]

Build a stopwatch that starts **paused at zero**.

| Input / state | Behaviour |
| --- | --- |
| Short BOOT press | Start or pause |
| BOOT held for at least 1 s | Reset to zero and pause; do not also toggle start/pause |
| Running | LED blinks at 1 Hz |
| Paused | LED off |

Print elapsed time as `mm:ss.mmm` every 100 ms.

### Hardware

ESP32-S3 DevKitC-1 with the display disconnected.

| Part | Connection |
| --- | --- |
| BOOT button | GPIO0, input with pull-up, pressed = LOW; never configure as output |
| External LED | GPIO15 → 220 Ω–330 Ω → LED anode; cathode to GND |

### Requirements

- Use `driver/gptimer.h` with a **1 ms auto-reload alarm**. Do not rearm the alarm in the callback.
- The alarm callback may only increment a `volatile` millisecond counter and return. No logging, delays or GPIO operations.
- Derive elapsed time only from the GPTimer-driven counter, never from accumulated `vTaskDelay()` periods or `esp_timer_get_time()`.
- Handle and debounce the button in a task. Recognise short presses without also triggering them after a long press.
- Pause/resume must preserve accumulated running time and exclude paused time.
- Protect counter resets against concurrent callback updates, including across cores. `volatile` does not make `elapsed_ms++` atomic; be ready to explain your protection.
- Read the shared counter once into a local snapshot before formatting each output.
- Use named constants for pins, periods and durations.
- Check every `gptimer_*` return value. Use `ESP_ERROR_CHECK()` during setup only.

### Hints

A 1 MHz resolution with an alarm count of 1000 gives a 1 ms tick. Use `reload_count = 0` and `auto_reload_on_alarm = true`.

Lifecycle: `gptimer_new_timer()` → `gptimer_set_alarm_action()` → `gptimer_register_event_callbacks()` → `gptimer_enable()`. Start only when requested. Investigate `gptimer_stop()` / `gptimer_start()` for pause/resume.

The LED can use the elapsed counter with a 500 ms half-period; no second timer is needed. A suggested button debounce is 20 ms.

### Verification

Compare a one-minute run with a reference stopwatch to catch major timing errors; manual starts cannot verify millisecond accuracy. Pause for several seconds, resume and confirm paused time is excluded. Test reset while running and paused, including release after a long press.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
