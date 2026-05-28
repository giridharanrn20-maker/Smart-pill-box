# Arduino Code

This folder contains the Arduino programs used in the Smart Medication Reminder Pill Box project.

## Files

### `smart_pillbox_main.ino`
Main Arduino program for the pill box.

It handles:
- Reading real-time from DS3231 RTC
- Comparing current time with medicine schedule
- Displaying messages on 16x2 I2C LCD
- Blinking LEDs for Morning, Afternoon, and Night dose
- Activating buzzer during reminder time
- Reading push button confirmation
- Sending TAKEN/MISSED events to Python gateway through Serial

### `rtc_time_setter.ino`
Separate Arduino program used to set or update the DS3231 RTC time during demonstration.

This is useful because the RTC keeps real-world time even after Arduino is restarted.

## Hardware Pins

| Component | Arduino Pin |
|---|---|
| Morning LED | D4 |
| Afternoon LED | D5 |
| Night LED | D6 |
| Buzzer | D7 |
| Push Button | D10 |
| LCD SDA | A4 |
| LCD SCL | A5 |
| RTC SDA | A4 |
| RTC SCL | A5 |

## Libraries Required

Install these libraries in Arduino IDE:

- `Wire`
- `RTClib`
- `LiquidCrystal_I2C`

## Notes

The main reminder system works locally using Arduino and RTC.  
Firebase and the web dashboard are only used for cloud monitoring and remote updates.
