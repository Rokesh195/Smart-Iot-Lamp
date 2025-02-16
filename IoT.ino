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
#define SINRIC_API_KEY "ff23af34-60ce-4713-9063-8638198c49a8"
#define SINRIC_APP_SECRET "7d296f78-38e7-4c91-9741-8c33ee30adcf-37623b51-c4eb-49b8-acee-e140132c090d"
#define SINRIC_DEVICE_ID "670173e70d73840bd1d2530e"

// AWS IoT Credentials
const char* AWS_ENDPOINT = "a1mtxi7lam8esu-ats.iot.eu-north-1.amazonaws.com";
const char* MQTT_SUB_TOPIC = "esp32/sub";

// AWS Certificate Strings
const char* AWS_CERT_CA = R"EOF( 
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
 )EOF";
 
const char* AWS_CERT_CRT = R"EOF( 
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUEQrIiI9KYHoTVlfWHSqatPvia1MwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI0MTAyMzEwMjI1
MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ0TA1p1Y5DCpJ4hj27
Oa61gpsC+yLR8f5RyMIUG+l/S2s1+gnU6SDUDuy3wIAFNxnUIxSBaVjdrlBM45Ef
bSRVERdQP3t2ncpo49MQ4eXg35Er47XDGV56V9hKnY9iixEb4WJ9hn6tjbVsFUh4
+iOZ50Y9tefncSvRQ43eySBO2tZwhlu8uGHWwGB7LRx0fyG7mp+UZ6+TibxQFA0p
tSEyx/5TI1NUSl5dDT/25zCwqO36uuwa3H6iYVtYZgdMf5ouY5Vr+X4gLixlXxxJ
oAl1jOTtgYNak1Lky1QBKDDGCETh2ggivg9hxlxBE/HWDsdGdvZi9wqwetVUQEfs
5k0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUgzLsTN4r6so03Xi5+87HA1GXLLQwHQYD
VR0OBBYEFHZGiTrmomAIAqpCWp+nGAcP2LiHMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCGOAbriqrdBoitmCtd9wSVCoOC
VO3n1aLQiheUkLKe1C/27hosoxnUPMPc0YsPoxMpZYsSuAInM+d210WHX6ukSBY8
cx3gMSL1RGmie8vQac0q2chfwTHigYp9ae9ysbCf9vzgRvmqJ14MWeGruZLrQmix
HGqVUhXT5OJPETY+vlTBRtXZ+HRt7mOVPlwxL3agbQegB/ncrqHBc+tXalmQzpPh
D7izJPP1MK7fwMTBFfb/+1GlFMBKxauagX8rj5HGKb7aa02FCV6OJt3eOcC9ImV0
bQN0kkq3KA3Kpk4dQqPoaT6LHW1qqOPCRbN7d/TRhemwo5LTQCjBuc/A67c+
-----END CERTIFICATE-----
 )EOF";

const char* AWS_CERT_PRIVATE = R"EOF( 
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAsnRMDWnVjkMKkniGPbs5rrWCmwL7ItHx/lHIwhQb6X9LazX6
CdTpINQO7LfAgAU3GdQjFIFpWN2uUEzjkR9tJFURF1A/e3adymjj0xDh5eDfkSvj
tcMZXnpX2Eqdj2KLERvhYn2Gfq2NtWwVSHj6I5nnRj215+dxK9FDjd7JIE7a1nCG
W7y4YdbAYHstHHR/Ibuan5Rnr5OJvFAUDSm1ITLH/lMjU1RKXl0NP/bnMLCo7fq6
7BrcfqJhW1hmB0x/mi5jlWv5fiAuLGVfHEmgCXWM5O2Bg1qTUuTLVAEoMMYIROHa
CCK+D2HGXEET8dYOx0Z29mL3CrB61VRAR+zmTQIDAQABAoIBACYByujYb5lbAfds
H/sp0wE4u1ZUbjReTpIBzSY4rjEu4Vs2JcjORdCuGXelMySQ98lSex406cD5hdSo
a5dPACnKzFcQylR7UOP5LvyXDeWt3S/IDzl5EgAjQm5YhfIx5LiCHaCiZ3C3egT3
vhivW2cVskIthn6btmjEqwSePGvaxIUWG+nmSYPx58ipfgYjezC0qDIkierH4y2I
o5YaWvDN/we9vHF5ekM5kGQW355IWt7Kmva8UNL6qbYk4pE4q4E57YV9LaKEE2/p
17qtAn1R3lgPVqoyEiY7lIRFlUsu5tcsfGS6K8QJmG+fe9QX5PosV30XhNuJ5xyC
kLoIisECgYEA2ettG/MLDi/J8ehISYOE9XAiD+L+BfvjDmdBvzHg4jfRbMsm/w+s
Nv+aoMJ9SD8tsF0t7nM3INgQSfSh8+pJSUAvS0N9i3O2Eaq1WrfwoShawtNFg7Im
Kz1mAkw8hyIRrE9ykE3afPjXXWzXFGgTVbrFQv4FCdQvElNP6iZU+okCgYEA0aNm
SmLI27YTx+4Ib54LInf52dgMYoV5bHM/iZdHMNTlFRs99ykZwaGF3qY0e03zDGOn
m7KtFe/L2iDcNvG0qfk0V8DFGxwG44pbi9jfAiNO4hi91yB1ut5UztQnyYDHw2Yr
Uz4a6CsYEKhA42lEFCoVG+F+8NyV0/C6QP3hDKUCgYEAwQjHdNuc7mssosmUxvHm
pBc5RQNY62oN3mNfHQMM5rZ1H77xAz0Z2voeOcsueQHl8fUIS9HpgoGCJFC76Tme
w3FEs5HMMuxD2nByBzMqx0bc5vmI+TikkObpZp2rfDSUnSMfI26jXNJ8y7gu6Ver
0OQJ+ubTBbrcG4qLHqnn2qkCgYBLIgTHJCBYnij4vpbHah0TfWxOZuAJb0zVUXi0
F0OsfKCdr+XFbzWL8BfKdEba6lTxW8X2R1NBSXAF/UDvcDEPugDIco4x6RmO80dc
F53VZlIfNj7Tmw6Wqpg6iE66KS1vGm7sppV+H/PABI08870yfOKT231jVatdCim2
oUbHAQKBgQCYD+4iEKvwXaBYJyNdV6e8kuud5REnY2p0miDcUvM0ePfLLl5db5No
EHyE3x6CkEUs8XgFYPaM+1jTBxiZu06vHn8YliwUs1Zh/aypGeoMthqZ+RTPNJwx
3pFgOSvXVivvIxRjCIkN0hu/cPZLSkMHPzpmHWjmtD/bere3g/j8AA==
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