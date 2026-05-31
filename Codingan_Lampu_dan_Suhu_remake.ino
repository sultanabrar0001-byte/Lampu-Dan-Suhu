
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Gusti Rizal";
const char* password = "Dewikos1234";

#define BOT_TOKEN "8881760002:AAGVZTYS8RaTs0VT87p1xk9s5Hw11BeLkiU"
#define CHAT_ID "8909914831"

#define DHTPIN 4        
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define RELAY1_PIN 25   
#define RELAY2_PIN 26   
#define RELAY3_PIN 14   
#define RELAY4_PIN 27   

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastBotTime = 0;
const unsigned long BOT_MTBS = 1000; 

unsigned long lastSensorTime = 0;
const unsigned long SENSOR_INTERVAL = 5000; 

bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;

void setupWiFi();
void setupSensors();
void setupRelays();
void handleNewMessages(int numNewMessages);
void updateRelayState(int relayNum, bool state);
void sendSensorData();
void printSensorData(float temp, float humidity);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32 IoT System Siap Dimulai...");
  
  setupRelays();
  setupSensors();
  setupWiFi();
  
  client.setInsecure(); 
  Serial.println("Setup Selesai!");
}

void loop() {
  
  if (millis() > lastBotTime + BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) {
      Serial.println("Menerima perintah Telegram...");
      handleNewMessages(numNewMessages);
    }
    lastBotTime = millis();
  }
  
  
  if (millis() > lastSensorTime + SENSOR_INTERVAL) {
    sendSensorData();
    lastSensorTime = millis();
  }
}

void updateRelayState(int relayNum, bool state) {
  int pin;
  switch(relayNum) {
    case 1: pin = RELAY1_PIN; relay1State = state; break;
    case 2: pin = RELAY2_PIN; relay2State = state; break;
    case 3: pin = RELAY3_PIN; relay3State = state; break;
    case 4: pin = RELAY4_PIN; relay4State = state; break;
    default: return;
  }
  
  digitalWrite(pin, state ? LOW : HIGH);
  
  Serial.print("Relay "); Serial.print(relayNum);
  Serial.println(state ? " -> KONDISI ON" : " -> KONDISI OFF");
}

void setupWiFi() {
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Terhubung!");
    Serial.print("IP Address ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Gagal terhubung ke WiFi!");
  }
}

void setupSensors() {
  dht.begin();
  Serial.println("Sensor DHT11 Berhasil Dikonfigurasi");
}

void setupRelays() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
  
  Serial.println("Semua Relay diset awal: MATI (OFF)");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "User Tidak Dikenal!", "");
      return;
    }
    
    if (text == "/start") {
      String welcome = "Welcome " + from_name + " ke ESP32 Control System!\n\n";
      welcome += "Daftar Perintah:\n";
      welcome += "/status - Cek Suhu & Status Relay\n";
      welcome += "/1_on - Nyalakan Relay 1\n";
      welcome += "/1_off - Matikan Relay 1\n";
      welcome += "/2_on - Nyalakan Relay 2\n";
      welcome += "/2_off - Matikan Relay 2\n";
      welcome += "/3_on - Nyalakan Relay 3\n";
      welcome += "/3_off - Matikan Relay 3\n";
      welcome += "/4_on - Nyalakan Relay 4\n";
      welcome += "/4_off - Matikan Relay 4\n";
      welcome += "/temp - Cek Suhu & Kelembapan\n";
      welcome += "/all_on - Nyalakan Semua\n";
      welcome += "/all_off - Matikan Semua\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
    else if (text == "/status") {
      float temp = dht.readTemperature();
      float humidity = dht.readHumidity();
      
      String status = " *STATUS SISTEM SAAT INI*\n\n";
      status += "  Suhu: " + String(temp, 1) + "°C\n";
      status += "  Kelembapan: " + String(humidity, 1) + "%\n\n";
      status += " Status Lampu/Relay:\n";
      status += "Relay 1: " + String(relay1State ? " NYALA" : " MATI") + "\n";
      status += "Relay 2: " + String(relay2State ? " NYALA" : " MATI") + "\n";
      status += "Relay 3: " + String(relay3State ? " NYALA" : " MATI") + "\n";
      status += "Relay 4: " + String(relay4State ? " NYALA" : " MATI") + "\n";
      
      bot.sendMessage(chat_id, status, "Markdown");
    }
    
    else if (text == "/temp") {
      float temp = dht.readTemperature();
      float humidity = dht.readHumidity();
      
      if (isnan(temp) || isnan(humidity)) {
        bot.sendMessage(chat_id, "Gagal membaca sensor DHT11!", "");
      } else {
        String tempMsg = " Suhu: " + String(temp, 1) + "°C\n";
        tempMsg += " Kelembapan: " + String(humidity, 1) + "%\n";
        bot.sendMessage(chat_id, tempMsg, "");
      }
    }
    
    
    else if (text == "/1_on")  { updateRelayState(1, true);  bot.sendMessage(chat_id, " Relay 1 NYALA", ""); }
    else if (text == "/1_off") { updateRelayState(1, false); bot.sendMessage(chat_id, " Relay 1 MATI", ""); }
    
    else if (text == "/2_on")  { updateRelayState(2, true);  bot.sendMessage(chat_id, " Relay 2 NYALA", ""); }
    else if (text == "/2_off") { updateRelayState(2, false); bot.sendMessage(chat_id, " Relay 2 MATI", ""); }
    
    else if (text == "/3_on")  { updateRelayState(3, true);  bot.sendMessage(chat_id, " Relay 3 NYALA", ""); }
    else if (text == "/3_off") { updateRelayState(3, false); bot.sendMessage(chat_id, " Relay 3 MATI", ""); }
    
    else if (text == "/4_on")  { updateRelayState(4, true);  bot.sendMessage(chat_id, " Relay 4 NYALA", ""); }
    else if (text == "/4_off") { updateRelayState(4, false); bot.sendMessage(chat_id, " Relay 4 MATI", ""); }
    
    
    else if (text == "/all_on") {
      updateRelayState(1, true); updateRelayState(2, true); updateRelayState(3, true); updateRelayState(4, true);
      bot.sendMessage(chat_id, " Semua Relay NYALA", "");
    }
    else if (text == "/all_off") {
      updateRelayState(1, false); updateRelayState(2, false); updateRelayState(3, false); updateRelayState(4, false);
      bot.sendMessage(chat_id, " Semua Relay MATI", "");
    }
  }
}

void sendSensorData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (!isnan(temp) && !isnan(humidity)) {
    printSensorData(temp, humidity);
  } else {
    Serial.println("Gagal membaca sensor DHT11!");
  }
}

void printSensorData(float temp, float humidity) {
  Serial.println("═══════════════════════════════");
  Serial.print("Suhu: "); Serial.print(temp); Serial.println(" °C");
  Serial.print("Kelembapan: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("R1: "); Serial.print(relay1State ? "ON " : "OFF ");
  Serial.print("| R2: "); Serial.print(relay2State ? "ON " : "OFF ");
  Serial.print("| R3: "); Serial.print(relay3State ? "ON " : "OFF ");
  Serial.print("| R4: "); Serial.println(relay4State ? "ON" : "OFF");
  Serial.println("═══════════════════════════════");
}