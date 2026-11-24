
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// --------------------------------------------------
// ACCESS POINT
// --------------------------------------------------

const char *ssid = "MOTRIX_RC01";
const char *password = "motrix123";

// --------------------------------------------------
// SERVERS
// --------------------------------------------------

WebServer server(80);
WebSocketsServer webSocket(81);

// --------------------------------------------------
// FAILSAFE
// --------------------------------------------------

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;

bool hasReceivedCommand = false;
bool timeoutActive = false;

// --------------------------------------------------
// LAST CONTROL STATE
// --------------------------------------------------

struct ControlState
{
    int steering = 0;
    int throttle = 0;
    int reverse = 0;
    char traction[4] = "";
    bool lights = false;
};

ControlState lastControl;
bool hasPreviousControl = false;

// --------------------------------------------------
// WEBSOCKET EVENTS
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
    {
        JsonDocument doc;

        DeserializationError error = deserializeJson(
            doc,
            payload,
            length);

        if (error)
        {
            Serial.print("Erro ao interpretar JSON: ");
            Serial.println(error.c_str());
            break;
        }

        const char *messageType = doc["type"] | "";

        if (strcmp(messageType, "control") != 0)
        {
            Serial.println("Tipo de mensagem desconhecido!");
            break;
        }

        // Validate fields
        if (
            !doc["steering"].is<int>() ||
            !doc["throttle"].is<int>() ||
            !doc["reverse"].is<int>() ||
            !doc["traction"].is<const char *>() ||
            !doc["lights"].is<bool>())
        {
            Serial.println("Campos ausentes ou invalidos!");
            break;
        }

        // Read commands
        int steering = doc["steering"];
        int throttle = doc["throttle"];
        int reverse = doc["reverse"];
        const char *traction = doc["traction"];
        bool lights = doc["lights"];

        // Validate limits
        if (
            steering < -100 || steering > 100 ||
            throttle < 0 || throttle > 100 ||
            reverse < 0 || reverse > 100)
        {
            Serial.println("Valores fora dos limites!");
            break;
        }

        // Validate traction
        if (
            strcmp(traction, "4x4") != 0 &&
            strcmp(traction, "fwd") != 0 &&
            strcmp(traction, "rwd") != 0)
        {
            Serial.println("Modo de tracao invalido!");
            break;
        }

        // Update communication timer
        lastCommandTime = millis();
        hasReceivedCommand = true;
        timeoutActive = false;

        // Check whether the control state changed
        bool controlChanged =
            !hasPreviousControl ||
            steering != lastControl.steering ||
            throttle != lastControl.throttle ||
            reverse != lastControl.reverse ||
            strcmp(traction, lastControl.traction) != 0 ||
            lights != lastControl.lights;

        // Print only when a value changes
        if (controlChanged)
        {
            Serial.println("\n========== MOTRIX CONTROL ==========");

            Serial.printf("Direção: %d\n", steering);
            Serial.printf("Aceleração: %d\n", throttle);
            Serial.printf("Ré: %d\n", reverse);
            Serial.printf("Tração: %s\n", traction);

            Serial.printf(
                "Faróis: %s\n",
                lights ? "Ligados" : "Desligados");

            Serial.println("====================================\n");

            // Save current state
            lastControl.steering = steering;
            lastControl.throttle = throttle;
            lastControl.reverse = reverse;

            strncpy(
                lastControl.traction,
                traction,
                sizeof(lastControl.traction) - 1);

            lastControl.traction[
                sizeof(lastControl.traction) - 1] = '\0';

            lastControl.lights = lights;

            hasPreviousControl = true;
        }

        break;
    }

    default:
        break;
    }
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup()
{
    Serial.begin(9600);

    // Initialize LittleFS
    if (!LittleFS.begin(true))
    {
        Serial.println("Erro ao inicializar LittleFS!");
        return;
    }

    Serial.println("LittleFS inicializado com sucesso!");

    // Configure Access Point
    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0));

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

    // HTTP SERVER

    server.on("/", HTTP_GET, []()
    {
        File file = LittleFS.open("/index.html", "r");

        if (!file)
        {
            server.send(
                404,
                "text/plain",
                "index.html nao encontrado");

            return;
        }

        server.streamFile(file, "text/html");
        file.close();
    });

    server.serveStatic("/", LittleFS, "/");

    server.begin();

    Serial.println("Servidor HTTP iniciado!");

    // WEBSOCKET SERVER

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

    // Failsafe
    if (
        hasReceivedCommand &&
        millis() - lastCommandTime > COMMAND_TIMEOUT &&
        !timeoutActive)
    {
        timeoutActive = true;

        Serial.println("Connection timeout!");
    }
}
