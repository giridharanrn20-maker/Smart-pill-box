#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

int morningLED = 4;
int afternoonLED = 5;
int nightLED = 6;

int buzzer = 7;
int button = 10;

bool reminderActive = false;
bool lowStockAlertActive = false;

bool morningHandled = false;
bool afternoonHandled = false;
bool nightHandled = false;

int currentReminder = 0;

unsigned long reminderStartMillis = 0;
const unsigned long reminderTimeout = 60000;

unsigned long lowStockStartMillis = 0;
const unsigned long lowStockAlertDuration = 15000;

unsigned long lastBeepMillis = 0;
bool buzzerState = false;

const unsigned long beepOnTime = 200;
const unsigned long beepOffTime = 400;

unsigned long lastLedBlinkMillis = 0;
bool ledBlinkState = false;
const unsigned long ledBlinkInterval = 300;

String cloudIncomingLine = "";

int cloudMorningHour = 8;
int cloudMorningMinute = 0;
int cloudMorningDoseCount = 2;
bool cloudMorningEnabled = true;

int cloudAfternoonHour = 13;
int cloudAfternoonMinute = 30;
int cloudAfternoonDoseCount = 1;
bool cloudAfternoonEnabled = true;

int cloudNightHour = 20;
int cloudNightMinute = 0;
int cloudNightDoseCount = 2;
bool cloudNightEnabled = true;

int morningStock = 5;
int afternoonStock = 5;
int nightStock = 5;

int lastDisplayedSecond = -1;

void handleCloudSerial();
void checkReminderTrigger(int hour, int minute);
void checkInventoryAndStartReminder(int reminderNumber);
void startReminder(int reminderNumber);
void startLowStockAlert(int reminderNumber);
void updateBuzzerBeep();
void updateReminderLED();
void runReminder(int hour, int minute);
void runLowStockAlert();
void completeCurrentDose(const char* status, int hour, int minute);
void sendDoseEvent(const char* doseType, const char* status, int scheduledHour, int scheduledMinute, int actualHour, int actualMinute, int doseCount);
void sendLowStockEvent(const char* doseType, int scheduledHour, int scheduledMinute, int stock, int requiredDose);
void displayCurrentTime(int hour, int minute, int second);
void lcdPrintTwoDigits(int value);
void printSerialTwoDigits(int value);
String cloudGetPart(String data, int index);
void printCloudSchedule();
void printInventory();
void handleCloudSetCommand(String command);
void handleStockSetCommand(String command);

