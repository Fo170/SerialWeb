# SerialWeb

Moniteur série **WebSocket** pour ESP8266 / ESP32 (ESPAsyncWebServer).

Chaque appel `printf` / `println` / `print` est dupliqué sur :
1. **USB Serial** (toujours),
2. un **buffer circulaire** en heap (backlog, défaut 24 lignes),
3. un **broadcast WebSocket** aux clients connectés.

À la connexion, le client reçoit le backlog des dernières lignes. Les messages envoyés par le client sont transmis à un **callback de commande** (ex. `Z`, `S`, `R`, `P`…).

## Caractéristiques

- Header-only, aucune allocation statique du monde : le buffer est alloué en heap dans `init()`.
- **Zéro dépendance globale** : `AsyncWebServer*` et le callback de commande sont injectés au constructeur (pas d'`extern`).
- Sécurité : aucun `yield()` / `delay()` / I²C dans le contexte WS (texte uniquement).

## Installation (PlatformIO)

```ini
lib_deps =
    https://github.com/Fo170/SerialWeb.git
```

## Utilisation

```cpp
#include <ESPAsyncWebServer.h>
#include <SerialWeb.h>

AsyncWebServer server(80);
SerialWeb      sw(&server);          // sans callback : commandes ignorées

void setup() {
    Serial.begin(115200);
    server.begin();
    sw.init();                       // buffer + handler WS
    sw.println("Hello depuis SerialWeb !");
}
```

### Avec callback de commande

```cpp
void handleSerialCommand(const String& cmd) {
    if (cmd == "P") {
        sw.printf("Heap libre : %d bytes\n", ESP.getFreeHeap());
    }
}

SerialWeb sw(&server, handleSerialCommand);
```

## API

| Méthode | Description |
|---|---|
| `SerialWeb(AsyncWebServer* srv, void(*cmd)(const String&)=nullptr, const char* path="/serialws", uint8_t bufLines=24)` | Constructeur |
| `init()` | Alloue le buffer + enregistre le handler WS (dans `setup()`) |
| `printf(fmt, ...)` | Log formaté → USB + backlog + WS |
| `println(fmt, ...)` / `println(String)` / `println()` | Log ligne → USB + backlog + WS |
| `print(const char*)` / `print(String)` | Log brut → USB + backlog + WS |

## Exemple

Voir `examples/basic/basic.ino`.

## Licence

GPL-3.0-only — voir `LICENSE`.
