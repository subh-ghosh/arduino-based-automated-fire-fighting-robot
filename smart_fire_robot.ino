#define BLYNK_PRINT Serial

/* 🔥🔥🔥 FILL YOUR BLYNK CREDENTIALS HERE 🔥🔥🔥
   (Copy these from the Blynk Website -> Device Info tab)
*/
#define BLYNK_TEMPLATE_ID           "TMPLxxxxxx" 
#define BLYNK_TEMPLATE_NAME         "FIRE ALERT 1"
#define BLYNK_AUTH_TOKEN            "YourAuthTokenHere"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h> 
#include <math.h>

// ==========================================
// 📶 1. IOT & NETWORK CREDENTIALS
// ==========================================
char ssid[] = "Wokwi-GUEST"; // Standard Wokwi WiFi
char pass[] = "";            // No password for Wokwi

// ==========================================
// 🧠 2. NAIVE BAYES BRAIN (YOUR NEW VALUES)
// ==========================================
// These are the EXACT values from your Python output

// Class: NO_FIRE
const float mean_NO_FIRE_L = 909.08; const float stdev_NO_FIRE_L = 57.66;
const float mean_NO_FIRE_C = 912.90; const float stdev_NO_FIRE_C = 63.88;
const float mean_NO_FIRE_R = 903.52; const float stdev_NO_FIRE_R = 69.48;

// Class: FIRE_LEFT
const float mean_FIRE_LEFT_L = 213.68; const float stdev_FIRE_LEFT_L = 95.67;
const float mean_FIRE_LEFT_C = 787.20; const float stdev_FIRE_LEFT_C = 118.89;
const float mean_FIRE_LEFT_R = 795.20; const float stdev_FIRE_LEFT_R = 131.44;

// Class: FIRE_CENTER
const float mean_FIRE_CENTER_L = 829.60; const float stdev_FIRE_CENTER_L = 119.77;
const float mean_FIRE_CENTER_C = 210.56; const float stdev_FIRE_CENTER_C = 106.90;
const float mean_FIRE_CENTER_R = 847.82; const float stdev_FIRE_CENTER_R = 117.98;

// Class: FIRE_RIGHT
const float mean_FIRE_RIGHT_L = 808.26; const float stdev_FIRE_RIGHT_L = 124.15;
const float mean_FIRE_RIGHT_C = 788.10; const float stdev_FIRE_RIGHT_C = 123.48;
const float mean_FIRE_RIGHT_R = 213.26; const float stdev_FIRE_RIGHT_R = 104.96;

// ==========================================
// 🛠️ 3. HARDWARE PINS (ESP32)
// ==========================================
const int motor_1 = 18; // Motor Left A
const int motor_2 = 5;  // Motor Left B
const int motor_3 = 17; // Motor Right A
const int motor_4 = 16; // Motor Right B

const int left_sensor = 34;   // Analog Input only
const int middle_sensor = 35; // Analog Input only
const int right_sensor = 32;  // Analog Input only

const int servo_pin = 26;
const int water_pump = 27;
const int buzzer = 14;

Servo myservo;
int pos = 0;

// Logic Variables
bool fireDetectedState = false;
unsigned long previousMillis = 0;
const long interval = 500; // IoT Update Interval

// ==========================================
// 🧮 4. MATH HELPERS (Classification + Regression)
// ==========================================

// Function A: Gaussian Probability (Classification)
float get_gaussian_prob(float x, float mean, float stdev) {
    if (stdev == 0) return 0;
    float exponent = exp(-0.5 * pow((x - mean) / stdev, 2));
    return (1.0 / (stdev * sqrt(2.0 * PI))) * exponent;
}

// Function B: Inverse Regression (Distance Estimation)
float predict_distance_cm(int sensor_val) {
    if (sensor_val <= 0) return 999.0;
    // Model: y = m/x (Calibrated constant 25000)
    float dist = 25000.0 / sensor_val;
    if (dist > 100) return 100.0; // Cap at 1m
    return dist;
}

void setup() {
    Serial.begin(115200);

    // Hardware Init
    pinMode(water_pump, OUTPUT);
    pinMode(motor_1, OUTPUT); pinMode(motor_2, OUTPUT);
    pinMode(motor_3, OUTPUT); pinMode(motor_4, OUTPUT);
    pinMode(buzzer, OUTPUT);
    
    digitalWrite(water_pump, HIGH); // Off
    digitalWrite(buzzer, LOW);

    myservo.attach(servo_pin);
    myservo.write(90);

    // Connect to Blynk
    Serial.print("Connecting to WiFi...");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("\n✅ Connected to Blynk");
    Blynk.logEvent("fire_alert", "🚨 AI Fire Robot ONLINE");
}

void loop() {
    Blynk.run();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        runAIAndIoT();
    }
}

