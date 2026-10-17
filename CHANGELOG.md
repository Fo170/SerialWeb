# Changelog - SerialWeb

Toutes les évolutions notables de cette librairie sont documentées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/),
versionnement [SemVer](https://semver.org/lang/fr/).

---

## [1.1.1] - 2026-10-17

### ✨ Added — macro de version `__<LIB>_H_VERSION__`

Chaque header déclare désormais sa version via une macro préprocesseur,
source **unique** (un projet hôte peut l'agréger, ex. page `/version`).

### 🏷️ Divers

- Version `1.1.0` → `1.1.1` (`library.json` + `library.properties`)

---

## [1.1.0] - 2026-10-17

### ✨ Added — `setLogSink()` : miroir de log vers un canal secondaire

Permet de **brancher un canal de log secondaire** qui reçoit une copie de tout ce
qui passe par `sw.print()` / `sw.println()` / `sw.printf()` — **en plus** de l'USB
et du WebSocket.

Cas d'usage typique : envoyer tous les logs vers un serveur **Syslog UDP**
(RFC 5424, ex. NAS OMV) sans dupliquer les appels de log dans le firmware.

```cpp
#include <Syslog_UDP.h>
Syslog_UDP syslog;

void setup() {
  Serial.begin(115200);
  syslog.begin("192.168.1.209", 5514, "coulometre");

  sw.init();                                   // buffer + WebSocket
  sw.setLogSink([](const char* l){             // ← miroir
    syslog.log(6 /* INFO */, l);               // fire-and-forget, non bloquant
  });
}
```

| API | Rôle |
|---|---|
| `typedef void (*LogSinkFn)(const char*)` | Signature du sink |
| `void setLogSink(LogSinkFn fn)` | Installe le miroir ; `nullptr` = désactive |

### 🔧 Implémentation

- Le sink est appelé dans `doLog()` — le point unique par lequel passent
  **toutes** les écritures (`print`, `println`, `printf`, `write`) :

  ```cpp
  void doLog(const char* text) {
      Serial.print(text);        // USB (toujours)
      if (_sink) _sink(text);    // miroir (ex. → Syslog_UDP) — ne bloque pas
      if (lines) { /* ring buffer WebSocket */ }
  }
  ```
- Coût : **1 pointeur de fonction** (`_sink`, 4 o) et un test de nullité par
  écriture. Aucune allocation, aucun buffer supplémentaire.

### ⚠️ Contraintes du sink (important)

- Il est appelé **depuis le contexte d'écriture** (parfois depuis un handler
  WebSocket asynchrone) : il ne doit **NI bloquer** (pas d'I/O synchrone longue,
  pas de `delay()`), **NI rappeler** `sw.*` (récursion infinie).
- Un sink « fire-and-forget » (file interne bornée, ex. `Syslog_UDP`) est le bon
  usage. Un envoi HTTP synchrone bloquerait la boucle principale.
- Le sink doit être **rapide** : il s'exécute à chaque ligne journalisée.

### 🏷️ Divers

- Ajout de la macro de version **`__SERIALWEB_H_VERSION__`** (source unique,
  agrégée par `liste_lib.h` côté projet hôte) — `library.json` passe en 1.1.0.

---

## [1.0.0] - 2026-10-02

### ✨ Added — Version initiale

- Moniteur série **WebSocket** sur `/serialws` (miroir de la sortie USB).
- **Backlog borné** en RAM (ring buffer de lignes, capacité configurable au
  constructeur, défaut 24 ; le projet Coulometre utilise 2 pour la RAM).
- **Commandes clavier** depuis le terminal web :
  `Z` (zéro), `S` (sync), `R` (reset), `C <volts>` (calibration),
  `P` (status), `D` (debug), `F` (force save).
- Callback `onCommand` pour déléguer le traitement au firmware hôte.
- Écritures via `print()` / `println()` / `printf()` / `write()`, redirigées
  simultanément vers l'USB et le WebSocket.
