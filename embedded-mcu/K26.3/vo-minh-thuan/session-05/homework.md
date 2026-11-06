# Assignment — Session: 05

**Deadline: 2026-11-15 23:59:00**

## Exercise_1 — Interrupt-Driven UART Command Console [review-only]

Build a UART0 command console that controls the onboard RGB LED.

### Hardware and Setup

Use an ESP32-S3 DevKitC-1 and **two USB cables**, one for each port:

| Board port | Use |
| --- | --- |
| **USB** — native USB Serial/JTAG | Flashing and `ESP_LOGI` / `ESP_LOGW` output |
| **UART** — CP2102N bridge, UART0 on GPIO43 TX / GPIO44 RX | Command input and character echo, 115200-8-N-1 |

Move the ESP-IDF log console to USB Serial/JTAG so it does not share UART0 with your commands:

```text
idf.py menuconfig
  → Component config → ESP System Settings → Channel for console output
  → USB Serial/JTAG Controller
```

Open `idf.py monitor` on the USB port and a serial terminal on the UART port's COM number.

The onboard WS2812 uses **GPIO48 on v1.0** or **GPIO38 on v1.1**. Check your board and state its revision and LED GPIO in a header comment in `main.c`. Drive it with the RMT-based `led_strip` component:

```sh
idf.py add-dependency "espressif/led_strip^2.4.1"
```

### Requirements

- Start with the LED off. Echo typed characters; support Backspace and DEL, using `"\b \b"` to erase a character on the terminal.
- Submit a line on Enter (`\r` or `\n`) and handle these commands:

| Command | LED state |
| --- | --- |
| `LED_ON` | White |
| `LED_OFF` | Off |
| `RED`, `GREEN`, `BLUE` | Matching colour |
| Anything else | `ESP_LOGW`; retain the current LED state and continue |

- Configure UART0 with `uart_param_config()` and `uart_set_pin()`. Install it with `uart_driver_install()` and an event queue (`QueueHandle_t`).
- Process UART events in a dedicated task created with `xTaskCreatePinnedToCore()`. Wait on the queue, then read `UART_DATA` bytes; do not poll `uart_read_bytes()` in a tight loop.
- Bounds-check the command buffer, reserve space for the null terminator, and safely reject or truncate overlong lines.
- On `UART_FIFO_OVF` or `UART_BUFFER_FULL`, call `uart_flush_input()`, reset the event queue and line state, and log a warning.
- Log recognised commands and resulting LED changes. Use named constants for command strings and buffer sizes.

### Verification

Test every command, unknown input, both editing keys and an overlong line. Typed text and echo must appear in the UART terminal; logs must appear in the USB monitor.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    ├── idf_component.yml    # led_strip dependency
    └── main.c
```
