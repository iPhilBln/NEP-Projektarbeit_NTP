/*  
*   ESP32 NTP Server <-> Meinberg GPS161AHSx-HQ over RS232
*   Author: Adrian Strehlau
*   Last edit: 16.07.2023
*/

//select which display (if any) and if wifi or ethernet (WIP!) should be used
//#define UC121902_ENABLED //SAMSUNG UC121902-TNARX-A (WIP!)
#define AV1623_ENABLED //ANAG VISION AV1623YFTY-SJW (might/will work with other 16x2 LCD diplays)
#define WIFI_ENABLED

#define UTC 0
#define MEZ 1
int weekDay = 0, isSummertime = 0, addHour = 0, addDay = 0;
String SweekDay = "", hourSpacer = "", minuteSpacer = "", secondSpacer = "", daySpacer = "", monthSpacer = "";
enum
{
  ERRORDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  SUNDAY
};

#ifdef UC121902_ENABLED //WIP!
  #include <UC121902-TNARX-A.h>
  const int CEPin = 14, CKPin = 12, DIPin = 13;
  const int IPPin = 33;
  bool displaysectoggle = false;
  String displayTime = "888888888888", displayIP = "888888888888";
  long millisOnUpdate = 0;
  int displaySecond = 0, displayMinute = 0;
#endif

#ifdef AV1623_ENABLED
  #include <LiquidCrystal.h>
  const int RSPin = 13, ENPin = 12, D4PIN = 14, D5PIN = 27, D6PIN = 26, D7PIN = 25; //4 bit mode
  const int IPPin = 33;
  LiquidCrystal lcd(RSPin, ENPin, D4PIN, D5PIN, D6PIN, D7PIN);
  bool displaysectoggle = false;
  String displayTime = "888888888888";
  long millisOnUpdate = 0;
  int displaySecond = 0, displayMinute = 0;
#endif

#ifdef WIFI_ENABLED
  #include <WiFi.h>
  #include "Secrets.h"
  #define WIFI_NETWORK MY_WIFI_NETWORK
  #define WIFI_PASSWORD MY_WIFI_PASSWORD
  #define WIFI_TIMEOUT_MS 20000
  bool wifiConnected = false;
#else //WIP FIXME (Ethernet needs to be properly set up, e.g. W5500 configuration)
  #include <ETH.h>
  bool ethConnected = false;
  bool ethGotIP = false;
  String ip = "";
#endif

struct tm mb;

const int RXPin = 3, TXPin = 1;
const int RXPin2 = 22, TXPin2 = 23;

//#include <Timezone.h>
//#include <TimeLib.h>
#include <ESP32Time.h>

#define NTP_PORT 123                  //Port 123 is the usual port for NTP Servers
#define NTP_PACKET_SIZE 48            //packet size for NTP data
byte packetBuffer[NTP_PACKET_SIZE];   //Array of bytes the size of the packet size where NTP reply data gets stored
WiFiUDP Udp;

const int PPSPin = 4; // PPS pin on the ESP32 board

//RS232 <-> Meinberg
const int maxAttempts = 10;
const uint32_t baud = 19200;  // PC baud
const uint32_t baud2 = 19200;  // Meinberg baud

ESP32Time rtc(0);

volatile bool ppsFlag;  // pulse per second flag from Meinberg

const long oneSecond_inMicroseconds_L = 1000000;                                  // one second in microseconds (signed long)

const unsigned long periodicTimeRefreshPeriod = 10 * 60 * 1000;                   // 10 minutes in millisenconds aka how often the system's real time clock is refreshed with GPS data
const time_t safeguardThresholdInSeconds = 3;                                     // used to ensure a GPS time refresh is only performed if the difference between the old and new times is this many seconds or less
volatile bool SafeGuardTripped = false;                                           // used to ensure the time isn't changed beyond that which would reasonably be expected within the periodicTimeRefreshPeriod

volatile bool isTimeBeingSet;                                                     // signifies when the time is being set / refreshed

