#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>

// Conexão Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuração do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_topic_voltage = "batteryVoltageOUT";
const char* mqtt_topic_percentage = "batteryPercentageOUT";

// Cliente Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Configuração da tela OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Configuração dos LEDs
const int ledGreen = 4;
const int ledRed = 2;

// Variáveis para armazenar os valores recebidos
float voltage = 0.0;
float batteryPercentage = 0.0;

void setup() {
  Serial.begin(115200);

  // Configura LEDs como saída
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);

  // Inicializa Wi-Fi
  setup_wifi();

  // Inicializa MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();

  // Inicializa a tela OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C é o endereço I2C padrão do OLED
      Serial.println("Falha ao inicializar a tela OLED");
      while (true);
  }

  display.clearDisplay();
  display.display();
}

void setup_wifi() {
  Serial.print("Conectando-se ao WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando-se ao broker MQTT...");
    if (client.connect("ESP32ClientReceiver")) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_voltage);
      client.subscribe(mqtt_topic_percentage);
    } else {
      Serial.print("Falha, código: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (String(topic) == mqtt_topic_voltage) {
    voltage = message.toFloat();
    Serial.println("Voltagem recebida: " + String(voltage) + "V");
  } else if (String(topic) == mqtt_topic_percentage) {
    batteryPercentage = message.toFloat();
    Serial.println("Porcentagem da bateria recebida: " + String(batteryPercentage) + "%");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Controle dos LEDs com base na porcentagem da bateria
  if (batteryPercentage >= 90.0) {
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
  } else {
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, HIGH);
  }

  // Atualiza a tela OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Voltagem: ");
  display.print(voltage);
  display.println(" V");
  display.print("Bateria: ");
  display.print(batteryPercentage);
  display.println(" %");
  display.display();

  delay(1000);
}
