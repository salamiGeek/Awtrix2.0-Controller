# AWTRIX2.0-Controller

1.修改B站UID  
  打开 B 站自己的频道，就可以在 URL 中看到自己频道的 ID  
2.修改Youtube频道ID  
  可以在自己频道的 URL 地址内得到，CHANNEL 后面是频道 ID  
3.修改Youtube APIKEY   
   youtube data api v3 的秘钥，需要登录谷歌申请。可以参考 https://zhuanlan.zhihu.com/p/96096543 获取  
4.和风天气创建 APIKEY 方法  
  https://dev.heweather.com/docs/start/get-api-key  
5.和风天气获取城市编码查询  
  https://github.com/heweather/LocationList/blob/master/China-City-List-latest.csv  
  
# 已经修复的坑
## ArduinoSDK 
在ESP8266 Arduino SDK 3.0.0+之后的版本有一些修改可能会导致FastLED库IO错误。知道问题原因后，解决方法也就清晰了，就是更改开发环境的SDK版本到2.6.3，首先在PlatformIO的工程配置文件（.ini）中更改。 
![](res/修改arduino SDK版本.webp)
参考作者：来自火星的Ace https://www.bilibili.com/read/cv13650707/ 出处：bilibili
