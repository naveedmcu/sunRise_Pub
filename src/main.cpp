#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS3232RTC.h>
#include <sunMoon.h>
#include "ArduinoOTA.h"

#define OUR_latitude 31.4331     // Pakistan LONG
#define OUR_longtitude 74.182152 //Pakistan LATI
#define OUR_timezone 300         // localtime with UTC difference in minutes +5*60
/**/
// #define demo
/**/
const byte DNS_PORT = 53;
IPAddress apIP(8, 8, 4, 4); // The default android DNS
DNSServer dnsServer;
WiFiServer server(80);
/**/
sunMoon sm;
DS3232RTC RTC;
/**/
void init_OTA();
void OTA_Loop(void *pvParametrs);
/**/
void printDate(time_t date)
{
  char buff[20];
  sprintf(buff, "%2d-%02d-%4d %02d:%02d:%02d",
          day(date), month(date), year(date), hour(date), minute(date), second(date));
  Serial.print(buff);
}

void setup()
{
  /**/
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  /**/
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
  breakTime(CusRise, tm);
#ifdef demo
  RTC.setAlarm(ALM1_MATCH_SECONDS, 5, 0, 0, 1);
#else
  RTC.setAlarm(ALM1_MATCH_HOURS, 0, tm.Minute, tm.Hour, 1);
#endif
  Serial.print("H:");
  Serial.println(tm.Hour);
  Serial.print("M:");
  Serial.println(tm.Minute);
  /*-----------------------------------------------------------------------------*/
  WiFi.softAP("Motor AP");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println(myIP);
  server.begin();
  /*-----------------------------------------------------------------------------*/
  init_OTA();
}
/**/

/**/

void loop()
{
  tmElements_t tm; // specific time
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
      time_t CusRise = sm.sunRise();
      setSyncProvider(RTC.get); // the function to get the time from the RTC
      if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
      else
        Serial.println("RTC has set the system time");
      breakTime(CusRise, tm);
#ifdef demo
      RTC.setAlarm(ALM1_MATCH_SECONDS, 5, 0, 0, 1);
#else
      RTC.setAlarm(ALM1_MATCH_HOURS, 0, tm.Minute, tm.Hour, 1);
#endif
      Serial.print("H:");
      Serial.println(tm.Hour);
      Serial.print("M:");
      Serial.println(tm.Minute);
    }
    if (RTC.alarm(ALARM_2)) // check alarm flag, clear it if set
    {
      Serial.println(" ALARM_2 ");
      printDate(RTC.get());
    }
    /**/
    if (digitalRead(2) == 1)
    {
      static int16_t counter = 0;
      counter++;
      if (counter == 10)
      {
        counter = 0;
        digitalWrite(2, LOW);
      }
    }
    /**/
  }
}
/*------------------------------------------------------------------------*/
void OTA_Loop(void *pvParametrs)
{
  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(25));
    {
      ArduinoOTA.handle();
    }
  }
}
/**/
void init_OTA()
{
  // Port defaults to 3232
  ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]

  ArduinoOTA.setHostname("motor"); //motor.local

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type);
               })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
                 Serial.printf("Error[%u]: ", error);
                 if (error == OTA_AUTH_ERROR)
                   Serial.println("Auth Failed");
                 else if (error == OTA_BEGIN_ERROR)
                   Serial.println("Begin Failed");
                 else if (error == OTA_CONNECT_ERROR)
                   Serial.println("Connect Failed");
                 else if (error == OTA_RECEIVE_ERROR)
                   Serial.println("Receive Failed");
                 else if (error == OTA_END_ERROR)
                   Serial.println("End Failed");
               });

  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      OTA_Loop,
      "OTA Loop", 8096, NULL, configMAX_PRIORITIES - 5, NULL, 1);
  if (xReturned != pdPASS)
  {
    Serial.println("[Error]vTask OTA_Loop");
  }
}