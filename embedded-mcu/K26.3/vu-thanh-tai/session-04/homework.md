# Assignment — Session: 04

**Deadline: 2026-11-08 23:59:00**

## Exercise_1 — The Same Counter, Interrupt-Driven [review-only]

Rebuild Session 03's counter using a GPIO interrupt for button detection. Keep the same wiring, register-level display code, initial digit `0` and wrap behaviour.

| Button gesture | Action |
| --- | --- |
| Single click | `+1` |
| Double click | `-1` |
| Long press | `+1` on entering long-press mode, then every 500 ms until release |

### Hardware

Use the Session 03 circuit unchanged: button on **GPIO14** with internal pull-up; segments `a`–`g` on GPIO4, 5, 6, 7, 15, 16, 17, each through a 220 Ω–330 Ω resistor.

### Requirements

- Configure the button with `driver/gpio.h`, `gpio_config()` and `GPIO_INTR_ANYEDGE` to capture both press (1 → 0) and release (0 → 1). Release detection is needed to end a hold. Install the ISR service with `gpio_install_isr_service()` and register the handler with `gpio_isr_handler_add()`.
- Mark the ISR `IRAM_ATTR`. It may only read the button level, timestamp the edge and save the state. No logging, delays, gesture decoding or display writes.
- Read the button pin **only inside the ISR**. Removing the interrupt must prevent all button detection.
- Keep the button state and separate press/release timestamps in `volatile` variables. Synchronise shared reads and writes so the main loop gets a consistent snapshot; `volatile` alone does not protect multi-field or 64-bit accesses.
- Decode gestures in `app_main()` and end each iteration with `vTaskDelay(pdMS_TO_TICKS(10))`. No separate task or inter-task communication is required.
- Debounce the saved edges. Use elapsed-time checks for double-click and repeat deadlines; do not block while waiting for them. Long-press repeats must continue without new edges, and release must not add a click.
- Copy the register-level display code from Session 03 into this project; use the Driver API only for the button.

### Hints

Use `esp_timer_get_time()` for microsecond timestamps and deadline checks. A saved state is not an event history: keep the loop short enough to handle presses and releases without losing them.

### Questions

Answer in `isr_notes.md`, one short paragraph each:

1. Why must the shared button-state variable be `volatile`? What could happen if the main loop reused a cached value?
2. Why should the ISR only save state and timestamps? Why do logging and delays belong in the main loop, and why is `vTaskDelay()` invalid inside the ISR?
3. Compare polling and interrupts: edge-detection latency, time until the display updates, CPU usage while idle, and implementation difficulty. State your timing assumptions; interrupts do not automatically make the display respond faster.

### Verification

Confirm all Session 03 gestures and wrap cases still work, including no extra count after a long press. Optional gesture logs must come from the main loop. Multiple events from one physical click indicate a debounce problem.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
├── isr_notes.md
└── main/
    ├── CMakeLists.txt
    └── main.c
```
