#include "WiFiConnector.h"
#include <Arduino.h>

WiFiConnector::WiFiConnector(const char *ssid, const char *password, int ledPinRed, int ledPinGreen)
{
    this->ssid = ssid;
    this->password = password;
    this->ledPinRed = ledPinRed;
    this->ledPinGreen = ledPinGreen;
}

// Conecta ao WiFi
void WiFiConnector::connect()
{
    pinMode(ledPinRed, OUTPUT);
    pinMode(ledPinGreen, OUTPUT);
    digitalWrite(ledPinRed, HIGH); // Liga o LED Vermelho indicando que está tentando conectar

    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20)
    {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(ledPinRed, LOW);    // Desliga o LED Vermelho se a conexão estiver estabelecida
        digitalWrite(ledPinGreen, HIGH); // Conexão estabelecida
        Serial.println();
        Serial.println("WiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        digitalWrite(ledPinRed, HIGH);
        digitalWrite(ledPinGreen, LOW);
        Serial.println("\nFalha ao conectar no WiFi.");
    }
}

IPAddress WiFiConnector::getLocalIP()
{
    return WiFi.localIP();
}

// Garante que a conexão WiFi esteja ativa durante o loop
void WiFiConnector::ensureConnection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WiFi] Conexão perdida. Tentando reconectar...");

        WiFi.begin(ssid, password);

        bool ledState = false;
        int tentativas = 0;
        while (WiFi.status() != WL_CONNECTED && tentativas < 10)
        {
            ledState = !ledState;
            digitalWrite(ledPinRed, ledState); // pisca o LED vermelho indicando tentativa de reconexão
            delay(500);
            Serial.print(".");
            tentativas++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\n[WiFi] Reconectado com sucesso.");
            digitalWrite(ledPinRed, LOW);
            digitalWrite(ledPinGreen, HIGH);
        }
        else
        {
            Serial.println("\n[WiFi] Falha na reconexão.");
            digitalWrite(ledPinRed, HIGH);
            digitalWrite(ledPinGreen, LOW);
        }
    }
}
