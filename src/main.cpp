#include "secrets.h"
#include "WiFi.h"
#include "Heartbeat.h"
#include "AWSClient.h"
#include <DHT.h>

#define LED 21
#define DHT_PIN 15     // GPIO onde o DHT11 está conectado
#define DHT_TYPE DHT11 // Tipo do sensor

const int analogPin = 36;       // GPIO36 (ADC1_CH0)
unsigned long intervalo = 1000; // Intervalo em milissegundos (1000ms = 1s)
unsigned long ultimoTempo = 0;  // Armazena a última vez que o LED mudou de estado
bool estadoLed = LOW;           // Estado atual do LED

float pressao;
int valorADC;
float temperatura = 0;
float umidade = 0;

// Instâncias das classes
Heartbeat heartbeat("pool.ntp.org", -3, 60000);
AWSClient awsClient(&heartbeat, THINGNAME, 1000 * 60 * 2); // passa ponteiro do heartbeat, thingname e intervalo
DHT dht(DHT_PIN, DHT_TYPE);                                // Instância do sensor DHT

void connectWiFi()
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
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Inicializa o heartbeat após conectar ao WiFi
    heartbeat.begin();
}

void lerSensorDHT()
{
    // Lê a temperatura em Celsius
    float temp = dht.readTemperature();
    // Lê a umidade
    float hum = dht.readHumidity();

    // Verifica se as leituras são válidas
    if (isnan(temp) || isnan(hum))
    {
        Serial.println("❌ Falha na leitura do sensor DHT!");
        return;
    }

    temperatura = temp;
    umidade = hum;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    // Inicializa o sensor DHT
    dht.begin();
    Serial.println("=== ESP32 Sensor de Pressão ===");

    // Conecta ao WiFi
    connectWiFi();

    // Conecta ao AWS IoT (agora usando a classe)
    awsClient.connectAWS();
    // Verifica se conectou com sucesso
    if (awsClient.isConnected())
    {
        Serial.println("✓ Sistema pronto para operar");
    }
    else
    {
        Serial.println("✗ Falha na inicialização do sistema");
    }
}

void loop()
{
    // Verifica e reconecta se perdeu conexão MQTT
    if (!awsClient.isConnected())
    {
        Serial.println("⚠ Conexão MQTT perdida, tentando reconectar...");
        awsClient.connectAWS();
    }

    // valorADC = analogRead(analogPin);         // Lê valor entre 0 e 4095
    // float tensao = (valorADC / 4095.0) * 3.3; // Converte para tensão aproximada (em Volts)
    // pressao = (tensao / 3.3) * 250;           // 250 bar é o valor que o sensor ler no máximo 250 bar

    unsigned long agora = millis(); // Pega o tempo atual em ms

    // Verifica se já passou o intervalo definido
    if (agora - ultimoTempo >= intervalo)
    {
        ultimoTempo = agora;    // Atualiza o "marcador de tempo"
        estadoLed = !estadoLed; // Inverte o estado do LED
        digitalWrite(LED, estadoLed);

        // Lê os dados do DHT11 a cada segundo
        lerSensorDHT();
        // Serial.print("ADC: ");
        // Serial.print(valorADC);
        // Serial.print(" | Pressão: ");
        // Serial.print(pressao, 3); // imprime com 3 casas decimais
        // Serial.print(" bar ");
        Serial.print("Temp: ");
        Serial.print(temperatura);
        Serial.print("°C");
        Serial.print(" | Umidade: ");
        Serial.print(umidade);
        Serial.print("% ");
        Serial.print(heartbeat.getDataHoraFormatada());
        Serial.print(" | Epoch: ");
        Serial.println(heartbeat.getEpochTime());
    }

    // Verifica se já se passaram 10s e publica usando a classe AWSClient
    if (millis() - awsClient.getLastPublish() >= awsClient.getPublishInterval())
    {
        // awsClient.publishMessage(valorADC, pressao); // Usa o método da classe
        awsClient.publishMessageDHT(temperatura, umidade); // Usa o método da classe
    }

    // Mantém a conexão MQTT ativa (substitui o client.loop())
    awsClient.loop();
}