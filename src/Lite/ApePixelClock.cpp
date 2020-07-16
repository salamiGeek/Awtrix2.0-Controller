#include "ApePixelClock.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <vector>

RTC_DS1307 *m_rtc;

std::vector<ApcEffectDef *> apcEffects;
std::vector<ApcScheduleCallbackDef *> apcScheduleCallbacks;

int apcEffectPointer;
unsigned long preCheckTime = 0;
unsigned long callbackCheckTime = 0;
bool autoChange = true;

int biliSubscriberCount = 0;

byte payload[1024];
unsigned int payLoadPointer = 0;

void timeShowEffect(unsigned int areaCount, unsigned int frameCount)
{
  DateTime now = m_rtc->now();
  if (frameCount < 5)
  {
    APC.plBegin().plPid(0).plCoord(7, 1).plColor();
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
    APC.plBegin().plPid(0).plCoord(6, 1).plColor();
    char timeChar[6];
    sprintf(timeChar, "%02d/%02d", now.month(), now.day());
    APC.plStr(String(timeChar)).plCallback();
  }
  int w = now.dayOfTheWeek() - 1;
  if (w < 0)
    w = 6;
  for (int i = 0; i < 7; i++)
  {
    APC.plBegin().plPid(6).plCoord(3 + 4 * i, 7).plCoord(3 + 4 * i + 2, 7);
    if (w == i)
    {
      APC.plColor();
    }
    else
    {
      APC.plColor(155, 155, 155);
    }
    APC.plCallback();
  }
}

const uint32 biliColorArr[2] ICACHE_RODATA_ATTR = {0x000000, 0x00A1F1};
const uint32 biliPixels[16] ICACHE_RODATA_ATTR =
    {
        0x00010000, 0x00000100,
        0x00000100, 0x00010000,
        0x00010101, 0x01010100,
        0x01000000, 0x00000001,
        0x01000100, 0x00010001,
        0x01000100, 0x00010001,
        0x01000000, 0x00000001,
        0x00010101, 0x01010100};

void bilibiliEffect(unsigned int areaCount, unsigned int frameCount)
{
  if (areaCount == 0)
  {
    APC.drawColorIndexFrame(biliColorArr, 8, 8, biliPixels);
  }
  else if (areaCount == 1)
  {
    String num = String(biliSubscriberCount);
    APC.plBegin().plPid(0).plCoord((uint16_t)APC.textCenterX(num.length(), 4, 6), 1).plColor().plStr(num).plCallback();
  }
}

const String API = "https://api.bilibili.com/x/relation/stat?vmid=";
void updateBilibiliSubscriberCount(int effectId)
{
  String url = API + String(BILIBILI_UID);
  int errCode = 0;
  const String &res = APC.httpsRequest(url, &errCode);
  if (errCode == 0)
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(res);
    biliSubscriberCount = json["data"]["follower"].as<int>();
  }
}

void ApePixelClock::ramCheck(const char *info)
{
  Serial.printf("%s: S:%d,H:%d,M:%d \n", info, ESP.getFreeContStack(), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());
}

void schedule_callback(int callbackId)
{
  switch (callbackId)
  {
  case 1:

    break;
  case 2:
    updateBilibiliSubscriberCount(callbackId);
    break;
  case 3:
    updateBilibiliSubscriberCount(callbackId);
    break;
  case 4:
    updateBilibiliSubscriberCount(callbackId);
    break;
  default:
    break;
  }
}

void ApePixelClock::addApcEffects()
{
  ApcEffectDef *apcEffect = NULL;

#if (TIME_SHOW == 1)
  apcEffect = new ApcEffectDef();
  memset(apcEffect, 0, sizeof(ApcEffectDef));
  apcEffect->effectId = 1;
  apcEffect->updateTime = 0;
  apcEffect->autoChangeTime = 10000;
  apcEffect->areaDef[0] = {0, 0, 32, 8, 10, 1000};
  this->addApcEffect(apcEffect);
#endif

#if (BILIBILI_SHOW == 1)
  apcEffect = new ApcEffectDef();
  memset(apcEffect, 0, sizeof(ApcEffectDef));
  apcEffect->effectId = 2;
  apcEffect->updateTime = 900000;
  apcEffect->autoChangeTime = 10000;
  apcEffect->areaDef[0] = {0, 0, 8, 8, 1, 0};
  apcEffect->areaDef[1] = {8, 0, 24, 8, 6, 0};
  this->addApcEffect(apcEffect);
#endif
  // apcEffect = new ApcEffectDef();
  // memset(apcEffect, 0, sizeof(ApcEffectDef));
  // apcEffect->effectId = 3;
  // apcEffect->updateTime = 10000;
  // apcEffect->autoChangeTime = 10000;
  // apcEffect->areaDef[0] = {0, 0, 8, 8, 1, 0};
  // apcEffect->areaDef[1] = {8, 0, 24, 8, 6, 1000};
  // this->addApcEffect(apcEffect);
  // Serial.printf("begin3: S:%d,H:%d,M:%d \n", ESP.getFreeContStack(), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());

  // apcEffect = new ApcEffectDef();
  // memset(apcEffect, 0, sizeof(ApcEffectDef));
  // apcEffect->effectId = 4;
  // apcEffect->updateTime = 10000;
  // apcEffect->autoChangeTime = 10000;
  // apcEffect->areaDef[0] = {0, 0, 8, 8, 1, 0};
  // apcEffect->areaDef[1] = {8, 0, 24, 8, 6, 1000};
  // this->addApcEffect(apcEffect);
  // Serial.printf("begin4: S:%d,H:%d,M:%d \n", ESP.getFreeContStack(), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());
}