void setup()
{
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  rtc.begin();

  pinMode(morningLED, OUTPUT);
  pinMode(afternoonLED, OUTPUT);
  pinMode(nightLED, OUTPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  digitalWrite(morningLED, LOW);
  digitalWrite(afternoonLED, LOW);
  digitalWrite(nightLED, LOW);
  digitalWrite(buzzer, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Pill Box");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();

  printCloudSchedule();
  printInventory();
}

void loop()
{
  DateTime now = rtc.now();

  handleCloudSerial();

  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  if (lowStockAlertActive)
  {
    runLowStockAlert();
    return;
  }

  if (!reminderActive)
  {
    checkReminderTrigger(hour, minute);
  }

  if (reminderActive)
  {
    runReminder(hour, minute);
  }
  else
  {
    displayCurrentTime(hour, minute, second);
  }

  if (hour == 0 && minute == 0 && second < 2)
  {
    morningHandled = false;
    afternoonHandled = false;
    nightHandled = false;
  }
}

void checkReminderTrigger(int hour, int minute)
{
  if (cloudMorningEnabled && hour == cloudMorningHour && minute == cloudMorningMinute && !morningHandled)
  {
    checkInventoryAndStartReminder(1);
  }
  else if (cloudAfternoonEnabled && hour == cloudAfternoonHour && minute == cloudAfternoonMinute && !afternoonHandled)
  {
    checkInventoryAndStartReminder(2);
  }
  else if (cloudNightEnabled && hour == cloudNightHour && minute == cloudNightMinute && !nightHandled)
  {
    checkInventoryAndStartReminder(3);
  }
}

void checkInventoryAndStartReminder(int reminderNumber)
{
  if (reminderNumber == 1)
  {
    if (morningStock < cloudMorningDoseCount)
    {
      morningHandled = true;
      sendLowStockEvent("MORNING", cloudMorningHour, cloudMorningMinute, morningStock, cloudMorningDoseCount);
      startLowStockAlert(1);
      return;
    }
  }
  else if (reminderNumber == 2)
  {
    if (afternoonStock < cloudAfternoonDoseCount)
    {
      afternoonHandled = true;
      sendLowStockEvent("AFTERNOON", cloudAfternoonHour, cloudAfternoonMinute, afternoonStock, cloudAfternoonDoseCount);
      startLowStockAlert(2);
      return;
    }
  }
  else if (reminderNumber == 3)
  {
    if (nightStock < cloudNightDoseCount)
    {
      nightHandled = true;
      sendLowStockEvent("NIGHT", cloudNightHour, cloudNightMinute, nightStock, cloudNightDoseCount);
      startLowStockAlert(3);
      return;
    }
  }

  startReminder(reminderNumber);
}

void startReminder(int reminderNumber)
{
  reminderActive = true;
  currentReminder = reminderNumber;
  reminderStartMillis = millis();

  digitalWrite(morningLED, LOW);
  digitalWrite(afternoonLED, LOW);
  digitalWrite(nightLED, LOW);
  digitalWrite(buzzer, LOW);

  buzzerState = false;
  ledBlinkState = false;

  lastBeepMillis = millis();
  lastLedBlinkMillis = millis();

  lcd.clear();
}

void startLowStockAlert(int reminderNumber)
{
  lowStockAlertActive = true;
  reminderActive = false;
  currentReminder = reminderNumber;
  lowStockStartMillis = millis();

  digitalWrite(morningLED, LOW);
  digitalWrite(afternoonLED, LOW);
  digitalWrite(nightLED, LOW);
  digitalWrite(buzzer, LOW);

  buzzerState = false;
  ledBlinkState = false;

  lastBeepMillis = millis();
  lastLedBlinkMillis = millis();

  lcd.clear();
}

void updateBuzzerBeep()
{
  unsigned long currentMillis = millis();

  if (buzzerState && currentMillis - lastBeepMillis >= beepOnTime)
  {
    buzzerState = false;
    digitalWrite(buzzer, LOW);
    lastBeepMillis = currentMillis;
  }
  else if (!buzzerState && currentMillis - lastBeepMillis >= beepOffTime)
  {
    buzzerState = true;
    digitalWrite(buzzer, HIGH);
    lastBeepMillis = currentMillis;
  }
}

void updateReminderLED()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastLedBlinkMillis >= ledBlinkInterval)
  {
    ledBlinkState = !ledBlinkState;
    lastLedBlinkMillis = currentMillis;
  }

  if (currentReminder == 1)
  {
    digitalWrite(morningLED, ledBlinkState);
    digitalWrite(afternoonLED, LOW);
    digitalWrite(nightLED, LOW);
  }
  else if (currentReminder == 2)
  {
    digitalWrite(morningLED, LOW);
    digitalWrite(afternoonLED, ledBlinkState);
    digitalWrite(nightLED, LOW);
  }
  else if (currentReminder == 3)
  {
    digitalWrite(morningLED, LOW);
    digitalWrite(afternoonLED, LOW);
    digitalWrite(nightLED, ledBlinkState);
  }
  else
  {
    digitalWrite(morningLED, LOW);
    digitalWrite(afternoonLED, LOW);
    digitalWrite(nightLED, LOW);
  }
}

void runReminder(int hour, int minute)
{
  updateBuzzerBeep();
  updateReminderLED();

  if (currentReminder == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Morning Dose    ");
    lcd.setCursor(0, 1);
    lcd.print("Take ");
    lcd.print(cloudMorningDoseCount);
    lcd.print(" Medicine ");
  }
  else if (currentReminder == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("Afternoon Dose  ");
    lcd.setCursor(0, 1);
    lcd.print("Take ");
    lcd.print(cloudAfternoonDoseCount);
    lcd.print(" Medicine ");
  }
  else if (currentReminder == 3)
  {
    lcd.setCursor(0, 0);
    lcd.print("Night Dose      ");
    lcd.setCursor(0, 1);
    lcd.print("Take ");
    lcd.print(cloudNightDoseCount);
    lcd.print(" Medicine ");
  }

  if (digitalRead(button) == LOW)
  {
    delay(50);

    if (digitalRead(button) == LOW)
    {
      completeCurrentDose("TAKEN", hour, minute);
    }
  }

  if (reminderActive && millis() - reminderStartMillis >= reminderTimeout)
  {
    completeCurrentDose("MISSED", hour, minute);
  }
}

void runLowStockAlert()
{
  updateBuzzerBeep();
  updateReminderLED();

  if (currentReminder == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Morning LowStock");
    lcd.setCursor(0, 1);
    lcd.print("Refill Medicine ");
  }
  else if (currentReminder == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("Afternoon Stock ");
    lcd.setCursor(0, 1);
    lcd.print("Refill Medicine ");
  }
  else if (currentReminder == 3)
  {
    lcd.setCursor(0, 0);
    lcd.print("Night LowStock  ");
    lcd.setCursor(0, 1);
    lcd.print("Refill Medicine ");
  }

  if (millis() - lowStockStartMillis >= lowStockAlertDuration)
  {
    lowStockAlertActive = false;
    currentReminder = 0;

    digitalWrite(buzzer, LOW);
    digitalWrite(morningLED, LOW);
    digitalWrite(afternoonLED, LOW);
    digitalWrite(nightLED, LOW);

    lcd.clear();
  }
}

