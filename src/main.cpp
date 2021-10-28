#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS3232RTC.h>
#include <sunMoon.h>

#define OUR_latitude 31.4331     // Pakistan LONG
#define OUR_longtitude 74.182152 //Pakistan LATI
#define OUR_timezone 300         // localtime with UTC difference in minutes +5*60

sunMoon sm;
DS3232RTC RTC;

void printDate(time_t date)
{
  char buff[20];
  sprintf(buff, "%2d-%02d-%4d %02d:%02d:%02d",
          day(date), month(date), year(date), hour(date), minute(date), second(date));
  Serial.print(buff);
}

void setup()
{
  tmElements_t tm; // specific time

  tm.Second = 0;
  tm.Minute = 12;
  tm.Hour = 12;
  tm.Day = 3;
  tm.Month = 8;
  tm.Year = 2016 - 1970;
  time_t s_date = makeTime(tm);

  Serial.begin(115200);
  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  sm.init(OUR_timezone, OUR_latitude, OUR_longtitude);

  Serial.print("Today is ");
  printDate(RTC.get());
  Serial.println("");

  uint32_t jDay = sm.julianDay(); // Optional call
  byte mDay = sm.moonDay();
  time_t CusRise = sm.sunRise();
  time_t CusSet = sm.sunSet();
  Serial.print("Today is ");
  Serial.print(jDay);
  Serial.println(" Julian day");
  Serial.print("Moon age is ");
  Serial.print(mDay);
  Serial.println("day(s)");
  Serial.print("Today sunrise and sunset: ");
  printDate(CusRise);
  Serial.print("; ");
  printDate(CusSet);
  Serial.println("");

  Serial.print("Specific date was ");
  printDate(s_date);
  Serial.println("");
  jDay = sm.julianDay(s_date);
  mDay = sm.moonDay(s_date);
  time_t sRise = sm.sunRise(s_date);
  time_t sSet = sm.sunSet(s_date);
  Serial.print("Specific date sunrise and sunset was: ");
  Serial.print("Julian day of specific date was ");
  Serial.println(jDay);
  Serial.print("Moon age was ");
  Serial.print(mDay);
  Serial.println("day(s)");
  printDate(sRise);
  Serial.print("; ");
  printDate(sSet);
  Serial.println("");
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_1_HZ);
  // RTC.setAlarm(ALM1_MATCH_SECONDS, 5, 0, 0, 1);
  breakTime(CusRise, tm);
  RTC.setAlarm(ALM1_MATCH_HOURS, 0, tm.Minute, tm.Hour, 1);
  Serial.print("H:");
  Serial.println(tm.Hour);
  Serial.print("M:");
  Serial.println(tm.Minute);
}

void loop()
{
  while (1)
  {
    delay(1000);
    setSyncProvider(RTC.get); // the function to get the time from the RTC
    if (timeStatus() != timeSet)
      Serial.println("Unable to sync with the RTC");
    else
      Serial.println("RTC has set the system time");
    sm.init(OUR_timezone, OUR_latitude, OUR_longtitude);

    Serial.print("Loop Today is:");
    printDate(RTC.get());
    Serial.println("");
    // put your main code here, to run repeatedly:
    if (RTC.alarm(ALARM_1)) // check alarm flag, clear it if set
    {
      Serial.println(" ALARM_1 ");
      printDate(RTC.get());
      pinMode(2, OUTPUT);
      digitalWrite(2, HIGH);
    }
    if (RTC.alarm(ALARM_2)) // check alarm flag, clear it if set
    {
      Serial.println(" ALARM_2 ");
      printDate(RTC.get());
    }
  }
}