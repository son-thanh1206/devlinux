# Assignment — Session: 13

**Deadline: 2026-12-13 23:59:00**

## Exercise_1 — A Deep-Sleep Doorbell [review-only]

Build a doorbell that sleeps between button presses and periodic heartbeats.

| Boot / wake cause | Action before returning to deep sleep |
| --- | --- |
| Power-on / reset | Log startup; press count starts at zero on fresh power-on |
| BOOT button, ext0 | Blink the LED 3 times, increment the press count and log it |
| Timer, after 60 s of sleep | Log a heartbeat with the current count |

### Hardware and Monitoring

ESP32-S3 DevKitC-1 with the display disconnected.

| Part | Connection |
| --- | --- |
| BOOT button | GPIO0, ext0 wake at LOW |
| External LED | GPIO15 → 220 Ω–330 Ω → LED anode; cathode to GND |

Monitor through the board's **UART** port (CP2102N) with the ESP-IDF console on **UART0**. Native USB Serial/JTAG disconnects during deep sleep. Session 05 moved logs to USB, so check this project's console configuration.

If flashing fails while the board sleeps: hold BOOT, tap RESET, release BOOT, then flash again.

### Requirements

- Use `esp_sleep_enable_ext0_wakeup()`, `esp_sleep_enable_timer_wakeup()` and `esp_deep_sleep_start()` from `esp_sleep.h`.
- Read `esp_sleep_get_wakeup_cause()` on every boot and distinguish button, timer and power-on/reset in the logs.
- Store the press count as `RTC_DATA_ATTR`, initialised to zero. Increment it only on button wakes; retain it across deep sleep and clear it after a power cycle.
- After ext0 wake, call `rtc_gpio_deinit()` before configuring and reading the button as a digital GPIO.
- Wait for button release before sleeping. ext0 is level-triggered: holding LOW must count as one press without repeated wakes.
- Configure the button's RTC pull-up with `driver/rtc_io.h` before sleep; a digital GPIO pull-up alone is insufficient.
- Use named constants for pins, timeouts and blink count.

### Hints

Deep-sleep wake restarts execution in `app_main()`; `esp_deep_sleep_start()` does not return. Handle the wake cause, wait for release, arm both wake sources, then sleep.

For the timer, use `60ULL * 1000000ULL` microseconds. A suggested LED half-period is 150 ms. Compare `RTC_DATA_ATTR` with Session 12's `RTC_NOINIT_ATTR` to understand why this counter needs no magic number.

### Verification

Test a fresh boot, repeated button presses, a held button and a 60 s heartbeat. The heartbeat must preserve the count; reconnecting power must clear it.

Current measurement is optional and ungraded. DevKit LEDs, regulator and USB-UART bridge draw current even while the S3 sleeps, so do not expect chip-only microamp readings at the USB supply.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
