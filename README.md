# Sistema de Monitoramento de Pressão com ESP32 e AWS IoT Core

Sistema de monitoramento em tempo real de pressão hidráulica utilizando ESP32, sensor XDB305 Series e integração com AWS IoT Core para coleta e armazenamento de dados.

## 📋 Visão Geral

Este projeto implementa um sistema completo de telemetria para monitoramento de pressão, realizando leituras do sensor XDB305 Series (4-20mA) através de um conversor de corrente para tensão HW-685, processando os dados no ESP32-ETH01 e enviando para a nuvem AWS via protocolo MQTT.

## 🔧 Hardware Necessário

- **ESP32-ETH01**: Microcontrolador com WiFi integrado
- **Sensor de Pressão XDB305 Series**: Sensor industrial com saída 4-20mA (0-250 bar)
- **Conversor HW-685**: Módulo conversor de corrente (4-20mA) para tensão (0-3.3V)
- **LED**: LED indicador no GPIO 2 (built-in)
- **Conexões**:
  - GPIO 36 (ADC1_CH0): Entrada analógica do conversor HW-685
  - GPIO 2: LED de status

## 📊 Especificações Técnicas

### Sensor XDB305 Series
- Faixa de medição: 0-250 bar
- Sinal de saída: 4-20mA
- Alimentação: 24V DC (típico)

### Conversor HW-685
- Entrada: 4-20mA
- Saída: 0-3.3V (compatível com ESP32)
- Conversão linear proporcional

### ESP32 ADC
- Resolução: 12 bits (0-4095)
- Tensão de referência: 3.3V
- Taxa de amostragem: 1 leitura/segundo (configurável)

## 🚀 Funcionalidades

- ✅ Leitura contínua do sensor de pressão via ADC
- ✅ Conversão automática de ADC para valores de pressão em bar
- ✅ Sincronização de tempo via NTP (Network Time Protocol)
- ✅ Publicação de dados no AWS IoT Core a cada 10 segundos
- ✅ Timestamp preciso em formato epoch e brasileiro (DD/MM/AAAA HH:MM:SS)
- ✅ LED indicador de operação
- ✅ Comunicação MQTT segura (TLS/SSL)
- ✅ Suporte a recebimento de comandos via MQTT

## 📦 Dependências

As bibliotecas necessárias estão definidas no arquivo `platformio.ini`:

```ini
lib_deps =
    arduino-libraries/NTPClient
    paulstoffregen/Time
    knolleary/PubSubClient@^2.8
    bblanchon/ArduinoJson
```

## ⚙️ Configuração

### 1. Configurar Credenciais WiFi e AWS

Edite o arquivo `secrets.h` com suas credenciais:

```cpp
#define THINGNAME "seu-dispositivo-iot"

const char WIFI_SSID[] = "Sua_Rede_WiFi";
const char WIFI_PASSWORD[] = "Sua_Senha_WiFi";

const char AWS_IOT_ENDPOINT[] = "xxxxx.iot.us-east-1.amazonaws.com";
```

### 2. Certificados AWS IoT

Substitua os certificados no `secrets.h` pelos certificados do seu dispositivo IoT:

- **AWS_CERT_CA**: Certificado root CA da Amazon
- **AWS_CERT_CRT**: Certificado do dispositivo
- **AWS_CERT_PRIVATE**: Chave privada do dispositivo (NUNCA compartilhe!)

Para obter os certificados:
1. Acesse o console AWS IoT Core
2. Crie uma "Thing" (dispositivo)
3. Baixe os certificados gerados
4. Cole o conteúdo nos campos correspondentes

### 3. Tópicos MQTT

O código utiliza os seguintes tópicos:

- **Publicação**: `pressure/sensor/data` (envia dados do sensor)
- **Assinatura**: `pressure/sensor/config` (recebe comandos)

## 📤 Formato dos Dados Publicados

Os dados são enviados em formato JSON:

```json
{
  "timestamp": 1727654400,
  "datetime": "29/09/2025 14:30:00",
  "ADC": 2048,
  "Pressao_bar": 125.5,
  "device_id": "sensor-pressao-01"
}
```

