#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "Heartbeat.h"

#define AWS_IOT_PUBLISH_TOPIC "pressure/sensor/data"
#define AWS_IOT_SUBSCRIBE_TOPIC "pressure/sensor/config"

#define LED 2
const int analogPin = 36;       // GPIO36 (ADC1_CH0)
unsigned long intervalo = 1000; // Intervalo em milissegundos (1000ms = 1s)
unsigned long ultimoTempo = 0;  // Armazena a última vez que o LED mudou de estado
bool estadoLed = LOW;           // Estado atual do LED

float pressao;
int valorADC;

unsigned long lastPublish = 0;      // guarda o último tempo em que publicou
const long publishInterval = 10000; // intervalo de 10 segundos

// Instância da classe Heartbeat
Heartbeat heartbeat("pool.ntp.org", -3, 60000); // servidor NTP, UTC-3(Brasil), update a cada 60s
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char *topic, byte *payload, unsigned int length);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  // Inicializa o heartbeat após conectar ao WiFi
  heartbeat.begin();
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  // Create a message handler
  client.setCallback(messageHandler);
  Serial.println("Connecting to AWS IOT");
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  // Atualiza o tempo antes de capturar
  heartbeat.update();

  // Verifica se o tempo é válido
  if (!heartbeat.isTimeValid())
  {
    Serial.println("Tempo NTP não sincronizado, pulando publicação...");
    return;
  }

  StaticJsonDocument<400> doc;
  doc["timestamp"] = heartbeat.getEpochTime();        // Para chave de partição DynamoDB
  doc["datetime"] = heartbeat.getDataHoraFormatada(); // Formato brasileiro legível
  doc["ADC"] = valorADC;
  doc["Pressao_bar"] = pressao;
  doc["device_id"] = THINGNAME; // Identificador do dispositivo

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  if (published)
  {
    Serial.println("✓ Dados enviados com sucesso:");
    Serial.println(jsonBuffer);
  }
  else
  {
    Serial.println("✗ Falha ao enviar dados MQTT");
  }
}

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensagem recebida em: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char *message = doc["message"];
  Serial.print("Conteúdo: ");
  Serial.println(message);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  Serial.println("=== ESP32 Sensor de Pressão ===");
  connectAWS();
}

void loop()
{
  valorADC = analogRead(analogPin);         // Lê valor entre 0 e 4095
  float tensao = (valorADC / 4095.0) * 3.3; // Converte para tensão aproximada (em Volts)
  pressao = (tensao / 3.3) * 250;           // 250 bar é o valor que o sensor ler no máximo 250 bar

  unsigned long agora = millis(); // Pega o tempo atual em ms

  // Verifica se já passou o intervalo definido
  if (agora - ultimoTempo >= intervalo)
  {
    ultimoTempo = agora;    // Atualiza o "marcador de tempo"
    estadoLed = !estadoLed; // Inverte o estado do LED
    digitalWrite(LED, estadoLed);

    Serial.print("ADC: ");
    Serial.print(valorADC);
    Serial.print(" | Pressão: ");
    Serial.print(pressao, 3); // imprime com 3 casas decimais
    Serial.print(" bar ");
    Serial.print(heartbeat.getDataHoraFormatada());
    Serial.print(" | Epoch: ");
    Serial.println(heartbeat.getEpochTime());
  }

  // Verifica se já se passaram 10s
  if (millis() - lastPublish >= publishInterval)
  {
    publishMessage();
    lastPublish = millis(); // atualiza tempo da última publicação
  }

  client.loop();
}