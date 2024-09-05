#include <ACS712.h>          // Include the library for the current sensor
#include <WiFi.h>            // Include the WiFi library
#include "time.h"            // Include the time library

// Constants for relay, current sensor, and voltage
const int relayPin = 23;         // GPIO pin for relay control
const int currentSensorPin = 34; // Analog pin for ACS712
const float voltage = 230.0;     // Voltage in Volts (e.g., 230V for EU or 120V for US)
ACS712 currentSensor(ACS712_30A, currentSensorPin); // Initialize ACS712 sensor

unsigned long previousMillis = 0;  // Store last time in milliseconds
const long interval = 1000;        // Interval to read current (1 second)
float totalEnergy = 0.0;           // Total energy consumed in kWh

// Wi-Fi and NTP configuration
const char* ssid = "your_SSID";        // Replace with your Wi-Fi SSID
const char* password = "your_PASSWORD"; // Replace with your Wi-Fi password
const char* ntpServer = "pool.ntp.org"; // NTP server to sync time
const long gmtOffset_sec = 0;          // Set GMT offset in seconds (adjust for your timezone)
const int daylightOffset_sec = 3600;   // Set daylight saving time offset in seconds

const char* applicationID = "APP_12345"; // Assign your application ID

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Relay setup
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Relay off by default
  
  // Current sensor setup
  currentSensor.calibrate();  // Calibrate current sensor if needed
}

void loop() {
  unsigned long currentMillis = millis(); // Get current time in milliseconds

  // Check if interval time has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last time current was read
    
    // Read current sensor value
    float currentValue = currentSensor.getCurrentAC();  // Get current value in Amperes

    // Calculate power in Watts
    float power = voltage * currentValue;  // Power (W) = Voltage (V) * Current (A)

    // Calculate energy consumed in kWh
    float energy = (power * (interval / 3600000.0)) / 1000.0;  // Energy in kWh for interval

    // Accumulate total energy consumption
    totalEnergy += energy;

    // Get current time and date
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    // Print the application ID, time, date, current, power, and total energy to Serial Monitor
    Serial.print("Application ID: ");
    Serial.print(applicationID);
    Serial.print(", Time: ");
    Serial.printf("%02d:%02d:%02d, ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.print("Date: ");
    Serial.printf("%04d-%02d-%02d, ", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    Serial.print("Current: ");
    Serial.print(currentValue);
    Serial.print(" A, Power: ");
    Serial.print(power);
    Serial.print(" W, Total Energy: ");
    Serial.print(totalEnergy);
    Serial.println(" kWh");

    // Control relay based on some condition
    if (currentValue > 2.0) {  // Example: Turn on relay if current > 2A
      digitalWrite(relayPin, HIGH);  // Turn on relay
    } else {
      digitalWrite(relayPin, LOW);  // Turn off relay
    }
  }
}