### Campos:
- `timestamp`: Tempo Unix epoch (usado como chave de partição no DynamoDB)
- `datetime`: Data/hora formatada (UTC-3 / horário de Brasília)
- `ADC`: Valor bruto do conversor A/D (0-4095)
- `Pressao_bar`: Pressão calculada em bar (0-250)
- `device_id`: Identificador único do dispositivo

## 🔄 Fluxo de Dados

```
Sensor XDB305 (4-20mA) 
    ↓
Conversor HW-685 (0-3.3V)
    ↓
ESP32 GPIO36 ADC (0-4095)
    ↓
Processamento (conversão para bar)
    ↓
MQTT + TLS → AWS IoT Core
    ↓
AWS IoT Rules / DynamoDB / CloudWatch
```

## 🛠️ Compilação e Upload

### Usando PlatformIO (VSCode)

```bash
# Compilar
pio run

# Fazer upload
pio run --target upload

# Monitorar serial
pio device monitor
```

### Ou use os botões da interface do PlatformIO no VSCode

## 📊 Monitoramento Serial

Exemplo de saída no monitor serial:

```
=== ESP32 Sensor de Pressão ===
Connecting to Wi-Fi
..........
WiFi conectado!
Sincronizando com servidor NTP...
NTP sincronizado com sucesso!
Connecting to AWS IOT
AWS IoT Connected!

ADC: 2048 | Pressão: 125.500 bar 29/09/2025 14:30:15 | Epoch: 1727654415
✓ Dados enviados com sucesso:
{"timestamp":1727654415,"datetime":"29/09/2025 14:30:15","ADC":2048,"Pressao_bar":125.5,"device_id":"sensor-pressao-01"}
```

## 🧮 Cálculo de Pressão

A conversão de ADC para pressão segue a fórmula:

```cpp
valorADC = analogRead(36);              // 0-4095
tensao = (valorADC / 4095.0) * 3.3;    // 0-3.3V
pressao = (tensao / 3.3) * 250;        // 0-250 bar
```

**Relação linear**:
- 0 ADC = 0V = 0 bar
- 4095 ADC = 3.3V = 250 bar

## 📂 Estrutura do Projeto

```
projeto-sensor-pressao/
├── src/
│   └── main.cpp              # Código principal
├── include/
│   ├── secrets.h             # Credenciais (NÃO commitar!)
│   └── Heartbeat.h           # Classe de sincronização NTP
├── lib/
│   └── Heartbeat/
│       └── Heartbeat.cpp     # Implementação do Heartbeat
├── platformio.ini            # Configurações do PlatformIO
└── README.md                 # Este arquivo
```

## 🔐 Segurança

- ⚠️ **NUNCA** faça commit do arquivo `secrets.h` com credenciais reais
- Adicione `secrets.h` ao `.gitignore`
- Use variáveis de ambiente em produção
- Mantenha as chaves privadas em local seguro
- Rotacione certificados periodicamente

## 🐛 Solução de Problemas

### WiFi não conecta
- Verifique SSID e senha no `secrets.h`
- Confirme que a rede é 2.4GHz (ESP32 não suporta 5GHz)

### Falha ao conectar no AWS IoT
- Verifique o endpoint AWS
- Confirme que os certificados estão corretos
- Verifique políticas (policies) anexadas ao certificado no console AWS

### Leituras de pressão incorretas
- Verifique conexões do conversor HW-685
- Confirme alimentação adequada do sensor XDB305
- Calibre o sensor se necessário

### NTP não sincroniza
- Verifique conexão com internet
- Tente outro servidor NTP (ex: `time.google.com`)
- Aguarde até 1 minuto para primeira sincronização

## 📝 Licença

Este projeto é de código aberto. Sinta-se livre para usar e modificar conforme necessário.

## 👥 Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para:
- Reportar bugs
- Sugerir melhorias
- Enviar pull requests

## 📧 Contato

Para dúvidas ou sugestões, abra uma issue no repositório.

---

**Desenvolvido com ESP32 + AWS IoT Core** 🚀
