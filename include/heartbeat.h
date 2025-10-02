// Heartbeat.h
#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

class Heartbeat
{
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    bool isInitialized;

public:
    // Construtor
    Heartbeat(const char *ntpServer = "pool.ntp.org", int timeZone = -3, unsigned long updateInterval = 60000);

    // Métodos públicos
    void begin();
    bool update();
    String getDataHoraFormatada();
    String getDataHoraISO(); // Formato ISO para DynamoDB
    unsigned long getEpochTime();
    bool isTimeValid();
};

#endif