void ApePixelClock::addApcEffect(ApcEffectDef *apcEffect)
{
  int apcEffectAreaCount = 0;
  for (int i = 0; i < MAX_APCEffECTAREA_COUNT; i++)
  {
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    if (apcEffectArea.frameCount > 0)
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

  if (apcEffect->updateTime > 0)
  {
    this->addApcScheduleCallback(apcEffect->effectId, apcEffect->updateTime);
  }
}

void ApePixelClock::addApcScheduleCallback(int callbackId, unsigned long callbackTime)
{
  ApcScheduleCallbackDef *apcScheduleCallback = new ApcScheduleCallbackDef();
  apcScheduleCallback->callbackId = callbackId;
  apcScheduleCallback->callbackTime = callbackTime;
  apcScheduleCallback->currentRefreshTime = 0;
  apcScheduleCallbacks.push_back(apcScheduleCallback);
}

void apcEffect_callback(int effectId, unsigned int areaCount, unsigned int frameCount)
{
  switch (effectId)
  {
  case 1:
    timeShowEffect(areaCount, frameCount);
    break;
  case 2:
    bilibiliEffect(areaCount, frameCount);
    break;
  case 3:
    bilibiliEffect(areaCount, frameCount);
    break;
  case 4:
    bilibiliEffect(areaCount, frameCount);
    break;
  default:
    break;
  }
}

ApePixelClock::ApePixelClock()
{
}

void ApePixelClock::apcSetup()
{
  this->upgradeTime();
  callbackCheckTime = millis();
  preCheckTime = millis();
  this->effectDisplayInit();
}

void ApePixelClock::apcLoop()
{
  this->renderCheck();
  this->apcCallbackAction();
}

void ApePixelClock::renderCheck()
{
  if (apcEffectPointer >= int(apcEffects.size()))
    return;
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];

  if (autoChange)
  {
    if (apcEffect->autoChangeTime > 0)
    {
      int diff = millis() - preCheckTime;
      if (diff > 0)
      {
        if (apcEffect->currentChangeTime > diff)
        {
          apcEffect->currentChangeTime -= diff;
        }
        else
        {
          this->clear();
          this->apcEffectChangeAction();
        }
      }
    }
  }
  apcEffect = apcEffects[apcEffectPointer];
  int areaCount = apcEffect->areaCount;
  for (int i = 0; i < areaCount; i++)
  {
    apcEffect->currentAreaIndex = i;
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    if (apcEffectArea.frameRefreshTime > 0)
    {
      int diff = millis() - preCheckTime;
      if (diff > 0)
      {
        if (apcEffectArea.currentRefreshTime > diff)
        {
          apcEffectArea.currentRefreshTime -= diff;
        }
        else
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
    }
  }
  preCheckTime = millis();
  this->show();
}

void ApePixelClock::apcCallbackAction()
{
  for (int i = 0; i < (int)apcScheduleCallbacks.size(); i++)
  {
    ApcScheduleCallbackDef *apcScheduleCallback = apcScheduleCallbacks[i];
    if (apcScheduleCallback->callbackTime > 0)
    {
      int diff = millis() - callbackCheckTime;
      if (diff > 0)
      {
        if (apcScheduleCallback->currentRefreshTime > diff)
        {
          apcScheduleCallback->currentRefreshTime -= (millis() - callbackCheckTime);
        }
        else
        {
          schedule_callback(apcScheduleCallback->callbackId);
          apcScheduleCallback->currentRefreshTime = apcScheduleCallback->callbackTime;
        }
      }
    }
  }
  callbackCheckTime = millis();
}

void ApePixelClock::apcEffectChangeAction()
{
  apcEffectPointer++;
  if (apcEffectPointer >= (int)apcEffects.size())
  {
    apcEffectPointer = 0;
  }
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];
  this->apcEffectRefresh(apcEffect);
  this->effectDisplayInit();
}

void ApePixelClock::effectDisplayInit()
{
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];
  int areaCount = apcEffect->areaCount;
  for (int i = 0; i < areaCount; i++)
  {
    apcEffect->currentAreaIndex = i;
    this->renderAction(apcEffect);
    this->show();
  }
}

void ApePixelClock::apcEffectRefresh(ApcEffectDef *apcEffect)
{
  apcEffect->currentChangeTime = apcEffect->autoChangeTime;
  for (int i = 0; i < apcEffect->areaCount; i++)
  {
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    apcEffectArea.currentFrameCount = 0;
    apcEffectArea.currentRefreshTime = 0;
  }
}

