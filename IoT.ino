#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// WiFi Credentials
#define WIFI_SSID "IOT"
#define WIFI_PASS "12345678"

// Relay Control Pin (connected to ESP32 GPIO 2)
#define RELAY_PIN 2  

// Sinric Pro Credentials
#define SINRIC_API_KEY ""
#define SINRIC_APP_SECRET ""
#define SINRIC_DEVICE_ID ""

// AWS IoT Credentials
const char* AWS_ENDPOINT = "";
const char* MQTT_SUB_TOPIC = "esp32/sub";

// AWS Certificate Strings
const char* AWS_CERT_CA = R"EOF( 
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
 )EOF";
 
const char* AWS_CERT_CRT = R"EOF( 
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
 )EOF";

const char* AWS_CERT_PRIVATE = R"EOF( 
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
 )EOF";

// Global Objects
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// Function to handle SinricPro switch request
bool onPowerState(const String& deviceId, bool state) {
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  Serial.printf("Device %s turned %s\r\n", deviceId.c_str(), state ? "ON" : "OFF");
  return true;
}

// Function to handle AWS IoT messages
void messageHandler(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  Serial.printf("Message from AWS IoT: %s\r\n", message.c_str());

  // Check if message is "ON" or "OFF" and control the relay
  if (message == "ON") {
    digitalWrite(RELAY_PIN, HIGH);  // Turn relay ON
    Serial.println("Relay turned ON by AWS IoT");
  } else if (message == "OFF") {
    digitalWrite(RELAY_PIN, LOW);   // Turn relay OFF
    Serial.println("Relay turned OFF by AWS IoT");
  }
}

// Setup function
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Configure AWS IoT
  secureClient.setCACert(AWS_CERT_CA);
  secureClient.setCertificate(AWS_CERT_CRT);
  secureClient.setPrivateKey(AWS_CERT_PRIVATE);
  mqttClient.setServer(AWS_ENDPOINT, 8883);
  mqttClient.setCallback(messageHandler);

  // Connect to AWS IoT
  while (!mqttClient.connect("ESP32Client")) {
    Serial.println("Connecting to AWS IoT...");
    delay(1000);
  }
  Serial.println("Connected to AWS IoT");
  mqttClient.subscribe(MQTT_SUB_TOPIC);

  // Configure SinricPro
  SinricProSwitch& mySwitch = SinricPro[SINRIC_DEVICE_ID];
  mySwitch.onPowerState(onPowerState);

  SinricPro.begin(SINRIC_API_KEY, SINRIC_APP_SECRET);
}

// Loop function
void loop() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connect("ESP32Client")) {
      Serial.println("Reconnecting to AWS IoT...");
      delay(1000);
    }
  }
  mqttClient.loop();
  SinricPro.handle();
}