SemaphoreHandle_t mutex;                                                          // used to ensure an NTP request results are not impacted by the process that refreshes the time

TaskHandle_t taskHandle0 = NULL;                                                  // task handle for updating the display
TaskHandle_t taskHandle1 = NULL;                                                  // task handle for refreshing the time 

void getTime(void *parameter)
{
  Serial.println("Getting time");
  static bool firstTime = true;

  /// used below to ensure a GPS time refresh if is only performed if the difference between the old and new times is reasonable for the periodicTimeRefreshPeriod
  const time_t safeguardThresholdHigh = safeguardThresholdInSeconds;
  const time_t safeguardThresholdLow = -1 * safeguardThresholdInSeconds;

  time_t candidateTime;
  char buffer[32];
  #define STX 0x02
  #define ETX 0x03

  while (true)
  {
    isTimeBeingSet = true;

    // wait for the ppsFlag to be raised at the start of the 1st second
    ppsFlag = false;
    while (!ppsFlag)
      ;
    Serial2.flush();
    Serial2.print("?");
    int attempts = 0;
    while (Serial2.available() < 32 && attempts >= maxAttempts)
    {
      attempts++;
      delay(50);
    }
    Serial2.readBytesUntil(ETX, buffer, 32);
    if (buffer[0] != STX)
    {
      Serial.println("received invalid serial message");
      Serial.println(buffer);
    }
    else
    {
      //struct tm mb;
        mb.tm_year = 2000 + (buffer[9] - '0') * 10 + buffer[10] - '0';
        mb.tm_mon = (buffer[6] - '0') * 10 + buffer[7] - '0';
        mb.tm_mday = (buffer[3] - '0') * 10 + buffer[4] - '0';
        mb.tm_hour = (buffer[18] - '0') * 10 + buffer[19] - '0';
        mb.tm_min = (buffer[21] - '0') * 10 + buffer[22] - '0';
        mb.tm_sec = (buffer[24] - '0') * 10 + buffer[25] - '0';

      if ((mb.tm_year > 2022) && (mb.tm_mon > 0) && (mb.tm_mon < 13) && (mb.tm_mday > 0) && (mb.tm_mday < 32) && (mb.tm_hour < 24) && (mb.tm_min < 60) && (mb.tm_sec < 61)) // make sure the values are within reason
      {
        Serial.println("time received ok");
        Serial.println(buffer+1);
        // set candidate time according to Meinberg (will be effective when the PPS flag is triggered again)
        isSummertime = summertime_EU((int)mb.tm_year, (byte)mb.tm_mon, (byte)mb.tm_mday, (byte)mb.tm_hour, (byte)MEZ);
        weekDay = (buffer[14] - '0');
        mb.tm_year -= 1900;                     // adjust year (see you again in 2036)
        mb.tm_mon -= 1;                         // adjust month (January is month 0)
        candidateTime = mktime(&mb) + 1; // +1 important
        time_t mb = candidateTime;
        time_t candidateTime_t = time(&mb);
        // give some time to ensure the PPS pin is reset
        vTaskDelay(200 / portTICK_PERIOD_MS);

        ppsFlag = false;
        while (!ppsFlag)
          ;

        unsigned long pegProcessingAdjustmentStartTime = micros();

        // at this point:
        // apply a sanity check; the current rtc time and the candidate time just taken from the gps readings which will be used to refresh the current rtc should be within a second of each other (safeguardThresholdInSeconds)
        // if the sanity check fails, do not set the time and raise a Safeguard flag which be used to update the display to show the user the latest time refresh failed
        // if the sanity check passes, proceed with refreshing the time and if the Safeguard flag been previously been raised then lower it

        bool SanityCheckPassed;
        time_t updateDelta;

        if (firstTime)
        {
          SanityCheckPassed = true;
        }
        else
        {
          time_t currentRTC_t = rtc.getEpoch();
          time_t currentRTCDateAndTime_t = time(&currentRTC_t);
          updateDelta = currentRTCDateAndTime_t - candidateTime_t;
          bool SanityCheckPassed = (((updateDelta >= safeguardThresholdLow) && (updateDelta <= safeguardThresholdHigh)));
        }

        if (SanityCheckPassed)
        {
          // place a hold on (the date and time) so if an NTP request is underway in the fraction of a second this code will take, the time and date values don't change mid way through that request.
          if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
          {
            // set the date and time
            unsigned long pegProcessingAdjustmentEndTime = micros();
            unsigned long ProcessingAdjustment = pegProcessingAdjustmentEndTime - pegProcessingAdjustmentStartTime;
            // set the real time clock
            rtc.setTime((unsigned long)candidateTime, (int)ProcessingAdjustment);
            // release the hold
            xSemaphoreGive(mutex);

            millisOnUpdate = millis();

            SafeGuardTripped = false;
            isTimeBeingSet = false;
            firstTime = false;
            //Serial.println("itbs false");

            vTaskDelay(periodicTimeRefreshPeriod / portTICK_PERIOD_MS);
          }
          else
          {
            //could not refresh the time as a NTP request was underway and will try again
          }
        }
        else
        {
          //sanity check failed as time delta was too big
          SafeGuardTripped = true;
        }
      }
      else
      {
        Serial.println("timecheck failed");
      }
    }
  }
}

