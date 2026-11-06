# Assignment — Session: 07

**Deadline: 2026-11-22 23:59:00**

## Exercise_1 — Reading the Capacitive Touch Panel Over I2C [review-only]

Read the FT6336U touch controller over I2C and draw a trail using your Session 06 display code.

### Hardware

Use **MSP3526**, the module with capacitive touch. MSP3525 cannot complete this exercise; contact the instructor if you have that variant.

Keep the Session 06 wiring and add:

| Module pin | ESP32-S3 |
| --- | --- |
| `CTP_SDA` | GPIO4 |
| `CTP_SCL` | GPIO5 |
| `CTP_RST` | GPIO6, active LOW |
| `CTP_INT` | GPIO7, wired but unused |

Keep this wiring for the later LVGL work.

### Requirements

- Use `driver/i2c_master.h`: `i2c_new_master_bus()`, `i2c_master_bus_add_device()` and `i2c_master_transmit_receive()`. Do not use `driver/i2c.h`.
- Reset the controller through `CTP_RST` before accessing it.
- At startup, read and log chip ID and vendor ID in hex; compare them with the FT6336U documentation before continuing.
- Poll about every 50 ms. When touched, read the first point, log its coordinates with `ESP_LOGI`, and draw a filled square of roughly 20×20 pixels centred on it.
- Read registers with one combined write-then-read transaction. Read all four coordinate bytes together and mask the high bytes before combining them.
- Reuse the SPI display code and `lcd_fill_rect()` at 480×320 landscape. Map touch coordinates to the display orientation, then clamp the square so it stays fully inside the screen.
- Keep the trail after release; stop drawing and logging touch coordinates when no finger is present.
- Use finite I2C timeouts. On a runtime read failure, log `ESP_LOGW` and continue with the next cycle; do not use `ESP_ERROR_CHECK()` inside the polling loop. It is allowed during setup.
- Use named constants for register addresses, the I2C address and bit masks.

### Hints

| Setting / register | Value |
| --- | --- |
| I2C address / suggested clock | `0x38` / 400 kHz |
| Suggested read timeout | 100 ms |
| Touch count | `0x02`, low nibble |
| First point: XH, XL, YH, YL | `0x03`–`0x06` |
| Chip ID / vendor ID | `0xA3` / `0xA8` |
| Coordinate high-byte mask | `0x0F` |

The touch controller's native orientation differs from the landscape display. Log raw coordinates along each screen edge to determine axis swaps and inversions. Record the mapping and observed axis ranges in a code comment.

### Verification

Confirm startup ID reads succeed and squares follow your fingertip across the screen, including all edges. The trail must remain visible after release, with no drawing outside the display.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
