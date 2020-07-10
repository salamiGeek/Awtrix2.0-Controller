#ifndef __APEPIXELCLOCK_H
#define __APEPIXELCLOCK_H

#include <Arduino.h>
#include "RTClib.h"
#include <PubSubClient.h>

#define MAX_APCEffECTAREA_COUNT 8

struct ApcEffectAreaDef
{
  int x; // from top-left to rigth-bottom
  int y;
  int width;
  int height;
  int frameCount;
  long frameRefreshTime; //ms
  int currentFrameCount;
  long currentRefreshTime;
};

typedef void (*ApcScheduleCallback)();
typedef void (*ApcEffectCallback)(unsigned int, unsigned int);

struct ApcEffectDef
{
  int areaCount;
  ApcEffectAreaDef areaDef[MAX_APCEffECTAREA_COUNT];
  ApcEffectCallback callbackFunc;
  int currentAreaIndex;
  long autoChangeTime;
  long currentChangeTime;
};

struct ApcScheduleCallbackDef
{
  long callbackTime;
  long currentRefreshTime;
  ApcScheduleCallback callbackFunc;
};

class ApePixelClock
{
public:
  MQTT_CALLBACK_SIGNATURE;

public:
  ApePixelClock();
  void systemInit(MQTT_CALLBACK_SIGNATURE, RTC_DS1307 *rtc);
  void apcLoop();
  void publish(String &s);
  ApePixelClock &plBegin();
  ApePixelClock &plPid(byte pid);
  ApePixelClock &plCoord(uint16_t c);
  ApePixelClock &plColor(byte r, byte g, byte b);
  ApePixelClock &plStr(const String &str);
  void plCallback();

private:
  bool internetConnected();
  void upgradeTime();
  uint32_t requestSecondStamp(int *errCode);
  String httpRequest(const String &url, int *errCode);
  void addApcEffects();
  void addApcEffect(ApcEffectDef *apcEffect);
  void addApcScheduleCallback(unsigned long callbackTime, ApcScheduleCallback scheduleCallback);
  void renderCheck();
  void renderAction(ApcEffectDef *apcEffect, bool needArea = true);
  void show();
  void clear();
};

extern ApePixelClock APC;

#endif // __APEPIXELCLOCK_H