uint64_t getCurrentTimeInNTP64BitFormat()
{

  const uint64_t numberOfSecondsBetween1900and1970 = 2208988800;

  uint64_t clockSecondsSinceEpoch = numberOfSecondsBetween1900and1970 + (uint64_t)rtc.getEpoch();
  long clockMicroSeconds = (long)rtc.getMicros();

  // as one might infer clockMicroSeconds is in microseconds (i.e. 1 second = 1,000,000 microseconds)
  //
  // accordingly, if the clockMicroSeconds is greater than one million ...
  //   for every million that is over:
  //     add 1 (second) to clockSecondsSinceEpoch, and
  //     reduce the clockMicroSeconds by one million (microseconds)
  //
  // likewise ...
  //
  // if the clockMicroSeconds is less than zero:
  //   for every million that is under zero:
  //     subtract (second) from clockSecondsSinceEpoch, and
  //     increase the clockMicroSeconds by one million (microseconds)

  while (clockMicroSeconds > oneSecond_inMicroseconds_L)
  {
    clockSecondsSinceEpoch++;
    clockMicroSeconds -= oneSecond_inMicroseconds_L;
  };

  while (clockMicroSeconds < 0L)
  {
    clockSecondsSinceEpoch--;
    clockMicroSeconds += oneSecond_inMicroseconds_L;
  };

  // for the next two lines to be clear, please see: https://tickelton.gitlab.io/articles/ntp-timestamps/

  double clockMicroSeconds_D = (double)clockMicroSeconds * (double)(4294.967296);
  uint64_t ntpts = ((uint64_t)clockSecondsSinceEpoch << 32) | (uint64_t)(clockMicroSeconds_D);

  return ntpts;
}

