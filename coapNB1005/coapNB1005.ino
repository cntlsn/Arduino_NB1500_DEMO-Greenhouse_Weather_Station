#include "coap.h"
#include "MKRNBIoT.h"
#include <SimpleDHT.h>

#define GPRS_APN "22210"
#define PINNUMBER ""

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

GSMUDP udp;
GPRS gprs;
GSM gsmAccess(true);
GSMScanner scannerNetworks;

unsigned long lastConnectionTime = 0; // track the last connection time
const unsigned long postingInterval = 15L * 1000L; // post data every 20 seconds

#define pinDHT11 2    // temp/humidity sensor on pin 2
SimpleDHT22 dht11;

// read with raw sample data.
byte temperature = 0;
byte humidity = 0;
byte DHTdata[40] = {0};

#define LDR A0   // LDR on pin A0
int light;

String rssi;    // variable to store WiFi wignal strength data

#define Rpin 8  // RGB Led Red on pin 8
#define Gpin 7  // RGB Led Green on pin 7
#define Bpin 6  // RGB Led Blue on pin 6

// UDP and CoAP class
Coap coap(udp);

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;

  Serial.println(p);
}

void setup() {
  Serial.begin(9600);

  while (!Serial) {}
  pinMode(Rpin, OUTPUT);
  pinMode(Gpin, OUTPUT);
  pinMode(Bpin, OUTPUT);
  digitalWrite(Rpin, LOW);
  digitalWrite(Gpin, LOW);
  digitalWrite(Bpin, LOW);
  
   Serial.println("GSM networks scanner");
  scannerNetworks.begin();
  // connection state
  boolean connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    Serial.println("connected");
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachNBIoT(GPRS_APN) == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  // client response callback.
  // this endpoint is single callback.
  Serial.println("Setup Response Callback");
  coap.response(callback_response);

  // start coap server/client
  coap.start();
}

void loop() {
  if (millis() - lastConnectionTime > postingInterval) {
    // check WiFi signal strength
    rssi = scannerNetworks.getSignalStrength();// WiFi.RSSI();
    Serial.print("Signal Strength: "); Serial.println(rssi);
    //Serial.println();
    // check light sensor
    light = analogRead(LDR);
    Serial.print("light: "); Serial.println(light);

    // check temp/humidity sensor
    dht11.read(pinDHT11, &temperature, &humidity, DHTdata);
    Serial.print("temp: "); Serial.println((int)temperature);
    Serial.print("humidity: "); Serial.println((int)humidity);

    httpRequest();
  }
}


void httpRequest() {

  // read analog pin 0
  // create data string to send to ThingSpeak
 // String data = String("field1=" + String(light, DEC) + "&field2=" + rssi);
  String data = String("field1=" + String((int)light, DEC) + "&field2=" + String((int)temperature, DEC) + "&field3=" + String((int)humidity, DEC) + "&field4=" + rssi);

  // build and send request
 // String data = String("field1=" + String((int)random(1000), DEC) + "&field2=" + String((int)random(1000), DEC) + "&field3=" + String((int)random(100), DEC) + "&field4=" + String((0-random(100)), DEC));
  int msgid = coap.post(IPAddress(35, 169, 153, 65), 5683, "Y956FYQG/measures", (char*)data.c_str());
  coap.loop();

  Serial.println("REQUEST SENT!");
  Serial.println(data);

  // note the last connection time
  lastConnectionTime = millis();

  // LED feedback on request sent
  for (int i = 3; i >= 0; i--) {
    analogWrite(Gpin, i);
    delay(3);
  }
  for (int i = 0; i <= 255; i++) {
    analogWrite(Gpin, i);
    delay(3);
  }
  for (int i = 255; i >= 3; i--) {
    analogWrite(Gpin, i);
    delay(3);
  }
}
