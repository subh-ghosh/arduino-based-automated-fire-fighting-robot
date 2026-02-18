#include <Servo.h>

const int motor_1 = 4;
const int motor_2 = 5;
const int motor_3 = 6;
const int motor_4 = 7;

const int left_sensor = A0; 
const int middle_sensor = A1; 
const int right_sensor = A2; 
const int servo_pin = 8;
const int water_pump = 9;
const int buzzer = 12;

Servo myservo;
int pos = 0;

const int flame_low_threshold = 30;
const int flame_high_threshold = 400;

bool carStopped = false;
bool audioSignalGiven = false;

unsigned long previousMillis = 0;
const long buzzerInterval = 350; // Toggle buzzer every 300ms

void setup() {
    pinMode(water_pump, OUTPUT);
    pinMode(motor_1, OUTPUT);
    pinMode(motor_2, OUTPUT);
    pinMode(motor_3, OUTPUT);
    pinMode(motor_4, OUTPUT);
    pinMode(buzzer, OUTPUT);


    digitalWrite(motor_1, LOW);
    digitalWrite(motor_2, LOW);
    digitalWrite(motor_3, LOW);
    digitalWrite(motor_4, LOW);
    digitalWrite(water_pump, HIGH); // Ensure pump is off at start
    digitalWrite(buzzer, LOW);   

    Serial.begin(9600);
    myservo.attach(servo_pin); 
    myservo.write(90);
}

void loop() {
    int left_value = analogRead(left_sensor);
    int middle_value = analogRead(middle_sensor);
    int right_value = analogRead(right_sensor);

    Serial.print("Sensor Readings: L=");
    Serial.print(left_value);
    Serial.print(" M=");
    Serial.print(middle_value);
    Serial.print(" R=");
    Serial.print(right_value);

    // Buzzer Alarm Toggle using millis()
    unsigned long currentMillis = millis();
    if (audioSignalGiven) {
        if (currentMillis - previousMillis >= buzzerInterval) {
            previousMillis = currentMillis;
            digitalWrite(buzzer, !digitalRead(buzzer)); // Toggle buzzer ON/OFF
            Serial.println(digitalRead(buzzer) ? "Buzzer ON" : "Buzzer OFF");
        }
    }

    if ((left_value < flame_high_threshold || middle_value < flame_high_threshold || right_value < flame_high_threshold) && !audioSignalGiven) {
        audioSignalGiven = true; // Start alarm
        Serial.println("Buzzer activated");
    }

    if (middle_value > flame_high_threshold && left_value > flame_high_threshold && right_value > flame_high_threshold && !carStopped) {
        stopCar();
        Serial.println("Car stopped. No Fire.");
    }
    else if (middle_value <= flame_low_threshold && !carStopped) {
        stopCar();
        Serial.println("Car stopped near fire");
        carStopped = true;
        extinguishFire();
    }
    else if (left_value < flame_high_threshold && left_value < middle_value && !carStopped) {
        turnLeftToFire();
        Serial.println("Car moving left towards fire");
    }
    else if (right_value < flame_high_threshold && right_value < middle_value && !carStopped) {
        turnRightToFire();
        Serial.println("Car moving right towards fire");
    }
    else if (middle_value > flame_low_threshold && middle_value < flame_high_threshold &&
    middle_value < left_value && middle_value < right_value && !carStopped) {
        moveForwardToFire();
        Serial.println("Car moving forward towards fire");
    }

    carStopped = false;
}

void moveForwardToFire() {
    Serial.println("Moving forward towards fire...");
    digitalWrite(motor_1, HIGH);
    digitalWrite(motor_2, LOW);
    digitalWrite(motor_3, LOW);
    digitalWrite(motor_4, HIGH);
}

void turnLeftToFire() {
    Serial.println("Turning left towards fire...");
    digitalWrite(motor_1, LOW);
    digitalWrite(motor_2, HIGH);
    digitalWrite(motor_3, LOW);
    digitalWrite(motor_4, HIGH);
    delay(150);
    stopCar();
}

void turnRightToFire() {
    Serial.println("Turning right towards fire...");
    digitalWrite(motor_1, HIGH);
    digitalWrite(motor_2, LOW);
    digitalWrite(motor_3, HIGH);
    digitalWrite(motor_4, LOW);
    delay(150);
    stopCar();
}

void stopCar() {
    digitalWrite(motor_1, LOW);
    digitalWrite(motor_2, LOW);
    digitalWrite(motor_3, LOW);
    digitalWrite(motor_4, LOW);
    Serial.println("Stopping the car...");
    delay(700); 
}

void extinguishFire() {
    digitalWrite(buzzer, LOW);
    audioSignalGiven = false;
    digitalWrite(water_pump, LOW);
    Serial.println("Extinguishing fire");
    delay(15);
    //myservo.attach(servo_pin); 
    myservo.write(60); 

    for (int j = 0; j < 4; j++) {
        for (pos = 60; pos <= 120; pos += 1) {
            myservo.write(pos);
            delay(15);
        }
        for (pos = 120; pos >= 60; pos -= 1) {
            myservo.write(pos);
            delay(15);
        }
    }

    //myservo.detach();
    digitalWrite(water_pump, HIGH);
    Serial.println("Fire extinguished");
}