// send NTP reply
void sendNTPpacket(IPAddress remoteIP, int remotePort)
{

  // set the receive time to the current time
  uint64_t receiveTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  // Initialize values needed to form NTP request

  // LI: 0, Version: 4, Mode: 4 (server)
  // packetBuffer[0] = 0b00100100;
  // LI: 0, Version: 3, Mode: 4 (server)
  packetBuffer[0] = 0b00011100;

  // Stratum, or type of clock
  packetBuffer[1] = 0b00000001;

  // Polling Interval
  packetBuffer[2] = 4;

  // Peer Clock Precision
  // log2(sec)
  // 0xF6 <--> -10 <--> 0.0009765625 s
  // 0xF7 <--> -9 <--> 0.001953125 s
  // 0xF8 <--> -8 <--> 0.00390625 s
  // 0xF9 <--> -7 <--> 0.0078125 s
  // 0xFA <--> -6 <--> 0.0156250 s
  // 0xFB <--> -5 <--> 0.0312500 s
  packetBuffer[3] = 0xF7;

  // 8 bytes for Root Delay & Root Dispersion
  // root delay
  packetBuffer[4] = 0;
  packetBuffer[5] = 0;
  packetBuffer[6] = 0;
  packetBuffer[7] = 0;

  // root dispersion
  packetBuffer[8] = 0;
  packetBuffer[9] = 0;
  packetBuffer[10] = 0;
  packetBuffer[11] = 0x50;

  // time source (namestring)
  packetBuffer[12] = 71; // G
  packetBuffer[13] = 80; // P
  packetBuffer[14] = 83; // S
  packetBuffer[15] = 0;

  // get the current time and write it out as the reference time to bytes 16 to 23 of the response packet
  uint64_t referenceTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  packetBuffer[16] = (int)((referenceTime_uint64_t >> 56) & 0xFF);
  packetBuffer[17] = (int)((referenceTime_uint64_t >> 48) & 0xFF);
  packetBuffer[18] = (int)((referenceTime_uint64_t >> 40) & 0xFF);
  packetBuffer[19] = (int)((referenceTime_uint64_t >> 32) & 0xFF);
  packetBuffer[20] = (int)((referenceTime_uint64_t >> 24) & 0xFF);
  packetBuffer[21] = (int)((referenceTime_uint64_t >> 16) & 0xFF);
  packetBuffer[22] = (int)((referenceTime_uint64_t >> 8) & 0xFF);
  packetBuffer[23] = (int)(referenceTime_uint64_t & 0xFF);

  // copy transmit time from the NTP original request to bytes 24 to 31 of the response packet
  packetBuffer[24] = packetBuffer[40];
  packetBuffer[25] = packetBuffer[41];
  packetBuffer[26] = packetBuffer[42];
  packetBuffer[27] = packetBuffer[43];
  packetBuffer[28] = packetBuffer[44];
  packetBuffer[29] = packetBuffer[45];
  packetBuffer[30] = packetBuffer[46];
  packetBuffer[31] = packetBuffer[47];

  // write out the receive time (it was set above) to bytes 32 to 39 of the response packet
  packetBuffer[32] = (int)((receiveTime_uint64_t >> 56) & 0xFF);
  packetBuffer[33] = (int)((receiveTime_uint64_t >> 48) & 0xFF);
  packetBuffer[34] = (int)((receiveTime_uint64_t >> 40) & 0xFF);
  packetBuffer[35] = (int)((receiveTime_uint64_t >> 32) & 0xFF);
  packetBuffer[36] = (int)((receiveTime_uint64_t >> 24) & 0xFF);
  packetBuffer[37] = (int)((receiveTime_uint64_t >> 16) & 0xFF);
  packetBuffer[38] = (int)((receiveTime_uint64_t >> 8) & 0xFF);
  packetBuffer[39] = (int)(receiveTime_uint64_t & 0xFF);

  // get the current time and write it out as the transmit time to bytes 40 to 47 of the response packet
  uint64_t transmitTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  packetBuffer[40] = (int)((transmitTime_uint64_t >> 56) & 0xFF);
  packetBuffer[41] = (int)((transmitTime_uint64_t >> 48) & 0xFF);
  packetBuffer[42] = (int)((transmitTime_uint64_t >> 40) & 0xFF);
  packetBuffer[43] = (int)((transmitTime_uint64_t >> 32) & 0xFF);
  packetBuffer[44] = (int)((transmitTime_uint64_t >> 24) & 0xFF);
  packetBuffer[45] = (int)((transmitTime_uint64_t >> 16) & 0xFF);
  packetBuffer[46] = (int)((transmitTime_uint64_t >> 8) & 0xFF);
  packetBuffer[47] = (int)(transmitTime_uint64_t & 0xFF);

  // send the reply
  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  Serial.println("Sent NTP time");
}

