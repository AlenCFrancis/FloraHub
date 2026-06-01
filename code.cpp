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

bool showingExpression = false;
int expressionType = 1; 
unsigned long expressionStartMillis = 0;
const unsigned long expressionDuration = 5000; 

unsigned long dataScreenDisplayStartMillis = 0;
const unsigned long minimumDataDisplayDuration = 3000; 

unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 200; 

int lastIrState = HIGH;
int lastTouchState = LOW;

void handleRootPage();
void readSensors();
void runAutomationRules();
void printSerialDebug();
void drawDataTelemetryScreen();
void drawAnimatedEyes();
void drawAmazedEyes();

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
  pinMode(TOUCH_PIN, INPUT); 

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
    Serial.println("\nWiFi Timeout. Offline mode active.");
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
    }
    server.handleClient(); 
  } else {
    isWiFiConnected = false;
    if (currentMillis - prevWiFiMillis >= wifiRetryInterval) {
      prevWiFiMillis = currentMillis;
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
  bool touchTriggered = false;

  if (currentTouchState == HIGH && lastTouchState == LOW) {
    if (currentMillis - lastTouchTime >= debounceDelay) {
      touchTriggered = true;
      lastTouchTime = currentMillis; 
    }
  }

  lastIrState = currentIrState;
  lastTouchState = currentTouchState;

  if (irTriggered) {
    if (!showingExpression && dataScreenHasShownLongEnough) {
      showingExpression = true;
      expressionType = 1; 
      expressionStartMillis = currentMillis; 
    } else if (showingExpression) {
      expressionType = 1;
      expressionStartMillis = currentMillis; 
    }
  }

  if (touchTriggered) {
    if (!showingExpression && dataScreenHasShownLongEnough) {
      showingExpression = true;
      expressionType = 2; 
      expressionStartMillis = currentMillis; 
      Serial.println(">> Touch Sensor Triggered: AMAZED Face");
    } else if (showingExpression) {
      expressionType = 2;
      expressionStartMillis = currentMillis; 
      Serial.println(">> Touch Sensor Refreshed: AMAZED Face");
    }
  }

  if (showingExpression && (currentMillis - expressionStartMillis >= expressionDuration)) {
    showingExpression = false; 
    dataScreenDisplayStartMillis = currentMillis; 
  }

  display.clearDisplay();
  if (showingExpression) {
    if (expressionType == 1) {
      drawAnimatedEyes(); 
    } else {
      drawAmazedEyes();   
    }
  } else {
    drawDataTelemetryScreen(); 
  }
  display.display();
}

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
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

void drawAmazedEyes() {
  display.drawCircle(38, 24, 11, WHITE);
  display.fillCircle(38, 24, 3, WHITE); 
  display.drawCircle(90, 24, 11, WHITE);
  display.fillCircle(90, 24, 3, WHITE); 
  display.drawCircle(64, 45, 7, WHITE);  
}

void printSerialDebug() {
  Serial.print("Moisture: "); Serial.print(soilRaw);
  Serial.print(" | Temp: "); Serial.print(temperature);
  Serial.print(" C | Tank: "); Serial.print(tankDistance);
  Serial.print(" cm | Pump: "); Serial.println(isPumpRunning ? "ON" : "OFF");
}

void handleRootPage() {
  int soilPercent = map(soilRaw, 4095, 1000, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
  String tankBadgeClass = isWaterAlarmActive ? "bg-error-container text-error border-error" : "bg-primary-container text-on-primary-container border-primary";
  String tankDistanceString = (tankDistance == -1.0) ? "ERROR" : String(tankDistance, 1) + " cm";
  String pumpBadgeClass = isPumpRunning ? "bg-primary text-on-primary border-primary font-bold" : "bg-surface-container-highest text-on-surface border-outline-variant";
  String pumpText = isPumpRunning ? "RUNNING" : "STANDBY";

  String html = R"=====(
<!DOCTYPE html>
<html class="dark" lang="en"><head>
<meta charset="utf-8"/>
<meta content="width=device-width, initial-scale=1.0" name="viewport"/>
<meta http-equiv="refresh" content="3"/>
<title>FloraHub Dashboard</title>
<script src="https://cdn.tailwindcss.com?plugins=forms,container-queries"></script>
<link href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:wght,FILL@100..700,0..1&display=swap" rel="stylesheet"/>
<link href="https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;500;700&family=Syne:wght@400;600;700;800&family=Work+Sans:wght@400;500;600&display=swap" rel="stylesheet"/>
<script id="tailwind-config">
  tailwind.config = {
    darkMode: "class",
    theme: {
      extend: {
        "colors": {
                "tertiary-fixed-dim": "#eec200",
                "on-surface-variant": "#bccabb",
                "inverse-primary": "#006d36",
                "surface-variant": "#313632",
                "surface": "#0f1511",
                "on-secondary-container": "#b0bbb2",
                "outline-variant": "#3d4a3e",
                "primary-fixed-dim": "#4de082",
                "on-primary-container": "#005e2d",
                "surface-container-lowest": "#0a0f0c",
                "on-tertiary": "#3c2f00",
                "surface-bright": "#353a37",
                "on-error": "#690005",
                "outline": "#869486",
                "surface-container-highest": "#313632",
                "tertiary-container": "#ebbf00",
                "on-primary-fixed-variant": "#005227",
                "secondary-container": "#414c45",
                "surface-dim": "#0f1511",
                "on-secondary-fixed-variant": "#3f4942",
                "on-surface": "#dfe4de",
                "secondary": "#bec9c0",
                "on-tertiary-fixed": "#231b00",
                "on-secondary": "#29332c",
                "primary": "#6bfb9a",
                "primary-fixed": "#6dfe9c",
                "tertiary-fixed": "#ffe083",
                "tertiary": "#ffdd75",
                "on-primary-fixed": "#00210c",
                "surface-container-low": "#181d19",
                "error": "#ffb4ab",
                "primary-container": "#4ade80",
                "on-primary": "#003919",
                "error-container": "#93000a",
                "on-tertiary-fixed-variant": "#574500",
                "secondary-fixed": "#dae5dc",
                "surface-container-high": "#262b28",
                "on-secondary-fixed": "#141e18",
                "on-tertiary-container": "#624f00",
                "on-error-container": "#ffdad6",
                "on-background": "#dfe4de",
                "surface-tint": "#4de082",
                "inverse-surface": "#dfe4de",
                "inverse-on-surface": "#2c322e",
                "secondary-fixed-dim": "#bec9c0",
                "background": "#0f1511",
                "surface-container": "#1c211d"
        },
        "borderRadius": { "DEFAULT": "0.125rem", "lg": "0.25rem", "xl": "0.5rem", "full": "0.75rem" },
        "spacing": { "gutter": "24px", "unit": "4px", "margin-desktop": "64px", "margin-mobile": "16px", "max-width": "1280px" },
        "fontFamily": { "body-md": ["Work Sans"], "label-sm": ["Space Mono"], "display-lg": ["Syne"], "headline-lg-mobile": ["Syne"], "headline-lg": ["Syne"] },
        "fontSize": { "body-md": ["16px", {"lineHeight": "1.6", "fontWeight": "400"}], "label-sm": ["12px", {"lineHeight": "1.2", "letterSpacing": "0.05em", "fontWeight": "500"}], "display-lg": ["56px", {"lineHeight": "1.1", "letterSpacing": "-0.02em", "fontWeight": "800"}], "headline-lg-mobile": ["28px", {"lineHeight": "1.2", "fontWeight": "700"}], "headline-lg": ["32px", {"lineHeight": "1.2", "fontWeight": "700"}] }
      }
    }
  }
</script>
<style>
        .material-symbols-outlined { font-variation-settings: 'FILL' 0, 'wght' 400, 'GRAD' 0, 'opsz' 24; }
        .material-symbols-outlined[data-weight="fill"] { font-variation-settings: 'FILL' 1; }
        .neu-card { box-shadow: 5px 5px 10px #0a0f0c, -5px -5px 10px #181d19; transition: all 0.3s ease; }
        .neu-card:hover { box-shadow: none; border: 3px solid #6bfb9a; }
        .neu-inset { box-shadow: inset 5px 5px 10px #0a0f0c, inset -5px -5px 10px #181d19; }
</style>
</head>
<body class="bg-background text-on-background font-body-md min-h-screen flex flex-col items-center">
<main class="flex-1 p-margin-mobile md:p-margin-desktop w-full max-w-max-width flex flex-col gap-gutter">
<header class="w-full py-8 text-center">
<h1 class="text-display-lg font-display-lg text-primary tracking-tight">FloraHub</h1>
</header>
<div class="grid grid-cols-1 lg:grid-cols-12 gap-gutter">
<div class="lg:col-span-7 bg-surface-container rounded-xl p-[32px] flex flex-col items-center justify-center neu-card relative overflow-hidden min-h-[400px]">
<div class="absolute top-4 right-4 flex gap-2">
<span class="bg-secondary-container text-on-secondary-container px-3 py-1 rounded-full text-label-sm font-label-sm flex items-center gap-1 border border-outline-variant">
<span class="material-symbols-outlined text-sm" data-icon="favorite" data-weight="fill">favorite</span>System Active</span>
</div>
<img alt="Monstera plant in a smart pot" class="w-full max-w-[300px] object-contain drop-shadow-2xl z-10" src="https://lh3.googleusercontent.com/aida/ADBb0uh51BPAtUFpEwINQnSqy9nylVt-XIAHBKtCi-8DlmO7soAP2yJ6ucTdxR9J_IiH04Fmdjrjlm2nIqeNndDFR3qkglgytAyni51lzd-aJI9sBm-faOePb7HJmzfAMd5ydN_0qKdvH4bUAMve1S0rqd9sjLiAcncezodcaNewEP1AP2_jWbOUSEm-vUjiDzT-x861zceAm0PFw-j_9oVc-K6y1AVTYsRy6w2QXrqAlKI4IMceHaReex9TWA"/>
</div>
<div class="lg:col-span-5 flex flex-col gap-gutter">
<div class="bg-surface rounded-xl p-[32px] neu-card flex-1 flex flex-col border border-transparent">
<div class="flex items-center gap-2 mb-6 border-b-2 border-primary-fixed-dim pb-2">
<span class="material-symbols-outlined text-primary" data-icon="thermometer">thermometer</span>
<h2 class="text-headline-lg-mobile font-headline-lg-mobile text-primary">Room Environment</h2>
</div>
<div class="space-y-4 flex-1 justify-center flex flex-col">
<div class="flex justify-between items-center bg-surface-container-low p-4 rounded-lg neu-inset">
<span class="text-body-md font-body-md text-on-surface">Temperature</span>
<span class="text-headline-lg-mobile font-headline-lg-mobile text-primary-fixed">%TEMPERATURE% °C</span>
</div>
<div class="flex justify-between items-center bg-surface-container-low p-4 rounded-lg neu-inset">
<span class="text-body-md font-body-md text-on-surface">Humidity</span>
<span class="text-headline-lg-mobile font-headline-lg-mobile text-primary-fixed">%HUMIDITY% %</span>
</div>
</div>
</div>
<div class="bg-surface rounded-xl p-[32px] neu-card flex-1 flex flex-col border border-transparent">
<div class="flex items-center gap-2 mb-6 border-b-2 border-primary-fixed-dim pb-2">
<span class="material-symbols-outlined text-primary" data-icon="grass">grass</span>
<h2 class="text-headline-lg-mobile font-headline-lg-mobile text-primary">Soil Status</h2>
</div>
<div class="flex justify-between items-center bg-surface-container-low p-6 rounded-lg neu-inset mt-auto">
<div class="flex flex-col">
<span class="text-body-md font-body-md text-on-surface">Moisture Level</span>
<span class="text-label-sm font-label-sm text-outline">(Raw ADC: %SOILRAW%)</span>
</div>
<span class="text-headline-lg font-headline-lg text-primary-fixed">%SOILPERCENT%%</span>
</div>
</div>
</div>
</div>
<div class="grid grid-cols-1 md:grid-cols-2 gap-gutter mt-4">
<div class="bg-surface rounded-xl p-[32px] neu-card border border-transparent">
<div class="flex items-center gap-2 mb-6 border-b-2 border-primary-fixed-dim pb-2">
<span class="material-symbols-outlined text-primary" data-icon="water_drop">water_drop</span>
<h2 class="text-headline-lg-mobile font-headline-lg-mobile text-primary">Watering System</h2>
</div>
<div class="flex justify-between items-center py-4">
<span class="text-body-md font-body-md text-on-surface">Tank Status:</span>
<div class="flex items-center gap-3">
<span class="px-3 py-1 rounded-full text-label-sm font-label-sm border %TANKBADGECLASS%">%TANKSTATUS%</span>
<span class="text-body-md font-body-md text-on-surface-variant">(%TANKDISTANCE%)</span>
</div>
</div>
</div>
<div class="bg-surface-container rounded-xl p-[32px] border-4 border-primary shadow-[5px_5px_0px_#4ade80]">
<div class="flex items-center gap-2 mb-6 border-b-2 border-primary pb-2">
<span class="material-symbols-outlined text-primary" data-icon="settings">settings</span>
<h2 class="text-headline-lg-mobile font-headline-lg-mobile text-primary">Actuator Status</h2>
</div>
<div class="flex justify-between items-center py-2">
<span class="text-body-md font-body-md text-on-surface font-semibold">Water Pump Line:</span>
<span class="px-4 py-2 rounded-full text-label-sm font-label-sm tracking-widest uppercase border-2 %PUMPBADGECLASS%">%PUMPTEXT%</span>
</div>
</div>
</div>
</main>
</body></html>
)=====";

  html.replace("%TEMPERATURE%", String(temperature, 1));
  html.replace("%HUMIDITY%", String(humidity, 1));
  html.replace("%SOILRAW%", String(soilRaw));
  html.replace("%SOILPERCENT%", String(soilPercent));
  html.replace("%TANKBADGECLASS%", tankBadgeClass);
  html.replace("%TANKSTATUS%", waterStatusString);
  html.replace("%TANKDISTANCE%", tankDistanceString);
  html.replace("%PUMPBADGECLASS%", pumpBadgeClass);
  html.replace("%PUMPTEXT%", pumpText);

  server.send(200, "text/html", html);
}
