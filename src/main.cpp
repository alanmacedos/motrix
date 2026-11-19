
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// --------------------------------------------------
// CONFIGURAÇÃO DO ACCESS POINT
// --------------------------------------------------

const char *ssid = "MOTRIX_RC01";
const char *password = "motrix123";

// --------------------------------------------------
// SERVIDORES
// --------------------------------------------------

WebServer server(80);
WebSocketsServer webSocket(81);

// --------------------------------------------------
// EVENTOS DO WEBSOCKET
// --------------------------------------------------

void webSocketEvent(
    byte num,
    WStype_t type,
    uint8_t *payload,
    size_t length)
{
    switch (type)
    {

    case WStype_DISCONNECTED:

        Serial.printf("Cliente %u desconectado\n", num);
        break;

    case WStype_CONNECTED:

        Serial.printf("Cliente %u conectado\n", num);
        break;

    case WStype_TEXT:

        Serial.println("\n========== WEBSOCKET ==========");

        // Cria documento JSON
        JsonDocument doc;

        // Interpreta a mensagem recebida
        DeserializationError erro = deserializeJson(
            doc,
            payload,
            length);

        // Verifica se o JSON é válido
        if (erro)
        {

            Serial.print("Erro ao interpretar JSON: ");
            Serial.println(erro.c_str());

            webSocket.sendTXT(
                num,
                "{\"type\":\"error\",\"message\":\"JSON invalido\"}");

            break;
        }

        // Identifica o tipo de mensagem
        const char *messageType = doc["type"] | "";

        // Verifica se é um comando de controle
        if (strcmp(messageType, "control") != 0)
        {

            Serial.println("Tipo de mensagem desconhecido!");

            break;
        }

        // Verifica a presença dos campos
        if (
            !doc["steering"].is<int>() ||
            !doc["throttle"].is<int>() ||
            !doc["reverse"].is<int>() ||
            !doc["lights"].is<bool>())
        {

            Serial.println("Campos ausentes ou inválidos!");

            break;
        }

        int steering = doc["steering"];
        int throttle = doc["throttle"];
        int reverse = doc["reverse"];

        const char* traction = doc["traction"] | "nenhuma";

        // Valida os limites dos controles
        if (
            steering < -100 || steering > 100 ||
            throttle < -100 || throttle > 100 ||
            reverse < -100 || reverse > 100
        ) {

            Serial.println("Valores fora dos limites!");
            
            break;
        }

        // Responde ao cliente
        bool enviado = webSocket.sendTXT(
            num,
            "{\"type\":\"ack\", \"message\":\"Menssage received by MOTRIX!\"}");

        if (enviado)
        {
            Serial.printf("ACK enviado");
        }

        if (!enviado)
        {
            Serial.printf("ACK não enviado");
        }

        break;
    }
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup()
{

    // BAUND
    Serial.begin(9600);

    // Inicializa o sistema de arquivos
    if (!LittleFS.begin(true))
    {
        Serial.println("Erro ao inicializar LittleFS!");
        return;
    }

    Serial.println("LittleFS inicializado com sucesso!");

    // Configura o ESP32 como Access Point
    WiFi.mode(WIFI_AP);

    // Define o IP do Access Point
    WiFi.softAPConfig(
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0));

    // Cria a rede Wi-Fi
    if (!WiFi.softAP(ssid, password))
    {
        Serial.println("Erro ao iniciar Access Point!");
        return;
    }

    Serial.println("Access Point iniciado!");
    Serial.print("Rede: ");
    Serial.println(ssid);

    Serial.print("IP do Motrix: ");
    Serial.println(WiFi.softAPIP());

    // --------------------------------------------------
    // SERVIDOR HTTP
    // --------------------------------------------------

    // Disponibiliza os arquivos da pasta LittleFS
    // e define index.html como página inicial.
    server.on("/", HTTP_GET, []()
              {
    File file = LittleFS.open("/index.html", "r");

    if (!file) {
        server.send(404, "text/plain", "index.html nao encontrado");
        return;
    }

    server.streamFile(file, "text/html");
    file.close(); });

    server.serveStatic("/", LittleFS, "/");

    server.begin();

    Serial.println("Servidor HTTP iniciado!");

    // --------------------------------------------------
    // SERVIDOR WEBSOCKET
    // --------------------------------------------------

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    Serial.println("Servidor WebSocket iniciado!");
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop()
{

    server.handleClient();

    webSocket.loop();
}
