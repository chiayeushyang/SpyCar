#include <WiFi.h> //Wifi library
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks
byte mac[6];
const char* host = "arduino.clanweb.eu"; //webserver
String url = "/eduroam/data.php"; //URL to target PHP file

//Identity for user with password related to his realm (organization)
//Available option of anonymous identity for federation of RADIUS servers or 1st Domain RADIUS servers

#define EAP_ANONYMOUS_IDENTITY "chiayeushyang0825" //anonymous@example.com, or you can use also nickname@example.com
#define EAP_IDENTITY "chiayeushyang0825" //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
#define EAP_PASSWORD "020825100291" //password for eduroam account

//SSID NAME
const char* ssid = "NEWERA"; // eduroam SSID



void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect from WiFi to set new WiFi connection
  WiFi.mode(WIFI_STA); //init wifi mode
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
  http_request(); //I will receive information about successful connection and identity realm (i can write it into Github project page as u have tested it)
}
void loop() {
  yield();
}

void http_request() {
  WiFiClient client;
  delay(1000);
  client.stop();
  String data = "ssid=" + String(ssid) + "&identity=" + String(EAP_IDENTITY) + "&anonymous_identity=" + String(EAP_IDENTITY);
  if (client.connect(host, 80)) {
    Serial.println(F("Connected to webserver!"));
    client.println("POST " + url + " HTTP/1.0");
    client.println("Host: " + (String)host);
    client.println(F("User-Agent: ESP32"));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/x-www-form-urlencoded;"));
    client.print(F("Content-Length: "));
    client.println(data.length());
    client.println();
    client.println(data);
    Serial.println(F("Data received by server, THANKS for trying this eduroam connection example!"));
    while (client.connected()) {
      String line = client.readStringUntil('\n'); //HTTP HEADER
      if (line == "\r") {
        break;
      }
    }
    String line = client.readStringUntil('\n'); //HTTP PAYLOAD
  } else {
    Serial.println(F("Connection wasn't sucessful, try again later"));
  }
}