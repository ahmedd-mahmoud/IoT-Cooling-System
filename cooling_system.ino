#include <DHT.h> // Including library for dht
#include <ESP8266WiFi.h>

String apiKey = "NMAHB3FL4DO0LQ1W"; //  Enter your Write API key from ThingSpeak

#define WIFI_SSID "Your Wifi SSID"
#define WIFI_PASSWORD "Your Wifi Password"

const char *server = "api.thingspeak.com";

#define DHTPIN 2 // pin where the dht11 is connected

DHT dht(DHTPIN, DHT11);

WiFiClient thingClient;

void setup()
{
  Serial.begin(115200);
  delay(10);
  dht.begin();

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

  void loop()
  {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    if (thingClient.connect(server, 80)) //   "184.106.153.149" or api.thingspeak.com
    {

      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(t);
      postStr += "&field2=";
      postStr += String(h);
      postStr += "\r\n\r\n";

      thingClient.print("POST /update HTTP/1.1\n");
      thingClient.print("Host: api.thingspeak.com\n");
      thingClient.print("Connection: close\n");
      thingClient.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      thingClient.print("Content-Type: application/x-www-form-urlencoded\n");
      thingClient.print("Content-Length: ");
      thingClient.print(postStr.length());
      thingClient.print("\n\n");
      thingClient.print(postStr);

      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" degrees Celcius, Humidity: ");
      Serial.print(h);
      Serial.println("%. Send to Thingspeak.");
    }
    thingClient.stop();

    Serial.println("Waiting...");

    // thingspeak needs minimum 15 sec delay between updates
    delay(1000);
  }
