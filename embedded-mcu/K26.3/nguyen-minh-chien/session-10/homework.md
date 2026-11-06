# Assignment — Session: 10

**Deadline: 2026-12-06 23:59:00**

## Exercise_1 — A Mini Voltmeter With a Threshold Alarm [review-only]

Read a potentiometer with ADC1, report its voltage every 500 ms, and drive an LED alarm with hysteresis.

| Measured voltage | Alarm action |
| --- | --- |
| Above 2000 mV | LED on |
| Below 1800 mV | LED off |
| 1800–2000 mV, inclusive | Keep the previous state |

### Hardware

Use an ESP32-S3 DevKitC-1, a 1 kΩ–100 kΩ linear potentiometer and a single-colour LED. Disconnect the display module for Sessions 10–13: GPIO4 is shared with its touch SDA line.

| Connection | ESP32-S3 |
| --- | --- |
| Potentiometer outer pins | 3V3 and GND |
| Potentiometer wiper | GPIO4, ADC1 `ADC_CHANNEL_3` |
| LED anode through 220 Ω–330 Ω | GPIO15 |
| LED cathode | GND |

Use an external LED; the onboard WS2812 cannot be controlled with `gpio_set_level()`.

### Requirements

- Use `esp_adc/adc_oneshot.h`: `adc_oneshot_new_unit()`, `adc_oneshot_config_channel()` and `adc_oneshot_read()`. Do not use `driver/adc.h`.
- Configure ADC1 with `ADC_ATTEN_DB_12` and document its supported input range. The potentiometer reaches 3.3 V, but the ADC's effective calibrated range at this attenuation is about **0–2.9 V**; do not expect accurate full-scale 3.3 V readings. See the [ESP32-S3 datasheet, ADC characteristics](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf#page=66).
- Average at least 8 samples per reading. Check `adc_oneshot_read()` results and skip failed samples without rebooting. Use `ESP_ERROR_CHECK()` for setup only.
- Use the **curve-fitting** calibration scheme from `esp_adc/adc_cali.h` and `esp_adc/adc_cali_scheme.h`. Create the handle once and convert with `adc_cali_raw_to_voltage()`.
- If calibration is unavailable, warn and log raw counts only. Do not calculate millivolts with a custom formula.
- Start with the LED off. Log raw counts and calibrated millivolts every 500 ms; log an extra line only when the alarm changes state.
- Use separate named constants for the two thresholds, plus constants for pins, channel, attenuation, period and sample count.

### Hints

You may average raw counts before conversion or average the converted millivolts; be ready to explain your choice. Hysteresis requires remembering the previous LED state.

### Verification

Turn the potentiometer slowly through both thresholds. For example, 2057 → 1958 → 1745 mV must produce LED states ON → ON → OFF. Readings near the upper threshold must not make the LED flicker.

### Submission

```text
Exercise_1/
├── CMakeLists.txt
└── main/
    ├── CMakeLists.txt
    └── main.c
```
