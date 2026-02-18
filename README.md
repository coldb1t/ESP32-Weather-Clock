# ESP32 Module

ESP32 project displaying real-time clock, temperature, and humidity on a 128×64 OLED screen. 
Built with FreeRTOS tasks and a custom thread-safe data wrapper.

## Hardware

| Component | Pin |
|-----------|-----|
| SSD1306 OLED (128×64, I2C) | SDA: 21, SCL: 22 |
| DS1307 RTC (I2C) | SDA: 21, SCL: 22 |
| DHT22 temperature/humidity sensor | GPIO 15 |
| Buzzer (LEDC PWM) | GPIO 25, channel 0 |
| Button | GPIO 23 (INPUT_PULLUP) |

## Features

- Real-time clock display (HH:MM) with seconds bar
- Temperature and humidity monitoring
- Heap usage monitor
- Thread-safe sensor reads via custom `core::locked<T>` mutex wrapper

## FreeRTOS Tasks

| Task | Stack | Priority | Period |
|------|-------|----------|--------|
| `SENSOR_TASK` | 2048 B | 1 | 5000 ms |
| `RENDER_TASK` | 8192 B | 1 | 60 ms |
| `BUTTON_TASK` | 1024 B | 2 | 50 ms |

## Resource Usage

```
RAM:   [=         ]   6.7% (used 22096 bytes from 327680 bytes)
Flash: [==        ]  24.0% (used 315013 bytes from 1310720 bytes)
```