void runAIAndIoT() {
    // 1. Read Sensors
    int raw_L = analogRead(left_sensor);
    int raw_C = analogRead(middle_sensor);
    int raw_R = analogRead(right_sensor);

    // Map for compatibility (ESP32 is 12-bit, training was 10-bit)
    int map_L = map(raw_L, 0, 4095, 0, 1023);
    int map_C = map(raw_C, 0, 4095, 0, 1023);
    int map_R = map(raw_R, 0, 4095, 0, 1023);

    // 2. Upload Raw Data to Blynk (Graphs)
    Blynk.virtualWrite(V0, map_C);
    Blynk.virtualWrite(V1, map_L);
    Blynk.virtualWrite(V2, map_R);

    // 3. AI Calculation (Naive Bayes)
    float p_no_fire = get_gaussian_prob(map_L, mean_NO_FIRE_L, stdev_NO_FIRE_L) *
                      get_gaussian_prob(map_C, mean_NO_FIRE_C, stdev_NO_FIRE_C) *
                      get_gaussian_prob(map_R, mean_NO_FIRE_R, stdev_NO_FIRE_R);

    float p_fire_left = get_gaussian_prob(map_L, mean_FIRE_LEFT_L, stdev_FIRE_LEFT_L) *
                        get_gaussian_prob(map_C, mean_FIRE_LEFT_C, stdev_FIRE_LEFT_C) *
                        get_gaussian_prob(map_R, mean_FIRE_LEFT_R, stdev_FIRE_LEFT_R);

    float p_fire_center = get_gaussian_prob(map_L, mean_FIRE_CENTER_L, stdev_FIRE_CENTER_L) *
                          get_gaussian_prob(map_C, mean_FIRE_CENTER_C, stdev_FIRE_CENTER_C) *
                          get_gaussian_prob(map_R, mean_FIRE_CENTER_R, stdev_FIRE_CENTER_R);

    float p_fire_right = get_gaussian_prob(map_L, mean_FIRE_RIGHT_L, stdev_FIRE_RIGHT_L) *
                         get_gaussian_prob(map_C, mean_FIRE_RIGHT_C, stdev_FIRE_RIGHT_C) *
                         get_gaussian_prob(map_R, mean_FIRE_RIGHT_R, stdev_FIRE_RIGHT_R);

    // 4. Decision
    int best_class = 0;
    float max_prob = p_no_fire;

    if (p_fire_left > max_prob) { max_prob = p_fire_left; best_class = 1; }
    if (p_fire_center > max_prob) { max_prob = p_fire_center; best_class = 2; }
    if (p_fire_right > max_prob) { max_prob = p_fire_right; best_class = 3; }

    // 5. IoT Event Logging (Only notify once per event to avoid spam)
    if (best_class != 0) {
        if (!fireDetectedState) {
            Blynk.logEvent("fire_alert", "🔥 FIRE DETECTED! Deploying countermeasures.");
            fireDetectedState = true;
        }
        digitalWrite(buzzer, HIGH); // Alarm ON
    } else {
        if (fireDetectedState) {
            Blynk.logEvent("fire_alert", "✅ Area Secured. Fire cleared.");
            fireDetectedState = false;
        }
        digitalWrite(buzzer, LOW); // Alarm OFF
    }

    // 6. Actuators
    stopCar(); // Reset state
    switch (best_class) {
        case 0: // Safe
            Serial.println("Status: Patrolling (Safe)");
            break;
        case 1: // Left
            Serial.println("Status: 🔥 Fire LEFT");
            digitalWrite(motor_1, LOW); digitalWrite(motor_2, HIGH);
            digitalWrite(motor_3, LOW); digitalWrite(motor_4, HIGH);
            break;
        case 2: // Center
            // REGRESSION LOGIC: Check Distance
            {
                float dist_cm = predict_distance_cm(map_C);
                Serial.print("Status: 🔥 Fire AHEAD | Dist: ");
                Serial.print(dist_cm); Serial.println("cm");
                Blynk.virtualWrite(V3, dist_cm); // Optional: Graph distance on V3

                if (dist_cm < 15.0) { // If closer than 15cm
                    extinguishFire(); 
                } else {
                    // Move forward
                    digitalWrite(motor_1, HIGH); digitalWrite(motor_2, LOW);
                    digitalWrite(motor_3, LOW); digitalWrite(motor_4, HIGH);
                }
            }
            break;
        case 3: // Right
            Serial.println("Status: 🔥 Fire RIGHT");
            digitalWrite(motor_1, HIGH); digitalWrite(motor_2, LOW);
            digitalWrite(motor_3, HIGH); digitalWrite(motor_4, LOW);
            break;
    }
}

void stopCar() {
    digitalWrite(motor_1, LOW); digitalWrite(motor_2, LOW);
    digitalWrite(motor_3, LOW); digitalWrite(motor_4, LOW);
}

void extinguishFire() {
    Serial.println("ACTION: 💦 PUMP ACTIVE");
    stopCar();
    digitalWrite(water_pump, LOW); // Relay ON
    
    // Servo Sweep
    for (pos = 60; pos <= 120; pos += 5) { myservo.write(pos); delay(20); }
    for (pos = 120; pos >= 60; pos -= 5) { myservo.write(pos); delay(20); }
    
    digitalWrite(water_pump, HIGH); // Relay OFF
    Blynk.logEvent("fire_alert", "💦 Extinguishing Sequence Complete");
}