#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ADS1X15.h>


#define interval    5

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int16_t adc0, adc1, adc2, adc3;
int16_t adc4, adc5, adc6, adc7;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 2, 177);
IPAddress myDns(192, 168, 2, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):

EthernetClient ethClient;
PubSubClient client(ethClient);
// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
ADS1115 adsV(0x48);     /* Use this for the 12-bit version */
ADS1115 adsA(0x49);     /* Use this for the 12-bit version */

char server[] = "iotserv.io";    // name address for mqtt broker (using DNS)
const char* mqttServer = "iotserv.io";   //"94.177.217.71";
const int mqttPort = 1883;
const char* mqttUser = "deviceToken001";
const char* mqttPassword = "";
int errCount = 0;
unsigned long previousTime = 0;
char json[512];

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  adsV.begin();
  adsA.begin();

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }


  // give the Ethernet shield a second to initialize:
  delay(1000);

}

void loop()
{
  read2ADC();

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (millis() / 1000 < previousTime)
  {
    previousTime = 0;
  }
  if (((millis() / 1000) - previousTime) >= interval)
  {
    previousTime = millis() / 1000;
    Serial.println("Send: ");
    sendData();
    client.disconnect();
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  //Serial1.print("Message arrived on topic: ");
  //Serial1.print(topic);
  //Serial1.print(". Message: ");

  String messageTemp;
  String topicStr = String(topic);

  for (unsigned int i = 0; i < length; i++) {
    //Serial1.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  //Serial1.println();
}

void reconnect()
{
  //resetETH();
  //readMacI2C();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  //Ethernet.begin(mac, ip, ip_dns, gateway, subnet);
  Ethernet.begin(mac, ip, myDns);

  // Loop until we're reconnected
  while (!client.connected())
  {
    //Serial1.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Ethernet", mqttUser, NULL))
    {
      //Serial1.println("connected");
      // Once connected, publish an announcement...
      client.publish("EGAT/", "It is reconnected");
      client.publish("v1/devices/me/telemetry", "{\"word\":\"Hello\"}");

    } else
    {
      //Serial1.print("failed, rc=");
      //Serial1.print(client.state());
      //Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      errCount++;
    }
    if (errCount >= 10)
    {
      //NVIC_SystemReset();      // processor software reset
    }
  }
}

void sendData()
{
  StaticJsonDocument<512> JSONbuffer;
  JsonObject JSONencoder = JSONbuffer.to<JsonObject>();

  JSONencoder["adc0"] = adc0;
  JSONencoder["adc1"] = adc1;
  JSONencoder["adc2"] = adc2;
  JSONencoder["adc3"] = adc3;
  JSONencoder["adc4"] = adc4;
  JSONencoder["adc5"] = adc5;
  JSONencoder["adc6"] = adc6;
  JSONencoder["adc7"] = adc7;

  serializeJson(JSONencoder, json);

  client.publish("v1/devices/me/telemetry", json);
}

void read2ADC()
{
  adc0 = adsV.readADC(0);
  adc1 = adsV.readADC(1);
  adc2 = adsV.readADC(2);
  adc3 = adsV.readADC(3);
  adc4 = adsA.readADC(0);
  adc5 = adsA.readADC(1);
  adc6 = adsA.readADC(2);
  adc7 = adsA.readADC(3);

  Serial.print(adc0);
  Serial.print("\t");
  Serial.print(adc1);
  Serial.print("\t");
  Serial.print(adc2);
  Serial.print("\t");
  Serial.print(adc3);
  Serial.print("\t");
  Serial.print(adc4);
  Serial.print("\t");
  Serial.print(adc5);
  Serial.print("\t");
  Serial.print(adc6);
  Serial.print("\t");
  Serial.print(adc7);
  Serial.println();

}
