#include <pgmspace.h>
#define SECRET
#define THINGNAME "SEU_THINGNAME_AQUI"

// Configurações WiFi
const char WIFI_SSID[] = "SEU_WIFI_SSID_AQUI";
const char WIFI_PASSWORD[] = "SUA_SENHA_WIFI_AQUI";

// Endpoint AWS IoT - substitua pelo seu endpoint
const char AWS_IOT_ENDPOINT[] = "SEU_ENDPOINT_AWS_AQUI.iot.regiao.amazonaws.com";

// Amazon Root CA 1
// Substitua pelo certificado CA da AWS IoT
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
COLE_AQUI_O_CERTIFICADO_CA_DA_AWS
-----END CERTIFICATE-----
)EOF";

// Device Certificate
// Substitua pelo certificado do seu dispositivo AWS IoT
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
COLE_AQUI_O_CERTIFICADO_DO_SEU_DISPOSITIVO
-----END CERTIFICATE-----
)KEY";

// Device Private Key
// SUBSTITUA PELA SUA PRIVATE KEY - NUNCA COMPARTILHE ESTA CHAVE!
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
COLE_AQUI_SUA_PRIVATE_KEY_MUITO_SECRETA
-----END RSA PRIVATE KEY-----
)KEY";