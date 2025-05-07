// -- UNCHECKED CODE --

// Raspberry Pi Pico W - Controller and Receiver using ESP-NOW
#include <WiFi.h>
#include <esp_now.h>
#include <Servo.h>

// Controller or Receiver
#define ROLE_CONTROLLER 1
#define ROLE_RECEIVER 2
#define ROLE ROLE_CONTROLLER // Change to ROLE_RECEIVER for the receiver

// Servo and Motor Pins (Receiver)
#define SERVO_PIN 15
#define MOTOR_L1 2
#define MOTOR_L2 3
#define MOTOR_R1 4
#define MOTOR_R2 5

// Control values
int angle = 90;      // Default angle for the servo
int throttle = 0;    // Default throttle value

Servo servo;

// ESP-NOW peer address
uint8_t peerAddress[] = {0x24, 0x6F, 0x28, 0xFF, 0xFF, 0xFF}; // Replace with receiver MAC

// Structure to hold control data
typedef struct struct_message {
    int angle;
    int throttle;
} struct_message;

struct_message controlData;

// Callback for receiving data
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&controlData, incomingData, sizeof(controlData));
    angle = controlData.angle;
    throttle = controlData.throttle;
    servo.write(angle);
    analogWrite(MOTOR_L1, throttle);
    analogWrite(MOTOR_R1, throttle);
    Serial.printf("Received: Angle=%d, Throttle=%d\n", angle, throttle);
}

// Sending data
void sendControlData(int angle, int throttle) {
    controlData.angle = angle;
    controlData.throttle = throttle;
    esp_now_send(peerAddress, (uint8_t *) &controlData, sizeof(controlData));
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    if (ROLE == ROLE_RECEIVER) {
        servo.attach(SERVO_PIN);
        pinMode(MOTOR_L1, OUTPUT);
        pinMode(MOTOR_L2, OUTPUT);
        pinMode(MOTOR_R1, OUTPUT);
        pinMode(MOTOR_R2, OUTPUT);
        Serial.println("Receiver ready");
    } else {
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, peerAddress, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer");
            return;
        }
        Serial.println("Controller ready");
    }
}

void loop() {
    if (ROLE == ROLE_CONTROLLER) {
        if (Serial.available() > 0) {
            angle = Serial.parseInt();
            throttle = Serial.parseInt();
            sendControlData(angle, throttle);
            Serial.printf("Sent: Angle=%d, Throttle=%d\n", angle, throttle);
        }
    }
}
