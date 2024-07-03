#include <WiFi.h>
#include <WebServer.h>
#include <ThingSpeak.h>

const char* ssid = "Modem-WLM"; // Ganti dengan nama SSID WiFi Anda
const char* password = "Walemon-123"; // Ganti dengan password WiFi Anda

IPAddress local_IP(192, 168, 100, 200);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient client;
WebServer server(80); // HTTP server on port 80

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
int distance_inverse;
int tinggi_maksimal = 100; // Tinggi maksimal dalam cm (default value)
float persentase_distance;

unsigned long previousMillis = 0;
const long interval = 15000; // Interval pengiriman data ke ThingSpeak dalam milidetik

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Set Tinggi Maksimal</title><style>";
  html += "body { font-family: 'Arial', sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }";
  html += "form { display: flex; flex-direction: column; align-items: center; }";
  html += "p { margin-bottom: 20px; }";
  html += "input[type='number'] { margin: 10px 0; padding: 10px; font-size: 16px; }";
  html += "input[type='submit'] { padding: 10px 20px; font-size: 16px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "</style></head><body><form action='/set' method='POST'>";
  html += "<h1>Water Level Monitoring</h1>";
  html += "<p>Tugas Akhir: Teguh Wicaksono</p>";
  html += "<label for='tinggi'>Tinggi Maksimal (cm):</label>";
  html += "<input type='number' id='tinggi' name='tinggi' value='" + String(tinggi_maksimal) + "'>";
  html += "<input type='submit' value='Set'>";
  html += "</form></body></html>";

  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("tinggi")) {
    tinggi_maksimal = server.arg("tinggi").toInt();
    String html = "<!DOCTYPE html><html><head><title>Set Tinggi Maksimal</title><style>";
    html += "body { font-family: 'Arial', sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }";
    html += ".container { display: flex; flex-direction: column; align-items: center; }";
    html += "a { margin-top: 20px; padding: 10px 20px; font-size: 16px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 5px; }";
    html += "a:hover { background-color: #45a049; }";
    html += "</style></head><body><div class='container'>";
    html += "<h1>Tinggi Maksimal diatur ke " + String(tinggi_maksimal) + " cm</h1>";
    html += "<a href='/'>Kembali</a>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
    Serial.println("Berhasil mengatur tinggi maskimal:" + String(tinggi_maksimal));
  } else {
    server.send(400, "text/html", "<html><body><h1>Invalid Request</h1></body></html>");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  ThingSpeak.begin(client);

  // Start HTTP server
  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.begin();
}

void loop() {
  server.handleClient();
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

  // Menghitung jarak (ketinggian air dibalik)
  distance_inverse = tinggi_maksimal - distance;

  // Menghitung persentase ketinggian
  persentase_distance = ((float)distance_inverse / tinggi_maksimal) * 100;

  Serial.print("Ketinggian air: ");
  Serial.print(distance_inverse);
  Serial.println(" cm");
  Serial.print("Persentase ketinggian: ");
  Serial.print(persentase_distance);
  Serial.println(" %");

  // Jika persentase ketinggian air melebihi 60%, buzzer pasif akan berbunyi
  if (persentase_distance > 60) {
    int buzzerDelay = map(persentase_distance, 60, 100, 2500, 200);
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
