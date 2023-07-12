#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Dlink@unifi";
const char* password = "0126627814AC";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttClientId = "ESP32-CAM";
const char* mqttTopic = "topic/espCamCar";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print(WiFi.localIP());

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(mqttClientId)) {
      Serial.println("Connected to MQTT");
      mqttClient.subscribe(mqttTopic);
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  mqttClient.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}