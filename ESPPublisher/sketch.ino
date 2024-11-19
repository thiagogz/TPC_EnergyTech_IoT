#include <WiFi.h>          // Biblioteca para gerenciar conexões WiFi
#include <PubSubClient.h>  // Biblioteca para cliente MQTT

// Credenciais da rede WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org"; // Servidor MQTT público

WiFiClient espClient;             // Objeto para a conexão WiFi
PubSubClient client(espClient);   // Objeto cliente MQTT usando WiFi

// Pino para o potenciômetro
const int potentiometerPin = 34; // Pino analógico conectado ao potenciômetro

// Tópicos MQTT para envio de dados
const char* mqtt_topic_percentage = "batteryPercentageIN";
const char* mqtt_topic_voltage = "batteryVoltageIN";

// Variáveis para armazenar os valores
float batteryLevel = 0;
float batteryVoltage = 0;

// Constantes para calibração
const float maxVoltage = 12.6; // Voltagem máxima esperada

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial

  // Configuração do pino de entrada
  pinMode(potentiometerPin, INPUT);

  setup_wifi();  // Conecta ao WiFi
  client.setServer(mqtt_server, 1883);  // Configura o servidor MQTT
  client.setKeepAlive(15);       // Define o tempo de keep-alive (15 segundos)
  client.setSocketTimeout(15);   // Define o tempo de timeout do socket (15 segundos)
}

// Função para conectar ao WiFi
void setup_wifi() {
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  // Inicia a conexão WiFi

  // Espera até conectar ao WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());  // Exibe o endereço IP
}

// Função para publicar os dados no servidor MQTT
void publishData() {
  int potValue = analogRead(potentiometerPin); // Lê o valor do potenciômetro (0-4095)

  // Interpreta o valor do potenciômetro em diferentes faixas
  batteryLevel = map(potValue, 0, 4095, 0, 100);                       // Nível de bateria em %
  batteryVoltage = (potValue / 4095.0) * maxVoltage;                   // Tensão em volts

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
  while (!client.connected()) {  // Verifica se o cliente está conectado
    Serial.print("Tentando conexão MQTT...");
    // Tenta conectar com o ID "ESP32ClientPublisher"
    if (client.connect("ESP32ClientPublisher")) {
      Serial.println("conectado");
    } else {
      // Se falhar, exibe o código de erro e tenta novamente após 5 segundos
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tente novamente em 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  // Verifica se o cliente MQTT está conectado
  if (!client.connected()) {
    reconnect();  // Reconecta se necessário
  }
  publishData();  // Publica os dados
  client.loop();  // Mantém a conexão ativa
  delay(2000);    // Aguarda 2 segundos antes de publicar novamente
}
