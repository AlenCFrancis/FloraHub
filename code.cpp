#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SDA_PIN      21
#define SCL_PIN      22
#define SOIL_PIN     34  
#define DHT_PIN      4   
#define TOUCH_PIN    27  
#define IR_PIN       32  
#define BUZZER_PIN   26  
#define TRIG_PIN     18  
#define ECHO_PIN     19  
#define RELAY_PIN    23  

const float TANK_EMPTY_CM = 20.0;     
const int SOIL_DRY_THRESHOLD = 4000;  
const int SOIL_WET_THRESHOLD = 3000;  

const char* ssid     = "";     
const char* password = ""; 

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;
int soilRaw = 0;
float tankDistance = 0.0;
bool isWaterAlarmActive = false;
String waterStatusString = "Tank OK";
bool isPumpRunning = false;
bool isWiFiConnected = false;

unsigned long prevSensorMillis = 0;
const long sensorInterval = 2000; 

unsigned long prevBuzzerMillis = 0;
const long buzzerInterval = 300;  
bool buzzerState = false;

unsigned long prevWiFiMillis = 0;
const long wifiRetryInterval = 10000; 

bool showingSmiley = false;
unsigned long smileyStartMillis = 0;
const unsigned long smileyDuration = 5000; 

unsigned long dataScreenDisplayStartMillis = 0;
const unsigned long minimumDataDisplayDuration = 3000; 

int lastIrState = HIGH;
int lastTouchState = LOW;

void turnPumpOn() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  isPumpRunning = true;
}

void turnPumpOff() {
  pinMode(RELAY_PIN, INPUT); 
  isPumpRunning = false;
}

void setup() {
  Serial.begin(115200);
  delay(500); 
  Serial.println("\n--- Initializing Smart Plant Monitoring System ---");

  pinMode(SOIL_PIN, INPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(IR_PIN, INPUT_PULLUP);     
  pinMode(TOUCH_PIN, INPUT_PULLDOWN); 

  turnPumpOff(); 
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("OLED Setup Failed. Check connections."));
    }
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Connecting Wi-Fi...");
  display.display();

  pinMode(DHT_PIN, INPUT);
  delay(50);
  dht.begin();

  WiFi.begin(ssid, password);
  int authTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && authTimeout < 10) {
    delay(500);
    Serial.print(".");
    authTimeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    isWiFiConnected = true;
    Serial.print("\nConnected! Open this IP in your browser: ");
    Serial.println(WiFi.localIP());
    server.on("/", handleRootPage);
    server.begin();
  } else {
    isWiFiConnected = false;
    Serial.println("\nWiFi Timeout. Running in offline automation mode. Background auto-healing active.");
  }
  
  lastIrState = digitalRead(IR_PIN);
  lastTouchState = digitalRead(TOUCH_PIN);
  dataScreenDisplayStartMillis = millis(); 
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - prevSensorMillis >= sensorInterval) {
    prevSensorMillis = currentMillis;
    readSensors();
    runAutomationRules();
    printSerialDebug();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!isWiFiConnected) {
      isWiFiConnected = true;
      server.begin();
      Serial.print("Wi-Fi Auto-Reconnected! Local IP: ");
      Serial.println(WiFi.localIP());
    }
    server.handleClient(); 
  } else {
    isWiFiConnected = false;
    if (currentMillis - prevWiFiMillis >= wifiRetryInterval) {
      prevWiFiMillis = currentMillis;
      Serial.println("Dashboard Disconnected. Re-polling access point in background...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }

  if (isWaterAlarmActive) {
    if (currentMillis - prevBuzzerMillis >= buzzerInterval) {
      prevBuzzerMillis = currentMillis;
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW); 
  }

  int currentIrState = digitalRead(IR_PIN);
  int currentTouchState = digitalRead(TOUCH_PIN);

  bool dataScreenHasShownLongEnough = (currentMillis - dataScreenDisplayStartMillis >= minimumDataDisplayDuration);

  bool irTriggered = (currentIrState == LOW && lastIrState == HIGH);
  bool touchTriggered = (currentTouchState == HIGH && lastTouchState == LOW);

  if ((irTriggered || touchTriggered) && !showingSmiley && dataScreenHasShownLongEnough) {
    showingSmiley = true;
    smileyStartMillis = currentMillis; 
    Serial.println(">> Human Proximity Detected! Animating OLED expressions.");
  }

  lastIrState = currentIrState;
  lastTouchState = currentTouchState;

  if (showingSmiley && (currentMillis - smileyStartMillis >= smileyDuration)) {
    showingSmiley = false; 
    dataScreenDisplayStartMillis = currentMillis; 
    Serial.println(">> Returning display window to environmental telemetry.");
  }

  display.clearDisplay();
  if (showingSmiley) {
    drawAnimatedEyes(); 
  } else {
    drawDataTelemetryScreen(); 
  }
  display.display();
}

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (isnan(t) || isnan(h)) {
    Serial.println("Warning: DHT11 returned NaN. Check wiring.");
  } else {
    temperature = t;
    humidity = h;
  }

  soilRaw = analogRead(SOIL_PIN);

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  tankDistance = (duration == 0) ? -1.0 : (duration * 0.034 / 2);
}

void runAutomationRules() {
  if (tankDistance == -1.0) { 
    waterStatusString = "SENSOR ERR"; 
    isWaterAlarmActive = false; 
  } 
  else if (tankDistance >= TANK_EMPTY_CM) { 
    waterStatusString = "TANK EMPTY!"; 
    isWaterAlarmActive = true; 
  } 
  else { 
    waterStatusString = "Tank OK"; 
    isWaterAlarmActive = false; 
  }

  if (soilRaw >= SOIL_DRY_THRESHOLD) { 
    turnPumpOn(); 
  } else if (soilRaw < SOIL_WET_THRESHOLD) { 
    turnPumpOff(); 
  }
}

void drawDataTelemetryScreen() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("--- SMART PLANT ---");
  
  display.print("Soil Moisture: "); display.println(soilRaw);
  
  display.print("Room Temp    : "); display.print(temperature, 1); display.println(" C");
  display.print("Room Humid   : "); display.print(humidity, 1); display.println(" %");
  
  if (tankDistance == -1.0) {
    display.println("Tank Level   : ERROR");
  } else {
    display.print("Tank Distance: "); display.print(tankDistance, 1); display.println(" cm");
  }
  
  display.println("--------------------");
  display.print("Pump Status  : "); display.println(isPumpRunning ? "RUNNING" : "STANDBY");
}

