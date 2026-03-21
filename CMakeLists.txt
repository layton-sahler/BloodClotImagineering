cmake_minimum_required(VERSION 3.16)
project(SensorApp VERSION 0.1 LANGUAGES CXX)

# 1. Ensure we point to the MSVC version of Qt
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.6/msvc2022_64")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Updated safety check for MSVC
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Detected 32-bit. You MUST select the 'amd64' (64-bit) Kit in VS Code!")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Bluetooth Widgets Charts QuickControls2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(SensorApp main.cpp)

qt_add_qml_module(SensorApp
    URI SensorApp
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES main.qml
    SOURCES 
        btmanager.h
        notifmanager.h
)

target_link_libraries(SensorApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Bluetooth
    Qt6::Widgets
    Qt6::Charts
    Qt6::QuickControls2
)