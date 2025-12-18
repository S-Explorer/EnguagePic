cmake_minimum_required(VERSION 3.20)

project(enguageQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package( QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package( Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

message(STATUS "Qt${QT_VERSION_MAJOR} has been found")

file(GLOB header *.h)
file(GLOB source *.cpp)

qt_standard_project_setup()

qt_add_executable(enguage ${header} ${source})

target_link_libraries(enguage PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
