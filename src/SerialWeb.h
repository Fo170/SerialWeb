// ============================================================
// SerialWeb.h — Moniteur série WebSocket (bibliothèque autonome)
// https://github.com/Fo170/SerialWeb
// - Buffer circulaire de logs (heap, lignes × 64 chars)
// - Chaque log : écrit sur Serial (USB) + buffer + broadcast WS
// - /serialws : à la connexion → backlog ; messages = commandes
// - le WS diffuse du texte seulement.
// - Dépendances INJECTÉES (pas de globals extern) :
//     * AsyncWebServer* (à passer au constructeur)
//     * callback de commande void(*)(const String&) (optionnel)
// ============================================================

#ifndef SERIAL_WEB_H
#define SERIAL_WEB_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class SerialWeb {
private:
    AsyncWebServer* srv = nullptr;
    AsyncWebSocket  ws;
    void (*onCommand)(const String&) = nullptr;

    struct LogLine { uint32_t ts; char text[64]; };
    LogLine* lines = nullptr;
    uint8_t  capacity;
    uint8_t  head = 0;
    uint8_t  count = 0;

    void doLog(const char* text) {
        Serial.print(text);   // USB (toujours)
        if (lines) {
            LogLine& l = lines[head];
            l.ts = millis();
            strncpy(l.text, text, sizeof(l.text) - 1);
            l.text[sizeof(l.text) - 1] = 0;
            head = (head + 1) % capacity;
            if (count < capacity) count++;
        }
        if (ws.count() > 0) {
            ws.textAll(text);
        }
    }

public:
    // srvPtr   : AsyncWebServer à utiliser (addHandler interne)
    // cmdHandler : callback appelé pour chaque message WS (optionnel)
    // wsPath   : chemin WebSocket (défaut "/serialws")
    // bufLines : nombre de lignes du backlog circulaire (défaut 24)
    SerialWeb(AsyncWebServer* srvPtr,
              void (*cmdHandler)(const String&) = nullptr,
              const char* wsPath = "/serialws",
              uint8_t bufLines = 24)
        : srv(srvPtr), ws(wsPath), onCommand(cmdHandler),
          capacity(bufLines ? bufLines : 24) {}

    // Alloue le buffer + enregistre le handler WS. À appeler dans setup().
    void init() {
        if (!srv) return;
        lines = new (std::nothrow) LogLine[capacity];

        ws.onEvent([this](AsyncWebSocket* s, AsyncWebSocketClient* c,
                          AwsEventType type, void* arg, uint8_t* data, size_t len) {
            (void)s; (void)arg;
            if (type == WS_EVT_CONNECT) {
                // Backlog des dernières lignes
                uint8_t n = count;
                uint8_t start = (count < capacity) ? 0 : head;
                for (uint8_t i = 0; i < n; i++) {
                    uint8_t idx = (start + i) % capacity;
                    if (lines[idx].text[0]) c->text(lines[idx].text);
                }
                c->text("--- Connecté au moniteur ---\n");
            } else if (type == WS_EVT_DATA) {
                if (onCommand) {
                    String cmd = String((char*)data).substring(0, len);
                    cmd.trim();
                    if (cmd.length() > 0) onCommand(cmd);
                }
            }
        });
        srv->addHandler(&ws);
    }

    // Miroir de l'API Serial (printf/println/print) → USB + WS
    void printf(const char* fmt, ...) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        doLog(buf);
    }

    void println(const char* fmt, ...) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        doLog(buf);
        doLog("\n");
    }

    void println(const String& s) { doLog(s.c_str()); doLog("\n"); }
    void println()                { doLog("\n"); }
    void print(const char* s)     { doLog(s); }
    void print(const String& s)   { doLog(s.c_str()); }
};

// Instance globale par défaut (convention Fo170, cf. t_fct.h).
// À DÉFINIR dans le firmware, ex : `SerialWeb sw(&server, handleSerialCommand);`
// Sans `extern` ci-dessous, les utilisateurs qui ne référencent pas `sw` sont libres.
extern SerialWeb sw;

#endif // SERIAL_WEB_H
