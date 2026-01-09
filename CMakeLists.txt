cmake_minimum_required(VERSION 3.16)

set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/mingw_64")

project(SensorApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find the Bluetooth and Quick modules
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Bluetooth)

qt_standard_project_setup()
qt_policy(SET QTP0001 NEW)
# 2. Add your source files
qt_add_executable(SensorApp
    main.cpp
)

# 3. Link the libraries
target_link_libraries(SensorApp PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Quick 
    Qt6::Bluetooth
)

# 4. Include QML files
qt_add_qml_module(SensorApp
    URI SensorApp
    VERSION 1.0
    QML_FILES main.qml
)