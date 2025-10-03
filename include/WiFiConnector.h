#ifndef WIFI_CONNECTOR_H
#define WIFI_CONNECTOR_H

#include <WiFi.h>

class WiFiConnector
{
private:
    const char *ssid;
    const char *password;
    int ledPinRed;   // Indicação de falha na conexão WiFi
    int ledPinGreen; // Indicação de conexão bem-sucedida

public:
    WiFiConnector(const char *ssid, const char *password, int ledPinRed, int ledPinGreen);
    void connect();          // Conecta ao WiFi
    void ensureConnection(); // Garante que a conexão WiFi esteja ativa durante o loop
    IPAddress getLocalIP();  // Retorna o IP local
};

#endif
