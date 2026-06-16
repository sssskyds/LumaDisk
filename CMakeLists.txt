cmake_minimum_required(VERSION 3.20)
project(LumaDisk VERSION 1.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)
add_executable(LumaDisk
    src/main.cpp
    src/AppTypes.h
    src/ScanWorker.h src/ScanWorker.cpp
    src/OrganizerService.h src/OrganizerService.cpp
    src/MainWindow.h src/MainWindow.cpp
    resources.qrc
)
target_include_directories(LumaDisk PRIVATE src)
target_link_libraries(LumaDisk PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets)
if(WIN32)
    target_link_libraries(LumaDisk PRIVATE Shell32 Ole32)
endif()
if(WIN32 AND NOT MINGW)
    set_target_properties(LumaDisk PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
if(MINGW)
    target_link_options(LumaDisk PRIVATE "-mwindows")
    set_target_properties(LumaDisk PROPERTIES QT_NO_ENTRYPOINT TRUE)
endif()
install(TARGETS LumaDisk DESTINATION bin)