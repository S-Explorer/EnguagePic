cmake_minimum_required(VERSION 3.20)

project(enguageQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(Qt6_Dir D:Qt/6.9.2/msvc2022_64/lib/cmake/Qt6)
find_package( QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package( Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

message(STATUS "Qt${QT_VERSION_MAJOR} has been found")

file(GLOB header *.h)
file(GLOB source *.cpp)

# 资源文件（Qt 资源系统）
set(qrc_file resources.qrc)

qt_standard_project_setup()

# 平台特定的资源
if(WIN32)
    # Windows: 添加 .rc 文件设置可执行文件图标
    set(rc_file app_icon.rc)
    qt_add_executable(enguage ${header} ${source} ${qrc_file} ${rc_file})
else()
    # Linux/macOS: 仅使用 Qt 资源文件
    qt_add_executable(enguage ${header} ${source} ${qrc_file})
endif()

target_link_libraries(enguage PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
