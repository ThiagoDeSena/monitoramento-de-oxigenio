#include "Heartbeat.h"

Heartbeat::Heartbeat(const char *ntpServer, int timeZone, unsigned long updateInterval)
    : ntpUDP(),
      timeClient(ntpUDP, ntpServer, timeZone * 3600, updateInterval),
      isInitialized(false)
{
}

// chama no setup() para sincronizar inicialmente.
void Heartbeat::begin()
{
    timeClient.begin();

    // Tenta sincronizar pela primeira vez
    Serial.println("Sincronizando com servidor NTP...");
    int tentativas = 0;
    while (!timeClient.update() && tentativas < 10)
    {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    if (tentativas < 10)
    {
        isInitialized = true;
        Serial.println("\nNTP sincronizado com sucesso!");
        Serial.print("Hora atual: ");
        Serial.println(getDataHoraFormatada());
    }
    else
    {
        Serial.println("\nFalha na sincronização NTP!");
    }
}

// chama periodicamente no loop() para manter a hora atualizada.
bool Heartbeat::update()
{
    if (!isInitialized)
        return false;

    bool updated = timeClient.update();
    if (updated)
    {
        setTime(timeClient.getEpochTime());
    }
    return updated;
}

// Retorna a data e hora formatada como "DD/MM/AAAA HH:MM:SS"
String Heartbeat::getDataHoraFormatada()
{
    if (!isInitialized)
        return "00/00/0000 00:00:00";

    char buffer[32];
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
            day(), month(), year(),
            hour(), minute(), second());
    return String(buffer);
}

// Retorna a data e hora no formato ISO 8601 "AAAA-MM-DDTHH:MM:SSZ" para mandar dados para APIs/servidores
String Heartbeat::getDataHoraISO()
{
    if (!isInitialized)
        return "1970-01-01T00:00:00Z";

    char buffer[32];
    sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ",
            year(), month(), day(),
            hour(), minute(), second());
    return String(buffer);
}

// Retorna o timestamp epoch (segundos desde 1970)
unsigned long Heartbeat::getEpochTime()
{
    if (!isInitialized)
        return 0;
    return timeClient.getEpochTime();
}

// Verifica se a hora foi inicializada e é válida
bool Heartbeat::isTimeValid()
{
    return isInitialized && (getEpochTime() > 1000000000); // Timestamp válido após 2001
}