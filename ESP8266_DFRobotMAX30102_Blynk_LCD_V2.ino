#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6MrVrGpr4"
#define BLYNK_TEMPLATE_NAME "Medical Check Up Detak Jantung dan Tekanan Darah"
#define BLYNK_AUTH_TOKEN "M_PFcwIgjlBTG2LWbO8mdojUuH-tkoAa"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobot_MAX30102.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "NINOT";
char pass[] = "ikancupang";

LiquidCrystal_I2C lcd(0x27, 16, 2); // Address LCD I2C dan ukuran (16x2)
DFRobot_MAX30102 particleSensor;

bool fingerDetected = false;
const int buttonPin = D0; // Pin untuk push button
bool systemOn = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

void setup()
{
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP); // Mengatur pin D0 sebagai input dengan pull-up internal

  if (!particleSensor.begin())
  {
    Serial.println("MAX30102 was not found");
    while (1)
    {
      delay(1000);
    }
  }

  particleSensor.sensorConfiguration(50, SAMPLEAVG_4, MODE_MULTILED, SAMPLERATE_100, PULSEWIDTH_411, ADCRANGE_16384);

  lcd.init();
  lcd.backlight();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, "blynk-cloud.com", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);
}

int32_t SPO2;
int8_t SPO2Valid;
int32_t heartRate;
int8_t heartRateValid;

void loop()
{
  Blynk.run();

  int buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    unsigned long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = currentTime;
      systemOn = !systemOn; // Toggle system state
      if (systemOn) {
        Serial.println("System turned on");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System On       ");
      } else {
        Serial.println("System turned off");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System Off      ");
        return; // Stop further execution if the system is turned off
      }
    }
  }

  if (systemOn) {
    long irValue = particleSensor.getIR();
    Serial.print("IR=");
    Serial.print(irValue);

    if (irValue < 45000) {
      Serial.println("  No finger?");
      fingerDetected = false;

      lcd.setCursor(0, 0);
      lcd.print("No Finger       ");
    } else {
      if (!fingerDetected) {
        fingerDetected = true;
        lcd.setCursor(0, 0);
        lcd.print("                "); // Menghapus pesan "No Finger"
      }
      particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);

      if (heartRate == -999) {
        Serial.println("letakkan jari dengan benar");
        lcd.setCursor(0, 0);
        lcd.print("letakkan jari   ");
        lcd.setCursor(0, 1);
        lcd.print("dengan benar    ");
      } else {
        Serial.print(F("heartRate="));
        Serial.print(heartRate, DEC);
        Serial.print(F(", heartRateValid="));
        Serial.println(heartRateValid, DEC);

        lcd.setCursor(0, 0);
        lcd.print("HR:");
        lcd.print(heartRate);
        lcd.print(" bpm        ");
        lcd.setCursor(0, 1);
        lcd.print("                ");

        Blynk.virtualWrite(V7, heartRate);
        //Blynk.virtualWrite(V8, SPO2);
      }
    }

    Serial.println();
  }
}
