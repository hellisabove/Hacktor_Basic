# Hacktor Watch Zephyr Port

This repository contains the Zephyr v4.3 port of the Hacktor Watch firmware, originally written in Arduino/C++. The project has been fully refactored into pure C and leverages Zephyr's native driver APIs for a lightweight and efficient smartwatch experience.

## Features

- **Analog Watchface:** Smooth, flicker-free rendering of hour, minute, and second hands.
- **Info Screen:** Real-time data display including:
    - Battery percentage and precise voltage.
    - Hardware step counter (Pedometer).
    - Digital time readout.
- **Optimized Graphics:** Lightweight C graphics library with SPI burst optimizations for the GC9A01 display.
- **Native Zephyr Drivers:** 
    - **MAX17048 Fuel Gauge** via Zephyr's Fuel Gauge API.
    - **LSM6DS3 IMU** via Zephyr's Sensor API (with hardware pedometer support).
    - **GC9A01 LCD** via Zephyr's Display and MIPI-DBI APIs.
- **Power Management:** Automatic screen timeout after 10 seconds of inactivity.
- **Input Handling:** Physical button wakes the screen and toggles between the Watchface and Info screens.
- **Wall-Clock Time:** Accurate timekeeping initialized from the build date and updated using system uptime.

## Hardware Support

Target board: `esp32s3_devkitc/esp32s3/procpu`

### Pin Mapping

| Component | Signal | GPIO |
|-----------|--------|------|
| **LCD**   | SCK    | 12   |
|           | MOSI   | 11   |
|           | MISO   | 13   |
|           | CS     | 4    |
|           | DC     | 5    |
|           | RST    | 3    |
|           | BL     | 7    |
|           | PWR EN | 18 (gpio-hog) |
| **I2C**   | SDA    | 8    |
|           | SCL    | 9    |
| **Touch** | INT    | 1    |
|           | RST    | 2    |
| **Btn**   | BOOT0  | 0    |
| **IMU**   | INT1   | 17   |
|           | INT2   | 14   |

## Project Structure

- `src/main.c`: Application entry point and top-level state machine.
- `src/graphics.c`: Custom graphics primitives (lines, circles, text).
- `src/watchface.c`: Analog watchface rendering logic.
- `src/info_screen.c`: Data display screen implementation.
- `src/imu.c`: IMU initialization and hardware pedometer configuration.
- `src/battery_monitor.c`: Power monitoring using the Zephyr Fuel Gauge API.
- `src/time_keeper.c`: Accurate wall-clock time management.
- `app.overlay`: Devicetree configuration for the ESP32-S3 and peripherals.
- `prj.conf`: Zephyr subsystem and driver configuration.

## Getting Started

### Prerequisites

- Zephyr SDK v0.17.4+
- Zephyr RTOS v4.3.0

You can use the provided setup scripts (`zephyr-v4.3.99-arch-setup.sh` or `zephyr-v4.3.99-ubuntu-setup.sh`) to prepare your environment.

### Building

To build the project:

```bash
./build.sh
```

To perform a clean rebuild:

```bash
./build.sh --pristine
```

### Flashing

Flash the firmware to your Hacktor Watch:

```bash
./build.sh --flash --port /dev/ttyACMx
```
replace the x with the number showed when listing the ttys

## USB Console & Logging

The project uses Zephyr's logging subsystem. You can view real-time logs via the ESP32-S3 native USB port:

```bash
# Example using minicom
minicom -D /dev/ttyACMx
```

The logs are color-coded and provide information on hardware initialization, sensor status, and battery levels.
