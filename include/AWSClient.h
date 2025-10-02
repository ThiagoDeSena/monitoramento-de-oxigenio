// AWSClient.h
#ifndef AWSCLIENT_H
#define AWSCLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Heartbeat.h"
#include "secrets.h"

class AWSClient
{
private:
    WiFiClientSecure net;
    PubSubClient client;
    Heartbeat *heartbeat;
    String deviceId;
    unsigned long lastPublish; // guarda o último tempo em que publicou
    long publishInterval;      // intervalo de publicação da mensagem

    // Handler de mensagens recebidas
    static void messageHandlerStatic(char *topic, byte *payload, unsigned int length);
    void messageHandler(char *topic, byte *payload, unsigned int length);

    // Ponteiro estático para manter referência da instância
    static AWSClient *instance;

public:
    // Construtor
    AWSClient(Heartbeat *hb, const char *thingName, long pubInterval = 10000);

    // Métodos públicos
    void connectAWS();
    void loop();
    void publishMessage(int valorADC, float pressao);
    bool isConnected();
    unsigned long getLastPublish() const;
    long getPublishInterval() const;
};

#endif