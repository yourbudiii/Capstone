#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6MrVrGpr4"
#define BLYNK_TEMPLATE_NAME "Medical Check Up Detak Jantung dan Tekanan Darah"
#define BLYNK_AUTH_TOKEN "M_PFcwIgjlBTG2LWbO8mdojUuH-tkoAa"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "LAN-ang";
char pass[] = "anaklanang";

// Definisikan LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C mungkin berbeda, tergantung pada modul Anda

char buff[64];
bool b_read = false, b_discard = false;
char discard;
int i, j = 0;
char final_buff[64];

int hexSys, hexDias, hexBPM;

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

}



void loop() {
  // Baca input serial
  while (Serial.available()) {
    if (!b_read) {
      buff[0] = Serial.read();
      if (buff[0] == 'e') {
        buff[1] = Serial.read();
        if (buff[1] == 'r') {
          buff[2] = Serial.read();
          if (buff[2] == 'r') {
            buff[3] = Serial.read();
            if (buff[3] == ':') {
              buff[4] = Serial.read();
              if (buff[4] == '0') {
                b_read = true;
                j = 0;
                b_discard = false;
                i = 0;
              }
            }
          }
        }
      }
    }
    if (b_read) {
      if (!b_discard) {
        discard = Serial.read();
        i++;
      } else if (j < 11) {
        final_buff[j] = Serial.read();
        j++;
      } else {
        b_read = false;
      }
      if (i == 30) {
        b_discard = true;
      }
    }
    delay(2);
  }

  // Konversi karakter heksadesimal ke desimal
  if (final_buff[0] > '9') {
    hexSys = (final_buff[0] - '7') * 16;
  } else {
    hexSys = (final_buff[0] - '0') * 16;
  }
  if (final_buff[1] > '9') {
    hexSys += (final_buff[1] - '7');
  } else {
    hexSys += (final_buff[1] - '0');
  }

  if (final_buff[3] > '9') {
    hexDias = (final_buff[3] - '7') * 16;
  } else {
    hexDias = (final_buff[3] - '0') * 16;
  }
  if (final_buff[4] > '9') {
    hexDias += (final_buff[4] - '7');
  } else {
    hexDias += (final_buff[4] - '0');
  }

  //if (final_buff[9] > '9') {
    //hexBPM = (final_buff[9] - '7') * 16;
  //} else {
    //hexBPM = (final_buff[9] - '0') * 16;
  //}
  //if (final_buff[10] > '9') {
    //hexBPM += (final_buff[10] - '7');
  //} else {
    //hexBPM += (final_buff[10] - '0');
  //}

  // Cetak ke Serial Monitor
  Serial.print(hexSys);
  Serial.print("          ");
  Serial.print(hexDias);
  Serial.print("          ");
  //Serial.println(hexBPM);

  // Cetak ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MmHg :");
  lcd.print(hexSys);
  lcd.setCursor(9, 0);
  lcd.print("/");
  lcd.print(hexDias);
  Blynk.virtualWrite(V8, hexDias);
  Blynk.virtualWrite(V10, hexSys);
  // lcd.setCursor(0, 1);
  // lcd.print("BPM:");
  // lcd.print(hexBPM);

  Serial.println();
  Blynk.run();
  delay(500);
}
