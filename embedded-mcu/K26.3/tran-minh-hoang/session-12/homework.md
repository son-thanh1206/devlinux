# Assignment — Session: 12

**Deadline: 2026-12-13 23:59:00**

## Exercise_1 — Two Tasks Under a Watchdog [review-only]

Monitor two tasks with the Task Watchdog Timer (TWDT), deliberately hang one, and count watchdog resets across reboots.

### Hardware

ESP32-S3 DevKitC-1 only, with the display disconnected. Use BOOT on GPIO0 as an active-LOW input with pull-up.

### Requirements

- Create tasks named exactly `"sensor"` and `"report"` with `xTaskCreate()`. Their periods are **200 ms** and **1 s**, respectively.
- Use `esp_task_wdt.h`. Each task subscribes itself with `esp_task_wdt_add(NULL)` and calls `esp_task_wdt_reset()` once per iteration. Do not feed on another task's behalf.
- Configure a **3 s timeout** and `trigger_panic = true` with `esp_task_wdt_reconfigure()`, so a timeout reboots the board.
- BOOT requests a hang through a `volatile` flag or task notification. Inside `report`, block with `xSemaphoreTake(..., portMAX_DELAY)` on a semaphore that is never given. Do not substitute a busy loop or simply skip feeding.
- On every boot, log `esp_reset_reason()` as a readable name and the total watchdog-reset count.
- Keep both the counter and its magic-number variable in `RTC_NOINIT_ATTR` memory. If the magic is invalid or the reset reason is power-on, initialise the count to zero and write the magic.
- Increment the count only for a watchdog reset. It must survive those resets and return to zero after a power cycle.
- Use named constants for periods, timeout, pin and magic value.

### Hints

Check `CONFIG_ESP_TASK_WDT_INIT`: `esp_task_wdt_reconfigure()` requires an already initialised TWDT. Check its return value and choose `idle_core_mask` deliberately.

Trigger one timeout and inspect the actual reset reason before implementing the watchdog-reset check. Do not rely solely on an assumed value.

`RTC_NOINIT_ATTR` leaves memory uninitialised at startup, so validate the magic number before using the retained counter.

### Verification

1. Fresh power-on logs a readable reason and count `0`.
2. Press BOOT. The TWDT log identifies `report` as the only task that missed feeding; `sensor` continues running until reset.
3. After reboot, the count is `1`; repeat to obtain `2`, `3`, etc.
4. Disconnect and reconnect power: the count returns to `0`.

The timeout is measured from the last watchdog feed, not from the button press.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
