#include <Arduino.h>
#include <Servo.h>
#include <WiFiNINA.h>
// All libraries needed

Servo myservo1;
Servo myservo2;

char ssid[] = "Bombaclatt";        // Your Wi-Fi name
char pass[] = "Bombarass";         // Your Wi-Fi password
int keyIndex = 0;

int laser = 13; // Laser Connected to pin 13
int status = WL_IDLE_STATUS; // Wi-Fi status tracking
WiFiServer server(80); // Create server on port 80

//declare variables for the motor pins
//Motor3 pins
int motorUppNedPin1 = 2; // Blue - 28BYJ48 pin 1
int motorUppNedPin2 = 3; // Pink - 28BYJ48 pin 2
int motorUppNedPin3 = 4; // Yellow - 28BYJ48 pin 3
int motorUppNedPin4 = 5; // Orange - 28BYJ48 pin 4

int motorSpeed = 1200; //variable to set stepper speed
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001}; // step sequence

bool laserWasOn = false; //Tracks if laser was previously on

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for Serial monitor to open

  pinMode(laser, OUTPUT); // Set laser pin as output
  myservo1.attach(9); // Attach servo1 to pin 9
  myservo2.attach(10); // Atach servo2 to pin 10

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
  status = WiFi.beginAP(ssid, pass); // Start Access Point mode
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true);
  }

  delay(10000); // Wait 10 seconds for AP to fully start
  server.begin(); // Start the web server
  printWiFiStatus(); // Show network info

  //declare the motor pins as outputs
  pinMode(motorUppNedPin1, OUTPUT);
  pinMode(motorUppNedPin2, OUTPUT);
  pinMode(motorUppNedPin3, OUTPUT);
  pinMode(motorUppNedPin4, OUTPUT);
}

void loop() {
  WiFiClient client = server.available(); // Check for incoming client

  if (client) {
    Serial.println("New client");
    String req = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        // End of HTTP header
        if (c == '\n') {
          Serial.println(req);
          // Check for GET request containing "POS?"
          if (req.indexOf("GET /POS?") >= 0) {
            int xIndex = req.indexOf("x=");
            int yIndex = req.indexOf("&y=");
            if (xIndex > 0 && yIndex > xIndex) {
              // Extract x and y values from URL
              int xVal = req.substring(xIndex + 2, yIndex).toInt();
              int yVal = req.substring(yIndex + 3).toInt();
              // If x and y are both 999, trigger laser + motion action
              if (xVal == 999 && yVal == 999) {
                digitalWrite(laser, HIGH);

                if (!laserWasOn) {
                  // Move motor up and down once
                  for (int i = 0; i < 256; i++) { uppat(); }
                  delay(200);
                  for (int i = 0; i < 256; i++) { nerat(); }
                  delay(200);

                  laserWasOn = true; // Prevent repeat until reset
                }

                
              } else {
                digitalWrite(laser, LOW);
                laserWasOn = false; //reset when laser turns off
                // Move servos based on x/y from hand tracking
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
          // Send HTTP response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");
          client.println();
          client.println("OK");
          break;
        }
      }
    }

    delay(1); // tiny delay before disconnect
    client.stop(); // close connection
    Serial.println("Client disconnected");
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID()); // Show SSID
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Show local IP
}

void nerat(){
  for (int i = 0; i < 8; i++)
  {
    setOutputUppNed(i);
    delayMicroseconds(motorSpeed); // Set coil pattern for down motion
    Serial.print("Ned "); // Wait between steps
  }
}

void uppat(){
  for (int i = 7; i >= 0; i--)
  {
    setOutputUppNed(i);
    delayMicroseconds(motorSpeed); // Set coil pattern for up motion
    Serial.print("Upp "); // Wait between steps
  }
}

void setOutputUppNed(int out)
{ // Set all 4 pins of stepper motor using lookup table
  digitalWrite(motorUppNedPin1, bitRead(lookup[out], 0));
  digitalWrite(motorUppNedPin2, bitRead(lookup[out], 1));
  digitalWrite(motorUppNedPin3, bitRead(lookup[out], 2));
  digitalWrite(motorUppNedPin4, bitRead(lookup[out], 3));
}

