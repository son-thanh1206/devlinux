# Assignment — Session: 06

**Deadline: 2026-11-15 23:59:00**

## Exercise_1 — Driving the ST7796 Display Over SPI [review-only]

Drive the ST7796U display on `SPI2_HOST` in **480×320 landscape, RGB565**:

1. Reset and initialise the controller, then turn the display on.
2. Show three horizontal bars — red, green, blue from top to bottom — covering the screen in roughly equal thirds for 3 s.
3. Cycle full-screen red → green → blue → white → black, 1 s per colour, continuously.

### Hardware

Use the LCD Wiki 3.5" IPS SPI module: **MSP3525** or **MSP3526**.

| Module pin | ESP32-S3 |
| --- | --- |
| `VCC`, `GND` | **5V**, GND |
| `SCK`, `SDI(MOSI)`, `SDO(MISO)` | GPIO12, 11, 13 |
| `LCD_CS`, `LCD_RS`, `LCD_RST` | GPIO10, 9, 14 |
| `LED` | GPIO2, HIGH = backlight on |
| `SD_CS`, touch pins | Unconnected |

Use this pinout for the following sessions. Wire MISO even though this exercise does not read it. Signals use 3.3 V GPIO levels; module power is 5 V. Confirm your module's labels against the [module wiki](https://www.lcdwiki.com/3.5inch_IPS_SPI_Module_ST7796).

### Requirements

- Use `driver/spi_master.h`, `spi_bus_initialize()`, `spi_bus_add_device()` and `spi_device_polling_transmit()`. Do not use the `esp_lcd` panel API.
- Provide separate command and data helpers. Only these helpers control `LCD_RS`: LOW for commands, HIGH for data.
- Implement a window helper using `CASET`, `RASET`, `RAMWR`, and `lcd_fill_rect(x, y, w, h, colour)`. Use the same fill helper for rectangles and full-screen fills.
- Set landscape orientation through `MADCTL`; set `COLMOD = 0x55`. Send coordinates and RGB565 pixels high byte first.
- Buffer multiple pixels per transfer. Use a named chunk size and set `max_transfer_sz` at least that large; be ready to explain your choice.
- Check all SPI and GPIO calls that return `esp_err_t`. `ESP_ERROR_CHECK()` is allowed throughout.
- Use named constants for commands, pins and dimensions. Cite the source in a comment if you copy a vendor initialisation sequence.

### Hints

Start with SPI mode 0 at 20 MHz. The bring-up sequence includes hardware reset, `SWRESET`, `SLPOUT`, `MADCTL`, `COLMOD` and `DISPON`; follow the ST7796U datasheet and vendor demo for register values and mandatory delays.

Useful commands: `SWRESET=0x01`, `SLPOUT=0x11`, `INVON=0x21`, `DISPON=0x29`, `CASET=0x2A`, `RASET=0x2B`, `RAMWR=0x2C`, `MADCTL=0x36`, `COLMOD=0x3A`.

### Verification

Check full-screen coverage, bar order, colour accuracy and timing. If the screen stays black, check command/data selection and reset delays. For inverted colours, check `INVON`; for swapped red/blue, check `MADCTL`'s colour-order bit. Comment on any colour-order adjustment.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
