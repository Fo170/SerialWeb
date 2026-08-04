/*
 * SerialWeb - Example Basic
 * https://github.com/Fo170/SerialWeb
 *
 * Ouvre un serveur HTTP + un moniteur série WebSocket.
 * À la connexion, le client reçoit le backlog des logs.
 * Toute commande WS reçue est transmise au callback.
 */

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SerialWeb.h>

const char* SSID     = "VOTRE_SSID";
const char* PASSWORD = "VOTRE_MOT_DE_PASSE";

AsyncWebServer server(80);

// Callback appelé à chaque message WebSocket reçu (ex: Z, S, P...)
void handleSerialCommand(const String& cmd) {
    sw.printf("[CMD] Reçu : %s\n", cmd.c_str());
    if (cmd == "P") {
        sw.printf("Heap libre : %d bytes\n", ESP.getFreeHeap());
    }
}

SerialWeb sw(&server, handleSerialCommand);

void setup() {
    Serial.begin(115200);
    delay(200);

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connecté");
    Serial.println(WiFi.localIP());

    server.begin();

    // Doit être appelé APRÈS server.begin() (mais peut l'être avant,
    // addHandler est indépendant de begin())
    sw.init();
    sw.println("=== SerialWeb prêt ===");
    sw.printf("IP : %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
    static uint32_t last = 0;
    if (millis() - last >= 5000) {
        last = millis();
        sw.printf("[HEARTBEAT] uptime=%lus heap=%d\n",
                  (unsigned long)(millis() / 1000), ESP.getFreeHeap());
    }
}
