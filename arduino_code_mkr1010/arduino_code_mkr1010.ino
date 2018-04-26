/*
 Arduino --> ThingSpeak Channel via MKR1000 Wi-Fi
 
 The ThingSpeak Client sketch is designed for the Arduino MKR1000 Wi-Fi.
 This sketch writes analog and Wi-Fi signal strength data to a ThingSpeak channel
 using the ThingSpeak API (https://www.mathworks.com/help/thingspeak).
 Arduino Requirements:
 
   * Arduino MKR1000 with ATMEL WINC1500 Wi-Fi module
   * Arduino 1.6.5+ IDE
 
 ThingSpeak Setup:
 
   * Sign Up for New User Account - https://thingspeak.com/users/sign_up
   * Create a new Channel by selecting Channels, My Channels, and then New Channel
   * Enable two fields
   * Note the Write API Key and Channel ID
   Tutorial: http://community.thingspeak.com/tutorials/arduino/using-the-arduino-mkr1000-with-thingspeak/
 
 Setup Wi-Fi
  * Enter SSID
  * Enter Password
   
 Created: May 7, 2016 by Hans Scharler (http://www.nothans.com)
 
 Additional Credits:
 Example sketches from Arduino team, Tom Igoe, and Federico Vanzati
 
*/

#include <SPI.h>
#include <WiFi1010.h>
#include <SimpleDHT.h>


//char ssid[] = "Casa Milo"; //  your network SSID (name)
//char pass[] = "Milon3!?"; // your network password

char ssid[] = "SummerSchoolCR"; //  your network SSID (name)
char pass[] = "CIID2018"; // your network password

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// ThingSpeak Settings
char server[] = "api.thingspeak.com";
String writeAPIKey = "8ZBLLQSA7HIH6FVK";

unsigned long lastConnectionTime = 0; // track the last connection time
//const unsigned long postingInterval = 20L * 1000L; // post data every 20 seconds
const unsigned long postingInterval = 15L * 1000L; // post data every 20 seconds

#define pinDHT11 2    // temp/humidity sensor on pin 2
SimpleDHT11 dht11;

// read with raw sample data.
byte temperature = 0;
byte humidity = 0;
byte DHTdata[40] = {0};

#define LDR 0   // LDR on pin A0
int light;

long rssi;    // variable to store WiFi wignal strength data

#define Rpin 8  // RGB Led Red on pin 8
#define Gpin 7  // RGB Led Green on pin 7
#define Bpin 6  // RGB Led Blue on pin 6

void setup() {
  Serial.begin(9600);
  pinMode(Rpin, OUTPUT);
  pinMode(Gpin, OUTPUT);
  pinMode(Bpin, OUTPUT);
  digitalWrite(Rpin, LOW);
  digitalWrite(Gpin, LOW);
  digitalWrite(Bpin, LOW);
  
  // attempt to connect to Wifi network
  while ( status != WL_CONNECTED) {
    Serial.println("connecting...");
    // Connect to WPA/WPA2 Wi-Fi network
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection
    delay(10000);
  }
  
  Serial.println("connected!");
}

void loop() {
  // if interval time has passed since the last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    // check WiFi signal strength
    rssi = WiFi.RSSI();
    Serial.print("rssi: "); Serial.println(rssi);

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
//  int sensorValue = analogRead(0);
  // read Wi-Fi signal strength (rssi)
//  long rssi = WiFi.RSSI();
  // create data string to send to ThingSpeak
//  String data = String("field1=" + String(light, DEC) + "&field2=" + String(rssi, DEC));
  String data = String("field1=" + String(light, DEC) + "&field2=" + String((int)temperature, DEC) + "&field3=" + String((int)humidity, DEC) + "&field4=" + String(rssi, DEC)); 
  
  // close any connection before sending a new request
  client.stop();

  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    Serial.println("connected to server");

    // build and send request
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);

    Serial.println("REQUEST SENT!");  

    // note the last connection time
    lastConnectionTime = millis();

    // LED feedback on request sent
    for(int i=3; i>=0; i--){
      analogWrite(Gpin, i);
      delay(3);
    }
    for(int i=0; i<=255; i++){
      analogWrite(Gpin, i);
      delay(3);
    }
    for(int i=255; i>=3; i--){
      analogWrite(Gpin, i);
      delay(3);
    }
  }
}

