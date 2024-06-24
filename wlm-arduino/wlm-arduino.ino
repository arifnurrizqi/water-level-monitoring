#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "Wokwi-GUEST"; // Ganti dengan nama SSID WiFi Anda
const char* password = ""; // Ganti dengan password WiFi Anda

WiFiClient client;

// Variabel untuk menyimpan API Key dan Channel Number
char writeAPIKey[17] = "PFUUIPRWBG7MYXLG"; // Ganti dengan Write API Key ThingSpeak default Anda
char channelNumber[10] = "2560671"; // Ganti dengan Channel ID ThingSpeak default Anda

unsigned long myChannelNumber;

// Ultrasonic sensor pins
const int trigPin = 23;
const int echoPin = 22;

long duration;
int distance;
const int tinggi_maksimal = 100; // Tinggi maksimal dalam cm
float persentase_distance;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");

  ThingSpeak.begin(client);
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

  // Menghitung persentase ketinggian
  persentase_distance = ((float)distance / tinggi_maksimal) * 100;

  Serial.print("Ketinggian air: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("Persentase ketinggian: ");
  Serial.print(persentase_distance);
  Serial.println(" %");

  // Mengirim data ke ThingSpeak
  ThingSpeak.setField(1, distance);
  ThingSpeak.setField(2, persentase_distance);

  int x = ThingSpeak.writeFields(myChannelNumber, writeAPIKey);

  if (x == 200) {
    Serial.println("Data berhasil dikirim ke ThingSpeak");
  } else {
    Serial.println("Gagal mengirim data ke ThingSpeak, kode: " + String(x));
  }

  // Delay untuk beberapa waktu sebelum membaca ulang (misalnya setiap 15 detik)
  delay(5000);
}