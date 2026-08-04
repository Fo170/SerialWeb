# AGENTS.md

SerialWeb: single-header Arduino library (`src/SerialWeb.h`) exposing a WebSocket serial monitor for ESP8266/ESP32. All source comments and docs (README) are in **French** — keep new text consistent.

## Build / test

- No build, test, lint, or CI setup exists in the repo. There is no `platformio.ini`; examples are Arduino-IDE `.ino` sketches. Verification happens by compiling against PlatformIO/Arduino IDE locally — don't invent commands that don't exist.
- Not a git repository yet (`.gitignore` exists but no `.git/`).

## Architecture

- **Header-only.** Everything lives in `src/SerialWeb.h`; `SerialWeb` is a single class. There is no `.cpp`.
- **No global dependencies:** `AsyncWebServer*` and the command callback are injected via the constructor. Do not add `extern`-based globals.
- `extern SerialWeb sw;` at the end of the header is only a *declaration* — the firmware (not the library) must define the instance (`SerialWeb sw(&server, callback);`). Don't add a definition to the library, and keep the extern if you add a new global-free default.
- Data flow: every `print`/`println`/`printf` writes to USB `Serial`, a heap circular backlog (`LogLine{ts, char[64]}`, default 24 lines), then broadcasts to WS clients. On `WS_EVT_CONNECT` the backlog is replayed.
- `doLog` truncates lines to 63 chars (`strncpy` into `char[64]`); `printf`/`println` format into a 128-byte stack buffer first.
- `init()` must be called in `setup()`; it allocates the buffer on the heap and registers the WS handler via `srv->addHandler(&ws)`.
- WS `onEvent` callback context is async — never call `yield()`/`delay()`/blocking I²C there. WS data frames are treated as command strings (trimmed, empty dropped) forwarded to the injected callback.

## Conventions

- Dependency is `ESPAsyncWebServer` only (declared in `library.properties` and `library.json`). Don't introduce other deps.
- When adding/renaming methods or the class, update `keywords.txt` (Arduino IDE syntax highlighting) and the API table in `README.md`.
- When changing metadata (version, architecture, dependency), keep `library.properties`, `library.json`, and `README.md` in sync — the three currently match (v1.0.0, esp8266+esp32, GPL-3.0-only).
