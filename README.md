# rootlog
## The Privacy-First, Local Data Logging Solution

RootLog is a dedicated environmental monitoring system designed to put data ownership back into the hands of the user. Unlike cloud-based smart home sensors, RootLog operates 100% locally. It monitors ambient conditions and air quality, storing every data point on a MicroSD card for private analysis.
Project Description

    Privacy-Focused: No Wi-Fi required; no data ever leaves the device.

    Robust Logging: Saves data to a MicroSD card every 60 seconds.

    Data Integrity: Features a "Flush" strategy that physically saves data to the card every 5 minutes to prevent loss during power failure.

    Power Management: Supports a Deep Sleep mode (activated by a 2-second button hold) to conserve battery during portable use.

    Multi-View Display: A 0.91" OLED screen cycles through 4 distinct data pages with a single tap.

## BOM

    Microcontroller: ESP32-C3 SuperMini

    Environmental Sensor: BME280 (Temp, Hum, Pres)

    Air Quality Sensor: SGP30 (eCO2, TVOC)

    Real-Time Clock: DS3231 RTC

    Display: 0.91" I2C OLED (128x32)

    Storage: MicroSD Card Module

## Wiring Map

Component	Pin Function	ESP32-C3 Pin
I2C Bus	SDA	GPIO 8
I2C Bus	SCL	GPIO 9
MicroSD	CS (Chip Select)	GPIO 7
MicroSD	MOSI	GPIO 6
MicroSD	MISO	GPIO 5
MicroSD	SCK (Clock)	GPIO 4
Control	Push Button	GPIO 2
Software & Setup
Required Libraries

## Usage Instructions

    Boot: Upon power-up, the OLED will display "ROOTLOG BOOTING..." while initializing sensors.

    Cycle Pages: Short-press the button to cycle between:

        Temperature (°C)

        Humidity (%) and Pressure (hPa)

        Air Quality (eCO2 ppm and TVOC ppb)

        Current System Time

    Power Off: Hold the button for 2 seconds. The screen will show "BYE BYE" and enter deep sleep.

    Wake Up: Press the button again to resume logging.

    Note on Calibration: The SGP30 air quality sensor requires approximately 15 seconds of operation to stabilize its internal baseline before reporting accurate eCO2 and TVOC values.
