#include <Wire.h>
#include <MPU6050.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ----- Pin Definitions -----
#define DHTPIN 15
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 34
#define VIBRATION_PIN 35
#define BUZZER_PIN 13
#define RED_LED 25
#define YELLOW_LED 26
#define GREEN_LED 27
#define BUTTON_PIN 12  // Buzzer silence button

// ----- WiFi & API Keys -----
const char* ssid = "<SSID_HERE>";
const char* password = "<PASSWORD_HERE>";
const String THINGSPEAK_API_KEY = "<THINGSPEAK_API_KEY_HERE>";

// Telegram Bot credentials
const char* TELEGRAM_BOT_TOKEN = "<ADD_TOKEN_HERE>";
const char* CHAT_ID = "<CHAT_ID_HERE>";

// ----- Sensor Objects -----
MPU6050 mpu;
DHT dht(DHTPIN, DHTTYPE);

// ----- Thresholds -----
const int MOISTURE_THRESHOLD = 2500;
const float TEMP_HIGH = 30.0;
const float HUMIDITY_HIGH = 70.0;
const float TILT_ANGLE_THRESHOLD = 70.0;

unsigned long lastUpload = 0;
const unsigned long uploadInterval = 20000;
unsigned long lastBuzzerTime = 0;
bool buzzerSilenced = false;

// ----- Setup -----
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  mpu.initialize();
  dht.begin();

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  connectWiFi();
  Serial.println("System initialized.");
}

// ----- Main Loop -----
void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  bool vibrationDetected = digitalRead(VIBRATION_PIN) == HIGH;

  // MPU6050 Acceleration
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float tiltX = atan2(ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180 / PI; // Convert to Precentage
  float tiltY = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180 / PI;

  // Conditions
  bool landslideRisk = abs(tiltX) < TILT_ANGLE_THRESHOLD;
  bool earthquakeRisk = vibrationDetected;
  bool environmentalRisk = (temp >= TEMP_HIGH || humidity >= HUMIDITY_HIGH || soilMoisture >= MOISTURE_THRESHOLD);

  // Buzzer silence button
  if (digitalRead(BUTTON_PIN) == LOW) {
    buzzerSilenced = true;
    Serial.println("🔇 Buzzer silenced manually.");
  }

  // LED & Buzzer logic
  if (landslideRisk || earthquakeRisk) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    if (!buzzerSilenced) buzzerPattern(1000, 500); // High Pitch 0.5s
    sendTelegramMessage("⚠️ EARLY WARNING: Landslide or Earthquake Detected!");
  } else if (environmentalRisk) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    if (!buzzerSilenced) buzzerPattern(500, 1000); // Low Pitch 1s
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER_PIN);
    buzzerSilenced = false;  // Reset for next time
  }

  // Serial Monitor Output
  Serial.println("===== SENSOR STATUS =====");
  Serial.printf("Temperature: %.1f °C\n", temp);
  Serial.printf("Humidity: %.1f %%\n", humidity);
  Serial.printf("Soil Moisture: %d\n", soilMoisture);
  Serial.printf("TiltX: %.2f ° | TiltY: %.2f °\n", tiltX, tiltY);
  Serial.printf("Vibration: %s\n", earthquakeRisk ? "Detected" : "None");

  if (landslideRisk) Serial.println("⚠️ WARNING: Landslide Risk");
  if (earthquakeRisk) Serial.println("⚠️ WARNING: Earthquake Vibration");
  if (environmentalRisk) Serial.println("⚠️ Environmental Warning");
  if (!landslideRisk && !earthquakeRisk && !environmentalRisk)
    Serial.println("✅ All systems normal.");

  Serial.println("==========================\n");

  // Upload to ThingSpeak
  if (millis() - lastUpload > uploadInterval) {
    float tiltAverage = (abs(tiltX) + abs(tiltY)) / 2.0;
    sendToThingSpeak(temp, humidity, soilMoisture, tiltAverage, earthquakeRisk);
    lastUpload = millis();
  }

  delay(200);
}

// ----- WiFi -----
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");
}

// ----- ThingSpeak Upload -----
void sendToThingSpeak(float temp, float humidity, int moisture, float tilt, bool earthquake) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.thingspeak.com/update?api_key=" + THINGSPEAK_API_KEY +
                 "&field1=" + String(temp) +
                 "&field2=" + String(humidity) +
                 "&field3=" + String(moisture) +
                 "&field4=" + String(tilt) +
                 "&field5=" + String(earthquake ? 1 : 0);

    http.begin(url);
    int code = http.GET();
    if (code > 0) {
      String response = http.getString();
      Serial.print("📤 ThingSpeak: OK → Response: ");
      Serial.println(response);
    } else {
      Serial.print("❌ ThingSpeak Failed → Code: ");
      Serial.println(code);
    }
    http.end();
  }
}

// ----- Telegram Bot Alert -----
void sendTelegramMessage(String message) {
  static unsigned long lastSent = 0;
  const unsigned long cooldown = 10000; // Message Delay
  if (millis() - lastSent < cooldown) return;

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Skip cert validation
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
  String payload = "{\"chat_id\":\"" + String(CHAT_ID) + "\",\"text\":\"" + message + "\"}";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(payload);
  if (code > 0) Serial.println("📨 Telegram Sent");
  else Serial.println("❌ Telegram Failed");
  http.end();
  lastSent = millis();
}

// ----- Buzzer Patterns -----
void buzzerPattern(int frequency, int interval) {
  static unsigned long lastTone = 0;
  static bool toneOn = false;

  if (millis() - lastTone >= interval) {
    lastTone = millis();
    toneOn = !toneOn;
    if (toneOn)
      tone(BUZZER_PIN, frequency);
    else
      noTone(BUZZER_PIN);
  }
}
