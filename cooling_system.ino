#include <DHT.h> // Including library for dht
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WifiClientSecure.h>
#include "ThingSpeak.h"

#define WIFI_SSID "Your Wi-Fi SSID"
#define WIFI_PASSWORD "Your Wi-Fi Password"

//---------Hive MQ Channel Details---------//
#define mqtt_server "1ec4d4cdb4ed402694590af113b150f6.s1.eu.hivemq.cloud"
#define mqtt_username "esp8266"
#define mqtt_password "Esp8266cooling"
#define mqtt_port 8883

//---------Thingspeak Channel Details---------//
#define channelNumber 2518317          // Channel ID
#define readAPIKey "BWQIC1414ORGX997"  // Read API Key
#define writeAPIKey "3ZVB1MLC9OM7NK0Z" //  Enter your Write API key from ThingSpeak
#define tempField 1                    // The field you wish to read
#define humField 2                     // The field you wish to read
#define fanField 3                     // The field you wish to read

#define DHTPIN 2 // pin where the dht11 is connected
#define FANPIN 4 // pin where the Fan relay is connected

DHT dht(DHTPIN, DHT11);

WiFiClientSecure espClient;

WiFiClient thingClient;

PubSubClient client(espClient);

int fan_thresh;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg(MSG_BUFFER_SIZE);

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-mahran";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      client.subscribe("fan");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print("try again in 5 sec");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMsg = "";
  for (int i = 0; i < length; i++)
    incommingMsg += (char)payload[i];

  Serial.println("Message arrived [" + String(topic) + "] " + incommingMsg);

  //--- check the incomming message
  if (strcmp(topic, "fan") == 0)
  {
    fan_thresh = atoi(incommingMsg.c_str());
    Serial.print("New fan threshold: ");
    Serial.println(fan_thresh);
  }
}

void publishMsg(const char *topic, String payload)
{
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message published [" + String(topic) + "]" + payload);
}

void startFan(float temp)
{
  if (temp > fan_thresh)
  {
    digitalWrite(FANPIN, HIGH);
    Serial.println("Cooling");
  }
  else
  {
    digitalWrite(FANPIN, LOW);
    Serial.println("Fan is off");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  pinMode(FANPIN, OUTPUT);
  dht.begin();
  ThingSpeak.begin(thingClient);

  Serial.println("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
    reconnect();
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  startFan(t);

  DynamicJsonDocument tempDoc(1024);
  DynamicJsonDocument humDoc(1024);

  humDoc["humidity"] = h;
  tempDoc["temperature"] = t;

  char temp_message[128];
  char hum_message[128];

  serializeJson(tempDoc, temp_message);
  serializeJson(humDoc, hum_message);

  publishMsg("temp", temp_message);
  publishMsg("hum", hum_message);

  delay(1000);

  fan_thresh = ThingSpeak.readLongField(channelNumber, fanField, readAPIKey);
  int statusCode = ThingSpeak.getLastReadStatus();

  if (statusCode == 200)
  {
    Serial.print("Fan Threshold: ");
    Serial.println(fan_thresh);
    startFan(t, fan_thresh);
  }
  else
  {
    Serial.println("Unable to read channel / No internet connection");
  }
  delay(100);

  ThingSpeak.setField(tempField, t);
  ThingSpeak.setField(humField, h);

  int writeResp = ThingSpeak.writeFields(channelNumber, writeAPIKey);
  // thingspeak needs minimum 15 sec delay between updates
  delay(15000);

  if (writeResp == 200)
  {
    Serial.println("Channel update successful.");
  }
  else
  {
    Serial.println("Problem updating channel. HTTP error code " + String(writeResp));
  }

  thingClient.stop();

  Serial.println("Waiting...");

  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);
}