void completeCurrentDose(const char* status, int hour, int minute)
{
  if (currentReminder == 1)
  {
    morningHandled = true;

    if (strcmp(status, "TAKEN") == 0)
    {
      morningStock = morningStock - cloudMorningDoseCount;

      if (morningStock < 0)
      {
        morningStock = 0;
      }
    }

    sendDoseEvent("MORNING", status, cloudMorningHour, cloudMorningMinute, hour, minute, cloudMorningDoseCount);
  }
  else if (currentReminder == 2)
  {
    afternoonHandled = true;

    if (strcmp(status, "TAKEN") == 0)
    {
      afternoonStock = afternoonStock - cloudAfternoonDoseCount;

      if (afternoonStock < 0)
      {
        afternoonStock = 0;
      }
    }

    sendDoseEvent("AFTERNOON", status, cloudAfternoonHour, cloudAfternoonMinute, hour, minute, cloudAfternoonDoseCount);
  }
  else if (currentReminder == 3)
  {
    nightHandled = true;

    if (strcmp(status, "TAKEN") == 0)
    {
      nightStock = nightStock - cloudNightDoseCount;

      if (nightStock < 0)
      {
        nightStock = 0;
      }
    }

    sendDoseEvent("NIGHT", status, cloudNightHour, cloudNightMinute, hour, minute, cloudNightDoseCount);
  }

  reminderActive = false;
  currentReminder = 0;

  digitalWrite(buzzer, LOW);
  digitalWrite(morningLED, LOW);
  digitalWrite(afternoonLED, LOW);
  digitalWrite(nightLED, LOW);

  lcd.clear();

  if (strcmp(status, "TAKEN") == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Dose Taken");
    lcd.setCursor(0, 1);
    lcd.print("Stock Updated");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Dose Missed");
    lcd.setCursor(0, 1);
    lcd.print("Logged");
  }

  printInventory();

  delay(2000);
  lcd.clear();
}

void sendDoseEvent(const char* doseType, const char* status, int scheduledHour, int scheduledMinute, int actualHour, int actualMinute, int doseCount)
{
  Serial.print("EVENT,");
  Serial.print(doseType);
  Serial.print(",");
  Serial.print(status);
  Serial.print(",");

  printSerialTwoDigits(scheduledHour);
  Serial.print(":");
  printSerialTwoDigits(scheduledMinute);

  Serial.print(",");

  printSerialTwoDigits(actualHour);
  Serial.print(":");
  printSerialTwoDigits(actualMinute);

  Serial.print(",");
  Serial.println(doseCount);
}

void sendLowStockEvent(const char* doseType, int scheduledHour, int scheduledMinute, int stock, int requiredDose)
{
  Serial.print("EVENT,");
  Serial.print(doseType);
  Serial.print(",LOW_STOCK,");

  printSerialTwoDigits(scheduledHour);
  Serial.print(":");
  printSerialTwoDigits(scheduledMinute);

  Serial.print(",stock=");
  Serial.print(stock);
  Serial.print(",required=");
  Serial.println(requiredDose);
}

void displayCurrentTime(int hour, int minute, int second)
{
  if (second == lastDisplayedSecond)
  {
    return;
  }

  lastDisplayedSecond = second;

  lcd.setCursor(0, 0);
  lcd.print("Current Time    ");

  lcd.setCursor(0, 1);
  lcdPrintTwoDigits(hour);
  lcd.print(":");
  lcdPrintTwoDigits(minute);
  lcd.print(":");
  lcdPrintTwoDigits(second);
  lcd.print("        ");
}

void lcdPrintTwoDigits(int value)
{
  if (value < 10)
  {
    lcd.print("0");
  }

  lcd.print(value);
}

void printSerialTwoDigits(int value)
{
  if (value < 10)
  {
    Serial.print("0");
  }

  Serial.print(value);
}

String cloudGetPart(String data, int index)
{
  int commaCount = 0;
  int startIndex = 0;

  for (int i = 0; i < data.length(); i++)
  {
    if (data.charAt(i) == ',')
    {
      if (commaCount == index)
      {
        return data.substring(startIndex, i);
      }

      commaCount++;
      startIndex = i + 1;
    }
  }

  if (commaCount == index)
  {
    return data.substring(startIndex);
  }

  return "";
}

