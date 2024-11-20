#include <WiFi.h>          
#include <PubSubClient.h>  

// Credenciais da rede WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuração do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_topic_percentage = "batteryPercentageIN";
const char* mqtt_topic_voltage = "batteryVoltageIN";

// Cliente Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Pino para o potenciômetro
const int potentiometerPin = 34; 

// Variáveis para armazenar os valores
float batteryLevel = 0;
float batteryVoltage = 0;

// Constantes para calibração
const float maxVoltage = 12.6; // Voltagem máxima esperada

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial

  // Configuração do pino de entrada
  pinMode(potentiometerPin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// Função para conectar ao WiFi
void setup_wifi() {
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Espera até conectar ao WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP()); 
}

// Função para publicar os dados no servidor MQTT
void publishData() {
  int potValue = analogRead(potentiometerPin);

  // Interpreta o valor do potenciômetro em diferentes faixas
  batteryLevel = map(potValue, 0, 4095, 0, 100);             // Nível de bateria em %
  batteryVoltage = (potValue / 4095.0) * maxVoltage;         // Tensão em volts

  // Converte os valores para strings
  String batteryLevelStr = String(batteryLevel);
  String batteryVoltageStr = String(batteryVoltage, 2);

  // Publica os valores nos tópicos MQTT correspondentes
  client.publish(mqtt_topic_percentage, batteryLevelStr.c_str());
  client.publish(mqtt_topic_voltage, batteryVoltageStr.c_str());

  // Exibe os valores publicados no Serial Monitor
  Serial.println("Dados publicados:");
  Serial.println("Nível de Bateria: " + batteryLevelStr + " %");
  Serial.println("Tensão da Bateria: " + batteryVoltageStr + " V");
}

// Função para reconectar ao servidor MQTT caso a conexão seja perdida
void reconnect() {
  while (!client.connected()) { 
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP32ClientPublisher")) {
      Serial.println("conectado");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tente novamente em 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  publishData();
  client.loop();
  delay(2000);
}
