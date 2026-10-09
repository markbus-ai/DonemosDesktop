cmake_minimum_required(VERSION 3.21)

project(DonemosDesktop
    VERSION 0.1.0
    DESCRIPTION "Desktop staff client for the Donemos blood-bank system"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(DonemosDesktop
    src/main.cpp

    src/api/ApiClient.cpp
    src/api/ApiClient.h

    src/app/AppController.cpp
    src/app/AppController.h

    src/ui/Theme.cpp
    src/ui/Theme.h

    src/ui/LoginWindow.cpp
    src/ui/LoginWindow.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h

    src/ui/pages/InicioPage.cpp
    src/ui/pages/InicioPage.h
    src/ui/pages/PacientesPage.cpp
    src/ui/pages/PacientesPage.h
    src/ui/pages/TurnosPage.cpp
    src/ui/pages/TurnosPage.h
    src/ui/pages/PlaceholderPage.cpp
    src/ui/pages/PlaceholderPage.h

    src/ui/dialogs/NuevoPacienteDialog.cpp
    src/ui/dialogs/NuevoPacienteDialog.h
    src/ui/dialogs/PacienteDetalleDialog.cpp
    src/ui/dialogs/PacienteDetalleDialog.h
    src/ui/dialogs/NuevoTurnoDialog.cpp
    src/ui/dialogs/NuevoTurnoDialog.h
)

target_include_directories(DonemosDesktop PRIVATE src)

qt_add_resources(DonemosDesktop "brand"
    PREFIX "/"
    FILES resources/style.qss
)

target_link_libraries(DonemosDesktop PRIVATE
    Qt6::Widgets
    Qt6::Network
)

set_target_properties(DonemosDesktop PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE OFF
)
