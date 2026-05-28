#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin())
  {
    Serial.println("RTC not found");
    while (1);
  }

  rtc.adjust(DateTime(2026, 5, 22,12, 45 ,0));

  Serial.println("RTC time changed successfully");

  DateTime now = rtc.now();

  Serial.print("Current RTC Time: ");

  if (now.hour() < 10) Serial.print("0");
  Serial.print(now.hour());

  Serial.print(":");

  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());

  Serial.print(":");

  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());
}

void loop()
{
}