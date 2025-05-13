#include <Arduino.h>
#include <Servo.h>
#include <WiFiNINA.h>

Servo myservo1;
Servo myservo2;

char ssid[] = "Bombaclatt";        // Your Wi-Fi name
char pass[] = "Bombarass";         // Your Wi-Fi password
int keyIndex = 0;

int laser = 13;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

//declare variables for the motor pins
//Motor3 pins
int motorUppNedPin1 = 2; // Blue - 28BYJ48 pin 1
int motorUppNedPin2 = 3; // Pink - 28BYJ48 pin 2
int motorUppNedPin3 = 4; // Yellow - 28BYJ48 pin 3
int motorUppNedPin4 = 5; // Orange - 28BYJ48 pin 4

int motorSpeed = 1200; //variable to set stepper speed
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};

bool laserWasOn = false; //Tracks if laser was previously on

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  pinMode(laser, OUTPUT);
  myservo1.attach(9);
  myservo2.attach(10);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  Serial.print("Creating access point named: ");
  Serial.println(ssid);
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true);
  }

  delay(10000);
  server.begin();
  printWiFiStatus();

  //declare the motor pins as outputs
  pinMode(motorUppNedPin1, OUTPUT);
  pinMode(motorUppNedPin2, OUTPUT);
  pinMode(motorUppNedPin3, OUTPUT);
  pinMode(motorUppNedPin4, OUTPUT);
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client");
    String req = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;

        if (c == '\n') {
          Serial.println(req);

          if (req.indexOf("GET /POS?") >= 0) {
            int xIndex = req.indexOf("x=");
            int yIndex = req.indexOf("&y=");
            if (xIndex > 0 && yIndex > xIndex) {
              int xVal = req.substring(xIndex + 2, yIndex).toInt();
              int yVal = req.substring(yIndex + 3).toInt();

              if (xVal == 999 && yVal == 999) {
                digitalWrite(laser, HIGH);

                if (!laserWasOn) {

                  for (int i = 0; i < 256; i++) { uppat(); }
                  delay(200);
                  for (int i = 0; i < 256; i++) { nerat(); }
                  delay(200);

                  laserWasOn = true; //mark as already handled
                }

                
              } else {
                digitalWrite(laser, LOW);
                laserWasOn = false; //reset when laser turns off

                xVal = constrain(xVal, 0, 180);
                yVal = constrain(yVal, 0, 180);
                myservo1.write(xVal);
                myservo2.write(yVal);
              }

              Serial.print("X: ");
              Serial.print(xVal);
              Serial.print(" Y: ");
              Serial.println(yVal);
            }
          }

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");
          client.println();
          client.println("OK");
          break;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void nerat(){
  for (int i = 0; i < 8; i++)
  {
    setOutputUppNed(i);
    delayMicroseconds(motorSpeed);
    Serial.print("Ned ");
  }
}

void uppat(){
  for (int i = 7; i >= 0; i--)
  {
    setOutputUppNed(i);
    delayMicroseconds(motorSpeed);
    Serial.print("Upp ");
  }
}

void setOutputUppNed(int out)
{
  digitalWrite(motorUppNedPin1, bitRead(lookup[out], 0));
  digitalWrite(motorUppNedPin2, bitRead(lookup[out], 1));
  digitalWrite(motorUppNedPin3, bitRead(lookup[out], 2));
  digitalWrite(motorUppNedPin4, bitRead(lookup[out], 3));
}