void printCloudSchedule()
{
  Serial.println("CLOUD_SCHEDULE_VALUES");

  Serial.print("MORNING = ");
  printSerialTwoDigits(cloudMorningHour);
  Serial.print(":");
  printSerialTwoDigits(cloudMorningMinute);
  Serial.print(", doseCount=");
  Serial.print(cloudMorningDoseCount);
  Serial.print(", enabled=");
  Serial.println(cloudMorningEnabled ? "true" : "false");

  Serial.print("AFTERNOON = ");
  printSerialTwoDigits(cloudAfternoonHour);
  Serial.print(":");
  printSerialTwoDigits(cloudAfternoonMinute);
  Serial.print(", doseCount=");
  Serial.print(cloudAfternoonDoseCount);
  Serial.print(", enabled=");
  Serial.println(cloudAfternoonEnabled ? "true" : "false");

  Serial.print("NIGHT = ");
  printSerialTwoDigits(cloudNightHour);
  Serial.print(":");
  printSerialTwoDigits(cloudNightMinute);
  Serial.print(", doseCount=");
  Serial.print(cloudNightDoseCount);
  Serial.print(", enabled=");
  Serial.println(cloudNightEnabled ? "true" : "false");

  Serial.println("--------------------------------");
}

void printInventory()
{
  Serial.println("INVENTORY_VALUES");

  Serial.print("MORNING_STOCK = ");
  Serial.println(morningStock);

  Serial.print("AFTERNOON_STOCK = ");
  Serial.println(afternoonStock);

  Serial.print("NIGHT_STOCK = ");
  Serial.println(nightStock);

  Serial.println("--------------------------------");
}

void handleCloudSetCommand(String command)
{
  String commandType = cloudGetPart(command, 0);
  String doseType = cloudGetPart(command, 1);
  int hour = cloudGetPart(command, 2).toInt();
  int minute = cloudGetPart(command, 3).toInt();
  int doseCount = cloudGetPart(command, 4).toInt();
  bool enabled = cloudGetPart(command, 5).toInt() == 1;

  if (commandType != "SET")
  {
    Serial.println("INVALID_COMMAND_TYPE");
    return;
  }

  if (doseType == "MORNING")
  {
    cloudMorningHour = hour;
    cloudMorningMinute = minute;
    cloudMorningDoseCount = doseCount;
    cloudMorningEnabled = enabled;
    morningHandled = false;
    Serial.println("UPDATED_CLOUD_MORNING");
  }
  else if (doseType == "AFTERNOON")
  {
    cloudAfternoonHour = hour;
    cloudAfternoonMinute = minute;
    cloudAfternoonDoseCount = doseCount;
    cloudAfternoonEnabled = enabled;
    afternoonHandled = false;
    Serial.println("UPDATED_CLOUD_AFTERNOON");
  }
  else if (doseType == "NIGHT")
  {
    cloudNightHour = hour;
    cloudNightMinute = minute;
    cloudNightDoseCount = doseCount;
    cloudNightEnabled = enabled;
    nightHandled = false;
    Serial.println("UPDATED_CLOUD_NIGHT");
  }
  else
  {
    Serial.println("INVALID_DOSE_TYPE");
    return;
  }

  printCloudSchedule();
  Serial.println("VALID_SET_COMMAND");
}

void handleStockSetCommand(String command)
{
  String commandType = cloudGetPart(command, 0);
  String doseType = cloudGetPart(command, 1);
  int stockValue = cloudGetPart(command, 2).toInt();

  if (commandType != "STOCK")
  {
    Serial.println("INVALID_STOCK_COMMAND");
    return;
  }

  if (stockValue < 0)
  {
    stockValue = 0;
  }

  if (doseType == "MORNING")
  {
    morningStock = stockValue;
    morningHandled = false;
    Serial.println("UPDATED_MORNING_STOCK");
  }
  else if (doseType == "AFTERNOON")
  {
    afternoonStock = stockValue;
    afternoonHandled = false;
    Serial.println("UPDATED_AFTERNOON_STOCK");
  }
  else if (doseType == "NIGHT")
  {
    nightStock = stockValue;
    nightHandled = false;
    Serial.println("UPDATED_NIGHT_STOCK");
  }
  else
  {
    Serial.println("INVALID_STOCK_TYPE");
    return;
  }

  printInventory();
  Serial.println("VALID_STOCK_COMMAND");
}

void handleCloudSerial()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n')
    {
      cloudIncomingLine.trim();

      if (cloudIncomingLine.length() > 0)
      {
        Serial.print("ARDUINO_RECEIVED: ");
        Serial.println(cloudIncomingLine);

        if (cloudIncomingLine.startsWith("SET,"))
        {
          handleCloudSetCommand(cloudIncomingLine);
        }
        else if (cloudIncomingLine.startsWith("STOCK,"))
        {
          handleStockSetCommand(cloudIncomingLine);
        }
        else
        {
          Serial.println("UNKNOWN_CLOUD_COMMAND");
        }
      }

      cloudIncomingLine = "";
    }
    else
    {
      cloudIncomingLine += c;
    }
  }
}