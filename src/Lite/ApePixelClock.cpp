#include "ApePixelClock.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "ArduinoJson.h"
#include <vector>

WiFiClient wifiClient;
HTTPClient httpClient;
RTC_DS1307 *m_rtc;

std::vector<ApcEffectDef *> apcEffects;
std::vector<ApcScheduleCallbackDef *> apcScheduleCallbacks;

int apcEffectPointer;
unsigned long preCheckTime = 0;
bool autoChange = true;

byte payload[2048];
unsigned int payLoadPointer = 0;

void apcEffect_TimeShow_callback(unsigned int areaCount, unsigned int frameCount)
{
  DateTime now = m_rtc->now();
  if (frameCount < 5)
  {
    APC.plBegin().plPid(0).plCoord(7).plCoord(1).plColor(155, 155, 155);
    char timeChar[6];
    if (frameCount % 2 == 0)
    {
      sprintf(timeChar, "%02d:%02d", now.hour(), now.minute());
    }
    else
    {
      sprintf(timeChar, "%02d %02d", now.hour(), now.minute());
    }
    APC.plStr(String(timeChar)).plCallback();
  }
  else
  {
    APC.plBegin().plPid(0).plCoord(6).plCoord(1).plColor(155, 155, 155);
    char timeChar[6];
    sprintf(timeChar, "%02d/%02d", now.month(), now.day());
    APC.plStr(String(timeChar)).plCallback();
  }
  int w = now.dayOfTheWeek() - 1;
  if (w < 0)
    w = 6;
  for (int i = 0; i < 7; i++)
  {
    APC.plBegin().plPid(6).plCoord(3 + 4 * i).plCoord(7).plCoord(3 + 4 * i + 2).plCoord(7);
    if (w == i)
    {
      APC.plColor(155, 155, 155);
    }
    else
    {
      APC.plColor(55, 55, 55);
    }
    APC.plCallback();
  }
}

void ApePixelClock::addApcEffects()
{
  ApcEffectDef *apcEffect_TimeShow = new ApcEffectDef;
  memset(apcEffect_TimeShow, 0, sizeof(ApcEffectDef));
  apcEffect_TimeShow->areaDef[0] = {0, 0, 32, 8, 10, 1000};
  apcEffect_TimeShow->callbackFunc = apcEffect_TimeShow_callback;
  apcEffect_TimeShow->autoChangeTime = 10000;
  this->addApcEffect(apcEffect_TimeShow);
}

void ApePixelClock::addApcEffect(ApcEffectDef *apcEffect)
{
  int apcEffectAreaCount = 0;
  for (int i = 0; i < MAX_APCEffECTAREA_COUNT; i++)
  {
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    if (apcEffectArea.frameCount > 0 && apcEffectArea.frameRefreshTime > 0)
    {
      apcEffectArea.currentFrameCount = 0;
      apcEffectArea.currentRefreshTime = 0;
      apcEffectAreaCount++;
    }
  }
  apcEffect->areaCount = apcEffectAreaCount;
  apcEffect->currentAreaIndex = 0;
  apcEffect->currentChangeTime = apcEffect->autoChangeTime;
  apcEffects.push_back(apcEffect);
}

ApePixelClock::ApePixelClock()
{
}

void ApePixelClock::apcLoop()
{
  this->renderCheck();
}

void ApePixelClock::renderCheck()
{
  if (apcEffectPointer >= int(apcEffects.size()))
    return;
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];
  int areaCount = apcEffect->areaCount;
  for (int i = 0; i < areaCount; i++)
  {
    apcEffect->currentAreaIndex = i;
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    if (millis() - preCheckTime > 0)
    {
      apcEffectArea.currentRefreshTime -= (millis() - preCheckTime);
    }
    if (apcEffectArea.currentRefreshTime <= 0)
    {
      this->renderAction(apcEffect);
      apcEffectArea.currentFrameCount++;
      if (apcEffectArea.currentFrameCount >= apcEffectArea.frameCount)
      {
        apcEffectArea.currentFrameCount = 0;
      }
      apcEffectArea.currentRefreshTime = apcEffectArea.frameRefreshTime;
    }
  }
  // if (autoChange)
  // {
  //   if (millis() - preCheckTime > 0)
  //   {
  //     apcEffect->currentChangeTime -= (millis() - preCheckTime);
  //   }
  //   if (apcEffect->currentChangeTime <= 0)
  //   {
  //     this->apcEffectChangeAction();
  //     matrix.fillScreen(matrix.Color(0, 0, 0));
  //   }
  // }
  preCheckTime = millis();
  this->show();
}

