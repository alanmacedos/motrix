
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>

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
    size_t length
) {
    switch (type) {

        case WStype_DISCONNECTED:
            Serial.printf("Cliente %u desconectado\n", num);
            break;

        case WStype_CONNECTED:
            Serial.printf("Cliente %u conectado\n", num);
            break;

        case WStype_TEXT:
            Serial.printf("Mensagem recebida do cliente %u: ", num);
            Serial.write(payload, length);
            Serial.println();
            break;
    }
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

    // BAUND
    Serial.begin(9600);

    // Inicializa o sistema de arquivos
    if (!LittleFS.begin(true)) {
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
        IPAddress(255, 255, 255, 0)
    );

    // Cria a rede Wi-Fi
    if (!WiFi.softAP(ssid, password)) {
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
    server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");

    if (!file) {
        server.send(404, "text/plain", "index.html nao encontrado");
        return;
    }

    server.streamFile(file, "text/html");
    file.close();
});

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

void loop() {

    server.handleClient();

    webSocket.loop();

}
