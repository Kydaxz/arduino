//
//  ultrasonic jojo sensor
//  send updates via mqtt to HA
//
//  Author Trevor Steyn <trevor@webon.co.za>
//

// Tank Height in CM and Tank Size in litres 0 cm = full

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Configs

const int tankHeight      = 225;    // Distance from empty to top CM
const int tankSize        = 10500;  // Litres
const int sensorMax       = 300;    // Max distance sensor can read if sensor reads above assume 0 cm
const int sensorSamples   = 20;     // How Many samples to take and get mean value
const int sensorDelay     = 1000;   // How often to read sensor in ms
const int trigPin         = 12;     // Trigger
const int echoPin         = 14;     // Echo
const int sensorPub       = 10000;  // How often to publish sensor values ms
const char* wifiSsid      = "wifi_ssid";
const char* wifiPassword  = "wifi_pass";
const char* mqttServer    = "10.0.0.1";
const char* mqttUser      = "mqtt_user";
const char* mqttPass      = "mqtt_pass";
const char* mqttTopic     = "/home/outside/jojo_tank/jojo_sensor/tele/SENSOR";
const int mqttPort        = 1883;
const char* mqttClient   = "Jojo-Tank-Sensor";

// End of Config

DynamicJsonDocument mqttState(1024);
const int ledPin          = 2;      // LED Pin
const int meanArrayIn     = sensorSamples / 2;
int jojoLitres;
int jojoPercent;
int jojoDistance;
int readings[sensorSamples];      // the readings from the analog input
int readIndex = 0;                // the index of the current reading
int total = 0;                    // the running total
int average = 0;                  // the average
char buffer[512];
long duration, cm;                // longs for timings and cm
unsigned long timer;


// Get some modules ready
WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttClient, mqttUser, mqttPass)) {
      //if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqttTopic, "Online");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  timer = millis();
  //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  client.setServer(mqttServer, mqttPort);
  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
    delay (500);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Reconnect to Mqtt if not connected
    if (!client.connected()) {
      reconnect();
    }
  }
  if (millis() - timer >= sensorPub) {
    // Lets work out percentage
    Serial.print(average);
    Serial.print("/");
    Serial.println(tankHeight);

    jojoPercent = average * 100 / tankHeight; //*100;
    // Lets invert Percentage
    if (jojoPercent > 100) {
      jojoPercent = 0;
    } else {
      jojoPercent = 100 - jojoPercent;
    }
    Serial.print(jojoPercent);
    // Lets work out litres
    jojoLitres = tankSize * jojoPercent / 100;

    String input = "{\"distance\":" + String(average);
    input = input + ",\"percentage\":";
    input = input + String(jojoPercent);
    input = input + ",\"litres\":";
    input = input + String(jojoLitres);
    input = input + "}";
    deserializeJson(mqttState, input);
    serializeJson(mqttState, buffer);
    client.publish(mqttTopic, buffer, true);
    timer = millis();
  }
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  // Convert the time into a distance
  cm = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
  if ( cm > sensorMax ) {
    cm = 0;
  }
  // We have sensor reading now
  readings[readIndex] = int(cm);
  readIndex = readIndex + 1;
  if (readIndex >= sensorSamples) {
    // We have full new list of samples
    sort(readings, sensorSamples);
    average = readings[meanArrayIn];
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // send it to the computer as ASCII digits
  //Serial.println(average);
  //Serial.print(cm);
  //Serial.print("cm");
  //Serial.println();
  delay(sensorDelay);
}

void sort(int a[], int size) {
  for (int i = 0; i < (size - 1); i++) {
    for (int o = 0; o < (size - (i + 1)); o++) {
      if (a[o] > a[o + 1]) {
        int t = a[o];
        a[o] = a[o + 1];
        a[o + 1] = t;
      }
    }
  }
}