void ApePixelClock::show()
{
  APC.plBegin().plPid(8).plCallback();
}
void ApePixelClock::clear()
{
  APC.plBegin().plPid(9).plCallback();
}

void ApePixelClock::areaClear()
{
  APC.plBegin().plPid(23).plCoord(0, 0).plByte(m_areaWidth).plByte(m_areaHeight).plColor(0, 0, 0).plCallback();
}

void ApePixelClock::renderAction(ApcEffectDef *apcEffect, bool needArea)
{
  ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[apcEffect->currentAreaIndex];
  this->m_offsetX = apcEffectArea.x;
  this->m_offsetY = apcEffectArea.y;
  this->m_areaWidth = apcEffectArea.width;
  this->m_areaHeight = apcEffectArea.height;
  this->areaClear();
  apcEffect_callback(apcEffect->effectId, apcEffect->currentAreaIndex, apcEffectArea.currentFrameCount);
}

void ApePixelClock::publish(String &s)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(s);

  String infoType = json["type"].as<String>();
  if (infoType == "button")
  {
    Serial.println("button Action");
  }
}

void ApePixelClock::systemInit(MQTT_CALLBACK_SIGNATURE, RTC_DS1307 *rtc)
{
  this->callback = callback;
  m_rtc = rtc;
  apcEffectPointer = 0;
  this->addApcEffects();
  APC.apcSetup();
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
    }
  }
}
bool ApePixelClock::internetConnected()
{
  return WiFi.isConnected();
}

const String url = "http://worldtimeapi.org/api/ip";
uint32_t ApePixelClock::requestSecondStamp(int *errCode)
{
  const String &res = this->httpRequest(url, errCode);
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
  String res;
  HTTPClient *httpClient = new HTTPClient();
  WiFiClient *wifiClient = new WiFiClient();
  if (httpClient->begin(*wifiClient, url))
  { // HTTP
    int httpCode = httpClient->GET();
    *errCode = httpCode;
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        *errCode = 0;
        res = httpClient->getString();
      }
    }
    httpClient->end();
  }
  delete httpClient;
  delete wifiClient;
  return res;
}

String ApePixelClock::httpsRequest(const String &url, int *errCode)
{
  String urlTemp = url;
  urlTemp.replace("https://", "");
  int splitIndex = urlTemp.indexOf('/');
  const String &httpsServer = urlTemp.substring(0, splitIndex);
  const String &api = urlTemp.substring(splitIndex);
  Serial.print("Connecting to ");
  Serial.print(httpsServer);
  BearSSL::WiFiClientSecure *httpsClient = new BearSSL::WiFiClientSecure();
  httpsClient->setInsecure();
  httpsClient->setTimeout(2000);
  int retries = 6;
  while (!httpsClient->connect(httpsServer, 443) && (retries-- > 0))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  String res;
  if (!httpsClient->connected())
  {
    Serial.println("Failed to connect, going back to sleep");
    *errCode = -1;
    httpsClient->stop();
  }
  else
  {
    Serial.print("Request resource: ");
    httpsClient->print(String("GET ") + api +
                       " HTTP/1.1\r\n" +
                       "Host: " + httpsServer + "\r\n" +
                       "Connection: close\r\n\r\n");
    int timeout = 5 * 10; // 5 seconds
    while (!httpsClient->available() && (timeout-- > 0))
    {
      delay(100);
    }
    if (!httpsClient->available())
    {
      Serial.println("No response, going back to sleep");
      *errCode = -2;
      httpsClient->stop();
    }
    else
    {
      while (httpsClient->available())
      {
        res += char(httpsClient->read());
      }
      res = res.substring(res.indexOf('{'));
      Serial.println("\nclosing connection");
      delay(100);
      httpsClient->stop();
    }
  }
  delete httpsClient;
  return res;
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
ApePixelClock &ApePixelClock::plCoord(uint16_t x, uint16_t y)
{
  x += m_offsetX;
  x += m_offsetY;
  payload[payLoadPointer++] = (x >> 8) & 0xFF;
  payload[payLoadPointer++] = (x)&0xFF;
  payload[payLoadPointer++] = (y >> 8) & 0xFF;
  payload[payLoadPointer++] = (y)&0xFF;
  return APC;
}
ApePixelClock &ApePixelClock::plByte(byte b)
{
  payload[payLoadPointer++] = b;
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

int ApePixelClock::textCenterX(int strLength, int charWidth, int maxCharCount)
{
  if (strLength > maxCharCount)
    strLength = maxCharCount;
  return (maxCharCount - strLength) * charWidth / 2;
}

void ApePixelClock::drawColorIndexFrame(const uint32 *colorMap,
                                        unsigned char width, unsigned char height, const uint32 *pixels)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int count = y * (((width - 1) / 4) + 1) + (x / 4);
      unsigned long color = colorMap[pixels[count] >> (((3 - x % 4)) * 8) & 0xFF];
      APC.plBegin().plPid(4).plCoord(x, y).plColor(color >> 16 & 0xFF, color >> 8 & 0xFF, color & 0xFF).plCallback();
    }
  }
}

ApePixelClock APC;