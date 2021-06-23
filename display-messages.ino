#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiSSLClient.h>
#include <WiFi101.h>
#include <ArduinoHttpClient.h>

#include <SPI.h>

// network name
char ssid[] = "";
// network pass
char pass[] = "";
int keyIndex = 0;
bool val = true;

int status = WL_IDLE_STATUS;

WiFiServer server(80);

#include <LiquidCrystal.h>

// pin setup
const int rs = 0, en = 1, d4 = 2, d5= 3, d6 = 4, d7 = 5;
const int buzzer = 7;
// LCD conf
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(buzzer, OUTPUT);
  analogWrite(A1, 0);
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial.print("Start Serial ");
  Serial.print("WiFi 101 shield: ");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("NOT PRESENT");
    return;
  }
  Serial.println("DETECTED");
  while ( status != WL_CONNECTED ) {
    lcd.clear();
    lcd.print("connecting...");
    Serial.print("Attempting to connect to network...");
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  Serial.println("Success!");
  IPAddress ip = WiFi.localIP();
  server.begin();
  lcd.clear();
  lcd.print(ip);
}

void loop() {
  WiFiClient client = server.available();
  bool post = false;
  if (client) {
    Serial.println("new client");
    String currentLine = "";
    String message = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (post) {
              post = false;
            }
            else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.print("<form method=\"post\" action=\"/lcd_post\"><label for=\"lcdMsg\">LCD Message</label><input type=\"text\" maxlength=\"16\" id=\"lcdMsg\" name=\"lcdMsg\"><button type=\"submit\">Submit</button></form>");
              client.println();
              break;
            }
          } 
          else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }
        if (currentLine.endsWith("POST /lcd_post")) {
          post = true;
          Serial.println();
          Serial.println("client is sending a new message to the LCD");
        }
        if (currentLine.startsWith("lcdMsg=")) {
          message = currentLine;
          message.replace("lcdMsg=", "");
          if (message.length() != 0) {
            Serial.println();
            Serial.print("New LCD Message: ");
            Serial.println(message);
            lcd.clear();
            lcd.print(message);
            tone(buzzer, 250);
            delay(100);
            noTone(buzzer);
          }
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}