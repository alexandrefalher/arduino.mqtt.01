#include <WiFiNINA.h>
#include <MQTT.h>
#include <DHT.h>

#define SERIAL_ENABLED true
#define BAUDRATE 115200
#define WIFI_SSID ""
#define WIFI_PWD ""
#define MQTT_BROKER_ADDRESS ""
#define MQTT_BROKER_USER_ID ""
#define MQTT_BROKER_USER_USERNAME ""
#define MQTT_BROKER_USER_PWD ""
#define MQTT_SENSOR_TOPIC ""
#define MQTT_RECEPTOR_TOPIC ""
#define DHTPIN 0
#define DHTTYPE DHT11
#define LED_PIN 2

unsigned long lastMillis = 0;



/// ################ SERIAL #######################

void Serial_Begin()
{
    Serial.begin(BAUDRATE);
}

void Serial_print(String message)
{
    if(SERIAL_ENABLED == 1)
    {
        Serial.print(message);
    }
}

void Serial_println(String message)
{
    if(SERIAL_ENABLED == 1)
    {
        Serial.println(message);
    }
}



/// ################ WIFI #######################

WiFiClient wifi_client;

void WiFi_Begin()
{
    WiFi.begin(WIFI_SSID, WIFI_PWD);
}

void WiFi_Connect()
{
    Serial_print("checking wifi...");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial_print(".");
        delay(1000);
    }

    Serial_print("\n");
    Serial_println("WiFi connected!");
}



/// ################ MQTT #######################

MQTTClient mqtt_client;

void Mqtt_Subscribe(String topic)
{
    mqtt_client.subscribe(topic);
}

void Mqtt_Connect()
{
    Serial_print("\nconnecting...");
    while (!mqtt_client.connect("arduino-mkr1010", "", ""))
    {
        Serial_print(".");
        delay(1000);
    }

    Serial_print("\n");
    Serial_println("Mqtt connected!");

    Mqtt_Subscribe(MQTT_RECEPTOR_TOPIC);
}

bool Mqtt_Connected()
{
    return mqtt_client.connected();
}

void Mqtt_Loop()
{
    mqtt_client.loop();
}

void MessageReceived(String &topic, String &payload) {
  Serial_println("incoming: " + topic + " - " + payload);
  if(topic == MQTT_RECEPTOR_TOPIC)
  {
    SetLed(payload.toInt());
  }
}

void Mqtt_Publish(String topic, String message)
{
    mqtt_client.publish(topic, message);
}

void Mqtt_Begin()
{
    mqtt_client.begin(MQTT_BROKER_ADDRESS, wifi_client);
    mqtt_client.onMessage(MessageReceived);
}



/// ################ SENSOR #######################

DHT dht(DHTPIN, DHTTYPE);

void DHT_Begin()
{
    dht.begin();
}

float DHT_ReadTemperature_AsFloat()
{
    return dht.readTemperature();
}

int DHT_ReadTemperature_AsInt()
{
    return int(DHT_ReadTemperature_AsFloat());
}

String DHT_ReadTemperature_AsString(bool integer = true)
{
    if(!integer)
    {
        return String(DHT_ReadTemperature_AsFloat());
    }

    return String(DHT_ReadTemperature_AsInt());
}



/// ################ RECEPTOR #######################

void InitializeLedPin()
{
    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, 0);
}

void SetLed(int value)
{
    if(value < 0)
    {
        analogWrite(2, 0);
    }
    else if(value > 254)
    {
        analogWrite(2, 254);
    }
    else
    {
        analogWrite(2, value);
    }
}



/// ################ COMMON #######################

void setup() {
  Serial_Begin();
  InitializeLedPin();
  WiFi_Begin();
  Mqtt_Begin();
  DHT_Begin();

  WiFi_Connect();
  Mqtt_Connect();
}

void loop() {
  Mqtt_Loop();

  if (!Mqtt_Connected()) {
    WiFi_Connect();
    Mqtt_Connect();
  }

  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    
    String t = DHT_ReadTemperature_AsString();
    Mqtt_Publish("/temp", t);
  }
}