void processNTPRequests()
{
  unsigned long replyStartTime = micros();

  int packetSize = Udp.parsePacket(); // check incoming packet size

  if (packetSize == NTP_PACKET_SIZE) // an NTP request has arrived
  {
    Serial.println("Got NTP request");
    // store sender ip for later use
    IPAddress remoteIP = Udp.remoteIP();

    // read the data from the packet into the buffer for later use
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // hold here if and while the date and time are being refreshed
    // when ok to proceed place a hold on using the mutex to prevent the date and time from being refreshed while the reply packet is being built
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
      // send NTP reply
      sendNTPpacket(remoteIP, Udp.remotePort());
      xSemaphoreGive(mutex);
    };
  }
  else
  {
    if (packetSize > 0)
    {
      Udp.flush(); //not an ntp request so discarded
    };
  };
}

void ppsHandlerRising() // PPS interrupt handler
{                 
  ppsFlag = true; // raise the flag that signals the start of the next second
  #if defined(UC121902_ENABLED) || defined(AV1623_ENABLED)
    displaysectoggle = true;
  #endif
}

/*
#ifndef WIFI_ENABLED
  void EthEvent(WiFiEvent_t event)
  {
    switch (event)
    {
    case SYSTEM_EVENT_ETH_START:
      ETH.setHostname("MasterClock");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      ethConnected = true;
      break;
    case SYSTEM_EVENT_ethGotIP:
      ip = ETH.localIP().toString();
      ethGotIP = true;
      ethConnected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      ethConnected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      ethConnected = false;
      break;
    default:
      break;
    }
  }
#endif
*/

#if defined(UC121902_ENABLED) || defined(AV1623_ENABLED)
  void updateDisplay(void *parameter)
  {
    while(true)
    {
      #ifdef UC121902_ENABLED
        if(displaysectoggle == true)
        {
          display.sec.toggle();
          displaysectoggle = false;
        }
        if(!digitalRead(IPPin))
        {
          displayIP = WiFi.localIP().toString();
          displayIP.remove(11, 1);
          displayIP.remove(7, 1);
          displayIP.remove(3, 1);
          display.print(displayIP);
        }
        else
        {
          displaySecond = (mb.tm_sec + ((millis() - millisOnUpdate) / 1000));
          while(displaySecond >= 60)
          {
            displaySecond -= 60;
          }
          displayTime = "    " + (String) mb.tm_hour + ":" + (String) mb.tm_minute + ":" + (String) displaySecond;
          display.print(displayTime);
          vTaskDelay(400 / portTICK_PERIOD_MS);
        }
      #endif
      #ifdef AV1623_ENABLED
      lcd.clear();
        if(displaysectoggle == true)
        {
          lcd.setCursor(15,0);
          lcd.write(255);
          displaysectoggle = false;
        }
        else
        {
          lcd.setCursor(15,0);
          lcd.write(32);
        }
        if(!digitalRead(IPPin))
        {
          lcd.home();
          lcd.print(WiFi.localIP().toString());
          vTaskDelay(200 / portTICK_PERIOD_MS);
        }
        else
        {
          lcd.home();
          displaySecond = (mb.tm_sec + ((millis() - millisOnUpdate) / 1000));
          displayMinute = mb.tm_min;
          while(displaySecond >= 60)
          {
            displaySecond -= 60;
            displayMinute++;
          }
          if(displayMinute >= 60)
          {
            displayMinute -= 60;
            addHour = 1;
          }
          else
          {
            addHour = 0;
          }
          if((mb.tm_hour + addHour + 1 + isSummertime) >= 24)
          {
            addHour -= 24;
            addDay = 1;
            if((weekDay + addDay) == 8)
            {
              weekDay -= 7;
            }
          }
          else
          {
            addDay = 0;
          }
          if((mb.tm_hour + addHour + 1 + isSummertime) < 10)
          {
            hourSpacer = "0";
          }
          else
          {
            hourSpacer = "";
          }
          if(displayMinute < 10)
          {
            minuteSpacer = "0";
          }
          else
          {
            minuteSpacer = "";
          }
          if(displaySecond < 10)
          {
            secondSpacer = "0";
          }
          else
          {
            secondSpacer = "";
          }
          if((mb.tm_mday + addDay) < 10)
          {
            daySpacer = "0";
          }
          else
          {
            daySpacer = "";
          }
          if((mb.tm_mon + 1) < 10)
          {
            monthSpacer = "0";
          }
          else
          {
            monthSpacer = "";
          }
          displayTime = hourSpacer + (String) (mb.tm_hour + addHour + 1 + isSummertime) + ":" + minuteSpacer + (String) displayMinute + ":" + secondSpacer + (String) displaySecond;
          lcd.print(displayTime);
          displayTime = daySpacer + (String) (mb.tm_mday + addDay) + "." + monthSpacer + (String) (mb.tm_mon + 1) + "." + (String) (1900 + mb.tm_year);
          lcd.setCursor(0, 1);
          lcd.print(displayTime);
          lcd.setCursor(11, 1);
          switch(weekDay)
          {
            case MONDAY:
              lcd.print("Mon");
              break;
            case TUESDAY:
              lcd.print("Tue");
              break;
            case WEDNESDAY:
              lcd.print("Wed");
              break;
            case THURSDAY:
              lcd.print("Thu");
              break;
            case FRIDAY:
              lcd.print("Fri");
              break;
            case SATURDAY:
              lcd.print("Sat");
              break;
            case SUNDAY:
              lcd.print("Sun");
              break;
            default:
              lcd.print("Err");
              break;
          }
          vTaskDelay(200 / portTICK_PERIOD_MS);
        }
      #endif
    }
  }
