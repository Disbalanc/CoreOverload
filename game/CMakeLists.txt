# ============================================================
# Core Overload — Qt Desktop Frontend
# ============================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Включаем заголовочные файлы из ui/
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ui)

add_executable(core_overload
    main.cpp
    ui/CardWidget.h
    ui/EnemyWidget.h
    ui/PlayerHudWidget.h
    ui/CombatScreen.h
    ui/RewardScreen.h
    ui/RestScreen.h
    ui/AssetManager.h
    ui/HandWidget.h
    ui/GameController.h
    ui/MapScreen.h
    ui/SoundManager.h
    ui/TitleScreen.h
    ui/CombatLogWidget.h
    ui/DeckViewDialog.h
    ui/EventScreen.h
    ui/SettingsDialog.h
    ui/GameLog.h
    ui/ShopScreen.h
    ui/ToastNotification.h
    ui/ArtifactBarWidget.h

    ui/CardWidget.cpp
    ui/EnemyWidget.cpp
    ui/PlayerHudWidget.cpp
    ui/CombatScreen.cpp
    ui/RewardScreen.cpp
    ui/RestScreen.cpp
    ui/AssetManager.cpp
    ui/HandWidget.cpp
    ui/GameController.cpp
    ui/MapScreen.cpp
    ui/SoundManager.cpp
    ui/TitleScreen.cpp
    ui/CombatLogWidget.cpp
    ui/DeckViewDialog.cpp
    ui/EventScreen.cpp
    ui/SettingsDialog.cpp
    ui/GameLog.cpp
    ui/ShopScreen.cpp
    ui/ToastNotification.cpp
    ui/ArtifactBarWidget.cpp
)

target_link_libraries(core_overload
    PRIVATE
        ${QT_LIBRARIES}
        core_engine
)

# КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ: Копируем ассеты в директорию сборки
# Команда вызывается внутри той же директории, где создается цель 'core_overload'
add_custom_command(
    TARGET core_overload POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:core_overload>/assets
    COMMENT "Копирование папки assets в директорию сборки исполняемого файла"
)
