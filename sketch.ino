// Smart Room Monitor - Final Corrected Code
// Fixes: Active buzzer handling + inverted LDR readings

// ===== PINS & CONSTANTS =====
#define TEMP_SENSOR_PIN 2      // DHT22 data pin
#define LIGHT_SENSOR_PIN A0    // LDR analog pin
#define BUTTON_PIN 3           // Mode toggle button
#define LED_PIN 4              // LED indicator
#define BUZZER_PIN 5           // Buzzer control

// Thresholds (adjusted for inverted LDR)
const float tempThreshold = 30.0;  // Temperature alert threshold (°C)
const int lightThreshold = 200;    // Lower value = brighter light (now inverted)

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C        // I2C address for OLED

// Buzzer settings
#define BUZZER_FREQ 2000         // Frequency in Hz
#define BUZZER_DURATION 200      // Duration in ms

// ===== LIBRARIES =====
#include <DHT.h>                // For DHT22
#include <Wire.h>
#include <Adafruit_SSD1306.h>   // For OLED
#include <Adafruit_GFX.h>

// ===== GLOBAL VARIABLES =====
bool autoMode = true;           // Start in auto mode
float tempReadings[10];         // Circular buffer for temperature
int lightReadings[10];          // Circular buffer for light
byte currentIndex = 0;          // Buffer index

// Initialize devices
DHT dht(TEMP_SENSOR_PIN, DHT22);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  
  // Initialize sensors
  dht.begin();
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while(1); // Halt if display fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
  
  Serial.println("System initialized");
}

// ===== MAIN LOOP =====
void loop() {
  handleModeToggle();
  
  if (autoMode) {
    autoModeLogic();
  } else {
    manualModeLogic();
  }
  
  delay(1000); // Delay between readings
}

// ===== BUZZER FUNCTIONS ===== (NEW)
void triggerBuzzer() {
  tone(BUZZER_PIN, BUZZER_FREQ, BUZZER_DURATION); // Uses tone() instead of digitalWrite
}

void stopBuzzer() {
  noTone(BUZZER_PIN);
}

// ===== LDR CORRECTION (INVERTED READINGS) ===== (NEW)
int readCorrectedLight() {
  int raw = analogRead(LIGHT_SENSOR_PIN);
  return map(raw, 0, 1023, 1023, 0); // Inverts the reading
}

// ===== AUTO MODE ===== (UPDATED)
void autoModeLogic() {
  float temp = dht.readTemperature();
  int lightLevel = readCorrectedLight(); // Uses corrected light reading
  
  if (isnan(temp)) {
    Serial.println("Failed to read temperature!");
    return;
  }
  
  // Control outputs (updated for buzzer and inverted light)
  if (temp > tempThreshold) {
    triggerBuzzer(); // Uses new buzzer function
  } else {
    stopBuzzer();
  }
  
  digitalWrite(LED_PIN, lightLevel < lightThreshold ? HIGH : LOW);
  
  storeReading(temp, lightLevel);
  displayReadings(temp, lightLevel);
  
  Serial.print("Auto Mode - Temp: ");
  Serial.print(temp);
  Serial.print("°C, Light: ");
  Serial.println(lightLevel);
}

// ===== MANUAL MODE ===== (UPDATED)
void manualModeLogic() {
  if (Serial.available()) {
    char command = Serial.read();
    
    switch(command) {
      case 'l':
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        break;
        
      case 'b':
        triggerBuzzer(); // Uses new buzzer function
        break;
        
      case 'r':
        float temp = dht.readTemperature();
        int light = readCorrectedLight(); // Uses corrected light reading
        Serial.print("Manual Mode - Temp: ");
        Serial.print(temp);
        Serial.print("°C, Light: ");
        Serial.println(light);
        displayReadings(temp, light);
        break;
    }
  }
}

// ===== REST OF THE FUNCTIONS REMAIN THE SAME =====
void handleModeToggle() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      autoMode = !autoMode;
      Serial.print("Mode changed to: ");
      Serial.println(autoMode ? "Auto" : "Manual");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Mode: ");
      display.println(autoMode ? "Auto" : "Manual");
      display.display();
      while(digitalRead(BUTTON_PIN) == LOW);
    }
  }
}

void storeReading(float temp, int light) {
  tempReadings[currentIndex] = temp;
  lightReadings[currentIndex] = light;
  currentIndex = (currentIndex + 1) % 10;
}

void displayReadings(float temp, int light) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temp); display.print("°C");
  display.setCursor(0, 10);
  display.print("Light: "); display.print(light);
  display.setCursor(0, 20);
  display.print("Mode: "); display.print(autoMode ? "Auto" : "Manual");
  display.display();
}