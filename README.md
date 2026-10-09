# DonemosDesktop

Qt 6 Widgets desktop client for the **Donemos** blood-bank system. It talks to the
Laravel API over HTTP (`QNetworkAccessManager` + `QJsonDocument`) and targets the
staff/PC role.

## Requirements

- Qt 6 (Widgets + Network)
- CMake >= 3.21
- A C++20 compiler

## Build

```sh
cmake -S . -B build
cmake --build build -j
```

## Run

```sh
./build/DonemosDesktop
```

The API base URL defaults to `http://127.0.0.1:8000` and can be changed from the
login window. The staff token is persisted with `QSettings`, so a valid session
is restored on the next launch (validated against `GET /api/auth/me`).

## Layout

```
src/
├── main.cpp                 # entry point, wires the application controller
├── api/ApiClient.*          # HTTP + token storage + error mapping
├── app/AppController.*      # login/main window lifecycle and session bootstrap
└── ui/
    ├── LoginWindow.*        # authentication form
    ├── MainWindow.*         # sidebar navigation + stacked pages
    ├── pages/               # Inicio, Pacientes, Turnos, placeholders
    └── dialogs/             # patient/turno forms and detail views
```

## Scope (current pass)

Implemented: login/session, app shell, Pacientes (list/create/detail), Turnos
(list/create with 409 force flow). `Aptitud`, `Donaciones` and `Reportes` are
placeholder pages inside the real navigation shell.