void ApePixelClock::show()
{
  byte payload[1] = {8};
  this->callback(nullptr, payload, 1);
}
void ApePixelClock::clear()
{
  byte payload[1] = {9};
  this->callback(nullptr, payload, 1);
}

void ApePixelClock::renderAction(ApcEffectDef *apcEffect, bool needArea)
{
  ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[apcEffect->currentAreaIndex];
  // matrix.offsetX = apcEffectArea.x;
  // matrix.offsetY = apcEffectArea.y;
  // matrix.areaWidth = apcEffectArea.width;
  // matrix.areaHeight = apcEffectArea.height;
  this->clear();
  apcEffect->callbackFunc(apcEffect->currentAreaIndex, apcEffectArea.currentFrameCount);
}

void ApePixelClock::publish(String &s)
{
  Serial.println(s);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(s);

  String infoType = json["type"];
  if (infoType == "button")
  {
    Serial.println("button Action");
  }
}

void ApePixelClock::systemInit(MQTT_CALLBACK_SIGNATURE, RTC_DS1307 *rtc)
{
  this->callback = callback;
  m_rtc = rtc;
  this->upgradeTime();
  this->addApcEffects();
}
void ApePixelClock::upgradeTime()
{
  if (this->internetConnected())
  {
    int errCode = 0;
    uint32_t secondStamp = this->requestSecondStamp(&errCode);
    if (errCode == 0)
    {
      m_rtc->adjust(DateTime(secondStamp));
      Serial.println("Update succeed！！！");
    }
  }
  else
  {
    Serial.println("Internet is not connected");
  }
}
bool ApePixelClock::internetConnected()
{
  return WiFi.isConnected();
}

uint32_t ApePixelClock::requestSecondStamp(int *errCode)
{
  String url = "http://worldtimeapi.org/api/ip";
  String res = this->httpRequest(url, errCode);
  if (*errCode == 0)
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(res);
    uint32_t secondStamp = json["unixtime"].as<String>().substring(0, 10).toInt();
    int timeZone = json["utc_offset"].as<String>().substring(0, 3).toInt();
    return secondStamp + timeZone * 60 * 60;
  }
  return 0;
}

String ApePixelClock::httpRequest(const String &url, int *errCode)
{
  String payload;
  Serial.print("[HTTP] begin...\n");
  if (httpClient.begin(wifiClient, url))
  { // HTTP
    Serial.print("[HTTP] GET...\n");
    int httpCode = httpClient.GET();
    *errCode = httpCode;
    if (httpCode > 0)
    {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        *errCode = 0;
        payload = httpClient.getString();
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }
    httpClient.end();
  }
  else
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  return payload;
}

ApePixelClock &ApePixelClock::plBegin()
{
  payLoadPointer = 0;
  return APC;
}
ApePixelClock &ApePixelClock::plPid(byte pid)
{
  payload[payLoadPointer++] = pid;
  return APC;
}
ApePixelClock &ApePixelClock::plCoord(uint16_t c)
{
  payload[payLoadPointer++] = (c >> 8) & 0xFF;
  payload[payLoadPointer++] = (c)&0xFF;
  return APC;
}
ApePixelClock &ApePixelClock::plColor(byte r, byte g, byte b)
{
  payload[payLoadPointer++] = r;
  payload[payLoadPointer++] = g;
  payload[payLoadPointer++] = b;
  return APC;
}
ApePixelClock &ApePixelClock::plStr(const String &str)
{
  int length = str.length();
  for (int i = 0; i < length; i++)
  {
    payload[payLoadPointer++] = str[i];
  }
  return APC;
}

void ApePixelClock::plCallback()
{
  APC.callback(nullptr, payload, payLoadPointer);
}

ApePixelClock APC;