#include <arduino.h>
#include <SoftwareSerial.h>
#include <ThingSpeak.h>

// Variabel untuk menyimpan API Key dan Channel Number
char writeAPIKey[] = "PFUUIPRWBG7MYXLG"; // Ganti dengan Write API Key ThingSpeak Anda
unsigned long channelNumber = 2560671; // Ganti dengan Channel ID ThingSpeak Anda

// Ultrasonic sensor pins
const int trigPin = 23;
const int echoPin = 22;

// SIM800L pins
const int SIM800_TX = 26;
const int SIM800_RX = 27;
SoftwareSerial sim800l(SIM800_TX, SIM800_RX);

// AT command settings
const char apn[] = "your_apn"; // Ganti dengan APN provider Anda
const char apn_user[] = ""; // Ganti dengan APN username provider Anda
const char apn_pass[] = ""; // Ganti dengan APN password provider Anda

long duration;
int distance;
int maxHeight = 300; // Maksimal ketinggian dalam cm

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  sim800l.begin(9600);

  // Setup GPRS
  setupGPRS();

  // Setup ThingSpeak
  ThingSpeak.begin(sim800l);
}

void loop() {
  // Menyalakan pin trig untuk 10 mikrodetik untuk mengirimkan sinyal
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Membaca durasi dari echo pin (waktu yang dibutuhkan sinyal untuk kembali)
  duration = pulseIn(echoPin, HIGH);

  // Menghitung jarak (ketinggian air) dalam cm
  distance = duration * 0.034 / 2;
  float percentHeight = (float)distance / maxHeight * 100;

  Serial.print("Ketinggian air: ");
  Serial.print(distance);
  Serial.print(" cm, Persentase ketinggian: ");
  Serial.print(percentHeight);
  Serial.println(" %");

  // Mengirim data ke ThingSpeak
  ThingSpeak.setField(1, distance);
  ThingSpeak.setField(2, percentHeight);

  int x = ThingSpeak.writeFields(channelNumber, writeAPIKey);

  if (x == 200) {
    Serial.println("Data berhasil dikirim ke ThingSpeak");
  } else {
    Serial.println("Gagal mengirim data ke ThingSpeak, kode: " + String(x));
  }

  // Delay untuk beberapa waktu sebelum membaca ulang (misalnya setiap 15 detik)
  delay(15000);
}

void setupGPRS() {
  sendATCommand("AT", "OK", 1000);
  sendATCommand("AT+CSQ", "OK", 1000);
  sendATCommand("AT+CPIN?", "READY", 1000);
  sendATCommand("AT+CREG?", "0,1", 1000);
  sendATCommand("AT+CGATT?", "1", 1000);
  sendATCommand("AT+CSTT=\"" + String(apn) + "\",\"" + String(apn_user) + "\",\"" + String(apn_pass) + "\"", "OK", 1000);
  sendATCommand("AT+CIICR", "OK", 2000);
  sendATCommand("AT+CIFSR", ".", 10000);
  sendATCommand("AT+CIPMUX=1", "OK", 1000);
  sendATCommand("AT+CIPSTART=0,\"TCP\",\"api.thingspeak.com\",80", "CONNECT OK", 3000);
}

void sendATCommand(String command, String response, unsigned long timeout) {
  sim800l.println(command);
  unsigned long time = millis();
  while ((time + timeout) > millis()) {
    while (sim800l.available()) {
      if (sim800l.find(response.c_str())) {
        return;
      }
    }
  }
  Serial.println("Gagal mengirim AT Command: " + command);
}