#endif

boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{ 
  if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7))) 
    return true; 
  else 
    return false;
}

void setup()
{
  Serial.begin(baud, SERIAL_8N1, RXPin, TXPin);
  Serial2.begin(baud2, SERIAL_8N1, RXPin2, TXPin2);
  delay(100);
  Serial.println("startup");
  //
  #ifdef UC121902_ENABLED
    display.begin();
    display.print("StARtUP");
  #endif
  #ifdef AV1623_ENABLED
    lcd.begin(16, 2);
    lcd.print("STARTUP");
  #endif
  // mutex ensures NTP request results are not affected by time refreshes
   mutex = xSemaphoreCreateMutex();
  
  // pulse-per-second pin setup
  pinMode(PPSPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PPSPin), ppsHandlerRising, RISING);

  #if defined(UC121902_ENABLED) || defined(AV1623_ENABLED)
    pinMode(IPPin, INPUT_PULLUP);
  #endif
  

  xTaskCreatePinnedToCore
  (
      getTime,
      "Get Time Over RS232",
      3000,
      NULL,
      20,
      &taskHandle1,
      1
  );

  #if defined(UC121902_ENABLED) || defined(AV1623_ENABLED)
    xTaskCreatePinnedToCore
    (
        updateDisplay,
        "Update the display",
        3000,
        NULL,
        10,
        &taskHandle0,
        0
    );
  #endif
  
  while(isTimeBeingSet)
    delay(10);
  
  #ifndef WIFI_ENABLED
    WiFi.onEvent(EthEvent);
    WiFi.begin();
    while (!ethGotIP)
      delay(1);
  #endif

  #ifdef WIFI_ENABLED
    Serial.println("wifi start");
    while(!wifiConnected)
    {
      WiFi.mode(WIFI_STA);
      WiFi.setHostname("MasterClock");
      WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
      unsigned long startAttemptTime = millis();
      while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
        delay(50);
      if(WiFi.status() == WL_CONNECTED)
      {
        wifiConnected = true;
        Serial.println("wifi success");
        Serial.print("IP:");
        Serial.println(WiFi.localIP().toString());
      }
      else
      {
        Serial.println("wifi failure");
      }
    }
  #endif
  Udp.begin(NTP_PORT);
}

void loop()
{
  processNTPRequests();
}
