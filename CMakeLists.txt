# ============================================================
# Core Overload: Кремниевый Бунт
# Корневой CMake-файл проекта (Поддержка Qt5 и Qt6)
# ============================================================
cmake_minimum_required(VERSION 3.16)

project(CoreOverload
    VERSION 0.1.0
    DESCRIPTION "Cyberpunk deckbuilder roguelike"
    LANGUAGES CXX
)

# Используем C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(BUILD_GAME "Build Qt desktop game" ON)
option(BUILD_TESTS "Build unit tests" OFF)

# Сначала собираем движок
add_subdirectory(engine)

# Настройка автогенерации для Qt (выносим наверх)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск Qt5 или Qt6 с поддержкой мультимедиа
if(BUILD_GAME)
    find_package(Qt6 COMPONENTS Widgets Multimedia QUIET)

    if(Qt6_FOUND)
        message(STATUS "System is using: Qt6 (v${Qt6_VERSION})")
        set(QT_LIBRARIES Qt6::Widgets Qt6::Multimedia)
    else()
        find_package(Qt5 COMPONENTS Widgets Multimedia QUIET)
        if(Qt5_FOUND)
            message(STATUS "System is using: Qt5 (v${Qt5_VERSION})")
            set(QT_LIBRARIES Qt5::Widgets Qt5::Multimedia)
        else()
            message(WARNING "Neither Qt6 nor Qt5 was found!")
        endif()
    endif()

    if(Qt6_FOUND OR Qt5_FOUND)
        add_subdirectory(game)
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
