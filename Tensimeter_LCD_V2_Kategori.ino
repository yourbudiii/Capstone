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

int hexSys, hexDias;

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Inisialisasi Blynk
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
  hexSys = (final_buff[0] > '9' ? (final_buff[0] - '7') * 16 : (final_buff[0] - '0') * 16) +
           (final_buff[1] > '9' ? (final_buff[1] - '7') : (final_buff[1] - '0'));

  hexDias = (final_buff[3] > '9' ? (final_buff[3] - '7') * 16 : (final_buff[3] - '0') * 16) +
            (final_buff[4] > '9' ? (final_buff[4] - '7') : (final_buff[4] - '0'));

  // Tentukan kategori tekanan darah
  String kategori;
  if (hexSys < 0 || hexDias < 0) {
    kategori = "Tidak Terbaca";
  } else if (hexSys < 120 && hexDias < 80) {
    kategori = "Normal";
  } else if ((hexSys >= 120 && hexSys < 140) || (hexDias >= 80 && hexDias < 90)) {
    kategori = "Pra-HTN";
  } else if ((hexSys >= 140 && hexSys < 160) || (hexDias >= 90 && hexDias < 100)) {
    kategori = "HTN T1";
  } else if (hexSys >= 160 || hexDias >= 100) {
    kategori = "HTN T2";
  } else {
    kategori = "Unknown";
  }
  // Cetak ke Serial Monitor
  Serial.print("Sistolik: ");
  Serial.print(hexSys);
  Serial.print(" Diastolik: ");
  Serial.print(hexDias);
  Serial.print(" Kategori: ");
  Serial.println(kategori);

  // Cetak ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MmHg :");
  lcd.print(hexSys);
  lcd.setCursor(9, 0);
  lcd.print("/");
  lcd.print(hexDias);
  lcd.setCursor(0, 1);
  lcd.print(kategori);

  // Kirim data ke Blynk
  Blynk.virtualWrite(V8, hexDias);
  Blynk.virtualWrite(V10, hexSys);

  Blynk.run();
  delay(500);
}
