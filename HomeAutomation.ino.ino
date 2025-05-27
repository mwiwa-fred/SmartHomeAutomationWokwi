#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define DHTPIN 14
#define DHTTYPE DHT22
#define FAN_BUTTON 12
#define LIGHT_BUTTON 13
#define FAN_RELAY 27
#define LIGHT_RELAY 33
#define PIR_SENSOR 25
#define BUZZER 15
#define GAS_SENSOR 34
#define TEMP_PIN 4
#define LDR_PIN 35
#define DOOR_BUTTON 18
#define WATER_BUTTON 19

DHT dht(DHTPIN, DHTTYPE);

// States
bool fanState = false;
bool lightState = false;
bool motionDetected = false;
bool buzzerOn = false;
bool fanAutoOn = false;
bool isNight = false;
bool gasDetected = false;
bool simulatedGas = false;
bool doorOpen = false;
bool waterLeak = false;

unsigned long lastToggleTime = 0;
unsigned long motionStartTime = 0;
unsigned long lastGasToggleTime = 0;
unsigned long lastSensorDisplayTime = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(FAN_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_BUTTON, INPUT_PULLUP);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LIGHT_RELAY, OUTPUT);
  pinMode(PIR_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(GAS_SENSOR, INPUT);
  pinMode(DOOR_BUTTON, INPUT_PULLUP);
  pinMode(WATER_BUTTON, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  digitalWrite(FAN_RELAY, LOW);
  digitalWrite(LIGHT_RELAY, LOW);
  digitalWrite(BUZZER, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("System Ready");
  display.display();
  delay(2000);
}

void loop() {
  // Manual Fan Button
  if (digitalRead(FAN_BUTTON) == LOW && !fanAutoOn) {
    fanState = !fanState;
    digitalWrite(FAN_RELAY, fanState ? HIGH : LOW);
    Serial.println(fanState ? "Fan turned ON" : "Fan turned OFF");
    delay(200);
  }

  // Manual Light Button
  if (digitalRead(LIGHT_BUTTON) == LOW) {
    lightState = !lightState;
    digitalWrite(LIGHT_RELAY, lightState ? HIGH : LOW);
    Serial.println(lightState ? "Light turned ON" : "Light turned OFF");
    delay(200);
  }

  // Day/Night toggle
  if (millis() - lastToggleTime >= 10000) {
    lastToggleTime = millis();
    isNight = !isNight;
    if (isNight) {
      digitalWrite(LIGHT_RELAY, HIGH);
      lightState = true;
      Serial.println("Night detected – Light ON");
    } else {
      digitalWrite(LIGHT_RELAY, LOW);
      lightState = false;
      Serial.println("Daytime – Light OFF");
    }
  }

  // Motion detection
  if (digitalRead(PIR_SENSOR) == HIGH && !motionDetected) {
    motionDetected = true;
    motionStartTime = millis();
    digitalWrite(BUZZER, HIGH);
    buzzerOn = true;
    Serial.println("Motion detected!");
  }

  if (buzzerOn && millis() - motionStartTime >= 5000) {
    digitalWrite(BUZZER, LOW);
    buzzerOn = false;
    motionDetected = false;
  }

  // Simulate gas detection toggle
  if (millis() - lastGasToggleTime >= 8000) {
    lastGasToggleTime = millis();
    simulatedGas = !simulatedGas;
    gasDetected = simulatedGas;

    if (gasDetected) {
      Serial.println("Gas/Smoke Detected!");
      fanAutoOn = true;
      digitalWrite(FAN_RELAY, HIGH);
      fanState = true;
    } else {
      Serial.println("Gas/Smoke Cleared");
      fanAutoOn = false;
      fanState = false;
      digitalWrite(FAN_RELAY, LOW);
    }
  }

  // Read simulated temperature
  int analogValue = analogRead(TEMP_PIN);
  float temperature = map(analogValue, 0, 4095, 15, 40);

  if (temperature > 30) {
    fanAutoOn = true;
    fanState = true;
    digitalWrite(FAN_RELAY, HIGH);
  } else if (!gasDetected) {
    fanAutoOn = false;
    fanState = false;
    digitalWrite(FAN_RELAY, LOW);
  }

  // Read LDR, door, water leak every 5 seconds
  if (millis() - lastSensorDisplayTime >= 5000) {
    lastSensorDisplayTime = millis();

    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    doorOpen = digitalRead(DOOR_BUTTON) == LOW;
    Serial.println(doorOpen ? "Door/Window OPEN!" : "Door/Window CLOSED");

    waterLeak = digitalRead(WATER_BUTTON) == LOW;
    Serial.println(waterLeak ? "Water Leak Detected!" : "No Water Leak");
  }

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Home Automation");
  display.println("----------------");
  display.print("Temp: ");
  display.print(isnan(temperature) ? 0.0 : temperature);
  display.println(" C");

  display.print("Light: ");
  display.println(lightState ? "ON" : "OFF");

  display.print("Fan: ");
  display.println(fanState ? "ON" : "OFF");

  display.print("Motion: ");
  display.println(motionDetected ? "YES" : "NO");

  display.print("Gas: ");
  display.println(gasDetected ? "YES" : "NO");

  display.print("Door: ");
  display.println(doorOpen ? "OPEN" : "CLOSED");

  display.print("Water: ");
  display.println(waterLeak ? "YES" : "NO");

  display.display();
  delay(200);
}
