// AWSClient.cpp
#include "AWSClient.h"

// Inicializa ponteiro estático
AWSClient *AWSClient::instance = nullptr;

// Construtor
AWSClient::AWSClient(Heartbeat *hb, const char *thingName, long pubInterval)
    : net(),
      client(net),
      heartbeat(hb),
      deviceId(thingName),
      lastPublish(0),
      publishInterval(pubInterval)
{
    instance = this; // Mantém instância acessível no callback estático
}

// Conecta ao AWS IoT Core
void AWSClient::connectAWS()
{
    // Configura certificados AWS IoT
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Configura servidor MQTT
    client.setServer(AWS_IOT_ENDPOINT, 8883);
    client.setCallback(AWSClient::messageHandlerStatic);

    Serial.println("Conectando ao AWS IoT Core...");

    while (!client.connected())
    {
        if (client.connect(deviceId.c_str()))
        {
            Serial.println("AWS IoT Conectado!");
            client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
        }
        else
        {
            Serial.print(".");
            delay(500);
        }
    }
}

// Publica mensagem MQTT
void AWSClient::publishMessage(int valorADC, float pressao)
{
    // Atualiza o heartbeat
    heartbeat->update();

    // Verifica se o tempo NTP é válido
    if (!heartbeat->isTimeValid())
    {
        Serial.println("Tempo NTP inválido, pulando publicação...");
        return;
    }

    // Monta o JSON
    StaticJsonDocument<400> doc;
    doc["timestamp"] = heartbeat->getEpochTime();
    doc["datetime"] = heartbeat->getDataHoraFormatada();
    doc["ADC"] = valorADC;
    doc["Pressao_bar"] = pressao;
    doc["device_id"] = deviceId;

    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);

    // Publica no tópico AWS
    bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

    if (published)
    {
        Serial.println("✓ Dados enviados com sucesso:");
        Serial.println(jsonBuffer);
        lastPublish = millis(); // Atualiza o tempo do último envio
    }
    else
    {
        Serial.println("✗ Falha ao enviar dados MQTT");
    }
}

// Publica mensagem MQTT
void AWSClient::publishMessageDHT(float temperatura, float umidade)
{
    // Atualiza o heartbeat
    heartbeat->update();

    // Verifica se o tempo NTP é válido
    if (!heartbeat->isTimeValid())
    {
        Serial.println("Tempo NTP inválido, pulando publicação...");
        return;
    }

    // Monta o JSON
    StaticJsonDocument<400> doc;
    doc["timestamp"] = heartbeat->getEpochTime();
    doc["datetime"] = heartbeat->getDataHoraFormatada();
    doc["Temperatura"] = temperatura;
    doc["Umidade"] = umidade;
    doc["device_id"] = deviceId;

    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);

    // Publica no tópico AWS
    bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

    if (published)
    {
        Serial.println("✓ Dados enviados com sucesso:");
        Serial.println(jsonBuffer);
        lastPublish = millis(); // Atualiza o tempo do último envio
    }
    else
    {
        Serial.println("✗ Falha ao enviar dados MQTT");
    }
}

// Deve ser chamado periodicamente no loop()
void AWSClient::loop()
{
    client.loop();
}

// Verifica se está conectado
bool AWSClient::isConnected()
{
    return client.connected();
}

// Getters
unsigned long AWSClient::getLastPublish() const
{
    return lastPublish;
}

// Intervalo de publicação em ms
long AWSClient::getPublishInterval() const
{
    return publishInterval;
}

// Callback estático -> chama o método da instância
void AWSClient::messageHandlerStatic(char *topic, byte *payload, unsigned int length)
{
    if (instance)
    {
        instance->messageHandler(topic, payload, length);
    }
}

// Handler de mensagens recebidas
void AWSClient::messageHandler(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Mensagem recebida em: ");
    Serial.println(topic);

    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    const char *message = doc["message"];

    Serial.print("Conteúdo: ");
    Serial.println(message);
}