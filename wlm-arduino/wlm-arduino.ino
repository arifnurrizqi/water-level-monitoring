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

// Buzzer pin
const int buzzerPin = 19;

long duration;
int distance;
const int tinggi_maksimal = 100; // Tinggi maksimal dalam cm
float persentase_distance;

unsigned long previousMillis = 0;
const long interval = 15000; // Interval pengiriman data ke ThingSpeak dalam milidetik

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

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
  unsigned long currentMillis = millis();

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

  // Jika persentase ketinggian air melebihi 60%, buzzer pasif akan berbunyi
  if (persentase_distance > 60) {
    int buzzerDelay = map(persentase_distance, 60, 100, 5000, 1000);
    digitalWrite(buzzerPin, HIGH); // Menghidupkan buzzer
    delay(buzzerDelay);
    digitalWrite(buzzerPin, LOW); // Mematikan buzzer
    delay(500); // Tambahan delay untuk interval bunyi
  } else {
    digitalWrite(buzzerPin, LOW); // Memastikan buzzer mati jika persentase di bawah 60%
  }

  // Mengirim data ke ThingSpeak setiap interval yang ditentukan
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ThingSpeak.setField(1, distance);
    ThingSpeak.setField(2, persentase_distance);

    int x = ThingSpeak.writeFields(myChannelNumber, writeAPIKey);

    if (x == 200) {
      Serial.println("Data berhasil dikirim ke ThingSpeak");
    } else {
      Serial.println("Gagal mengirim data ke ThingSpeak, kode: " + String(x));
    }
  }
}
