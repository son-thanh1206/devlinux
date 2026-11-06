# Assignment — Session: 03

**Deadline: 2026-11-08 23:59:00**

## Exercise_1 — Button-Driven 7-Segment Counter, Register Level [review-only]

Build a single-digit counter that starts at `0` and wraps between `0` and `9`.

| Button gesture | Action |
| --- | --- |
| Single click | `+1` |
| Double click | `-1` |
| Long press | `+1` on entering long-press mode, then every 500 ms until release |

### Hardware

ESP32-S3 DevKitC-1, a single-digit 7-segment display, a push-button, breadboard and wires. Use one 220 Ω–330 Ω resistor per connected segment; do not share a resistor on the common pin.

| Signal | GPIO / connection |
| --- | --- |
| Segments `a`, `b`, `c`, `d` | GPIO4, 5, 6, 7 |
| Segments `e`, `f`, `g` | GPIO15, 16, 17 |
| `dp` (optional) | GPIO18 |
| Button | GPIO14 to GND, internal pull-up only |
| Common pin | GND for common cathode; 3V3 for common anode |

### Requirements

- **Register access only:** configure `IO_MUX`, GPIO matrix routing and output enables; use `GPIO_OUT_*`, `GPIO_ENABLE_*` and `GPIO_IN_REG`. Do not use `driver/gpio.h` or any `gpio_*()` API.
- **Polling only:** no GPIO interrupts. Debounce both press and release; pressed reads LOW.
- Support your display type with one digit table and a named polarity inversion. State common anode or common cathode in a header comment in `main.c`.
- A long press must stop repeating on release and must not also count as a click.
- Defer a single click until the double-click window expires, so a double click produces only `-1`.
- Use named constants for register addresses, bit positions, GPIOs and timing thresholds. Access registers through `volatile` pointers.

### Hints

Suggested thresholds: debounce 25 ms, double-click window 350 ms, long press 800 ms. The repeat period is fixed at 500 ms.

Use the TRM chapter **IO MUX and GPIO Matrix** to find the pad function, input buffer, pull-up, output routing and output-enable settings. All assigned GPIOs use the first register bank. Build segment masks and consider `GPIO_OUT_W1TS_REG` / `GPIO_OUT_W1TC_REG` when updating the display.

### Verification

Check both wrap directions, single and double clicks, continuous long-press repeats, and release without an extra count. Be ready to explain the single-click delay. Serial logging is optional.

### Submission

Submit an ESP-IDF project:

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
