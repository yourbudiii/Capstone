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

const int buttonPin = D6; // Pin push button
bool buttonState = false; // Menyimpan status push button
bool systemActive = false; // Menyimpan status sistem aktif/non-aktif
unsigned long lastDebounceTime = 0; // Waktu terakhir tombol ditekan
unsigned long debounceDelay = 50; // Debounce time (ms)

bool fingerDetected = false;
bool systemMessageDisplayed = false;



void setup()
{
  pinMode(buttonPin, INPUT_PULLUP); // Konfigurasi pin button dengan pull-up
  Serial.begin(115200);

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

void clearLine(int line)
{
  lcd.setCursor(0, line);
  lcd.print("                "); // 16 spaces to clear the line
}

void loop()
{
  int reading = digitalRead(buttonPin);
  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    buttonState = !buttonState;
    systemActive = !systemActive;
    systemMessageDisplayed = false; // Reset flag

    clearLine(0); // Clear first line
    clearLine(1); // Clear second line
    if (systemActive) {
      lcd.setCursor(0, 0);
      lcd.print("Sistem Aktif    ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Sistem Mati     ");
    }
    delay(1000); // Delay untuk menampilkan status sistem
  }

  if (systemActive) {
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
        clearLine(0);
        clearLine(1);
        lcd.print("                "); // Menghapus pesan "No Finger"
      }
      particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);

      if (heartRate == -999) {
        Serial.println("letakkan jari");
        lcd.setCursor(0, 0);
        lcd.print("letakkan jari   ");
  
      
      } else {
        Serial.print(F("heartRate="));
        Serial.print(heartRate, DEC);
        Serial.print(F(", heartRateValid="));
        Serial.println(heartRateValid, DEC);

  
        lcd.setCursor(0, 0);
        lcd.print("HR:");
        lcd.print(heartRate);
        lcd.print(" bpm        ");

        Blynk.virtualWrite(V7, heartRate);

      }
    }

    Serial.println();
  } else {
    if (!systemMessageDisplayed) {
   
      lcd.setCursor(0, 0);
      lcd.print("Tekan Tombol    ");
      lcd.setCursor(0, 1);
      lcd.print("untuk Aktif     ");
      systemMessageDisplayed = true; // Set flag to true after displaying the message
    }
  }

  Blynk.run();
}