void drawAnimatedEyes() {
  display.fillTriangle(24, 30, 38, 14, 52, 30, WHITE);
  display.fillTriangle(76, 30, 90, 14, 104, 30, WHITE);
  display.fillTriangle(24, 34, 38, 18, 52, 34, BLACK);
  display.fillTriangle(76, 34, 90, 22, 104, 34, BLACK);
  display.drawRoundRect(49, 38, 30, 12, 6, WHITE);
  display.fillRect(47, 34, 34, 8, BLACK); 
}

void printSerialDebug() {
  Serial.print("Moisture: "); Serial.print(soilRaw);
  Serial.print(" | Temp: "); Serial.print(temperature);
  Serial.print(" C | Tank: "); Serial.print(tankDistance);
  Serial.print(" cm | Pump: "); Serial.println(isPumpRunning ? "ON" : "OFF");
}

void handleRootPage() {
  String html = "<!DOCTYPE html><html lang=\"en\"><head>";
  html += "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<meta http-equiv=\"refresh\" content=\"3\">"; 
  html += "<title>Smart Plant Dashboard</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #a8e063 0%, #56ab2f 100%); color: #333; margin: 0; padding: 20px; text-align: center; min-height: 100vh; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += ".card { background: rgba(255, 255, 255, 0.95); padding: 20px; margin: 20px 0; border-radius: 15px; box-shadow: 0 10px 20px rgba(0,0,0,0.15); transition: transform 0.2s ease-in-out; }";
  html += ".card:hover { transform: translateY(-5px); }";
  html += "h1 { color: white; font-size: 2.2em; text-shadow: 1px 2px 4px rgba(0,0,0,0.4); margin-bottom: 30px; }";
  html += "h3 { color: #2e7d32; border-bottom: 2px solid #e0e0e0; padding-bottom: 10px; margin-top: 0; }";
  html += ".data-row { display: flex; justify-content: space-between; align-items: center; padding: 10px 0; font-size: 1.1em; font-weight: 500; }";
  html += ".val { font-size: 1.3em; font-weight: bold; color: #1565c0; }";
  html += ".badge { padding: 6px 12px; border-radius: 20px; color: white; font-weight: bold; font-size: 0.9em; letter-spacing: 1px; }";
  html += ".bg-green { background-color: #4caf50; }";
  html += ".bg-red { background-color: #f44336; }";
  html += ".bg-gray { background-color: #9e9e9e; }";
  html += "</style></head><body>";
  
  html += "<div class=\"container\"><h1>Smart Plant Monitor</h1>";
  
  html += "<div class=\"card\"> <h3>Room Environment</h3>";
  html += " <div class=\"data-row\"><span>Temperature:</span> <span class=\"val\">" + String(temperature, 1) + " &deg;C</span></div>";
  html += " <div class=\"data-row\"><span>Humidity:</span> <span class=\"val\">" + String(humidity, 1) + " %</span></div>";
  html += "</div>";

  html += "<div class=\"card\"> <h3>Water Systems</h3>";
  html += " <div class=\"data-row\"><span>Soil Moisture (Raw Adc):</span> <span class=\"val\">" + String(soilRaw) + "</span></div>";
  String tankBadgeColor = isWaterAlarmActive ? "bg-red" : "bg-green";
  html += " <div class=\"data-row\"><span>Tank Status:</span> <div><span class=\"badge " + tankBadgeColor + "\">" + waterStatusString + "</span> <span style=\"font-size:0.9em; color:#666;\">(" + String(tankDistance, 1) + " cm)</span></div></div>";
  html += "</div>";

  html += "<div class=\"card\"> <h3>Actuator Status</h3>";
  String pumpBadgeColor = isPumpRunning ? "bg-green" : "bg-gray";
  String pumpText = isPumpRunning ? "RUNNING" : "STANDBY";
  html += " <div class=\"data-row\"><span>Water Pump Line:</span> <span class=\"badge " + pumpBadgeColor + "\">" + pumpText + "</span></div>";
  html += "</div>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}
