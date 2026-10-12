# ============================================================
# Core Engine — чистый C++20, без внешних зависимостей
# Собирается как статическая библиотека
# ============================================================

# Собираем все .cpp файлы из src/
# (пока их мало, перечисляем явно — так надёжнее для CMake)
set(ENGINE_SOURCES
    src/core/Random.cpp
    src/core/EventBus.cpp
    src/cards/Card.cpp
    src/cards/Deck.cpp
    src/cards/CardDatabase.cpp
    src/entities/CombatEntity.cpp
    src/entities/Player.cpp
    src/entities/Enemy.cpp
    src/entities/EnemyDatabase.cpp
    src/entities/Artifact.cpp
    src/entities/ArtifactDatabase.cpp
    src/combat/CombatManager.cpp
    src/map/WorldMap.cpp
    src/map/MapGenerator.cpp
)

set(ENGINE_HEADERS
    include/engine/core/Types.h
    include/engine/core/Random.h
    include/engine/core/EventBus.h
    include/engine/cards/Card.h
    include/engine/cards/Deck.h
    include/engine/cards/CardDatabase.h
    include/engine/entities/CombatEntity.h
    include/engine/entities/Player.h
    include/engine/entities/Enemy.h
    include/engine/entities/EnemyDatabase.h
    include/engine/entities/Artifact.h
    include/engine/entities/ArtifactDatabase.h
    include/engine/combat/CombatManager.h
    include/engine/map/MapNode.h
    include/engine/map/WorldMap.h
    include/engine/map/MapGenerator.h
)

# Создаём статическую библиотеку
add_library(core_engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Публичные include-пути:
# Любой, кто подключит core_engine, автоматически получит
# доступ к #include <engine/core/Random.h> и т.д.
target_include_directories(core_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party>
        $<INSTALL_INTERFACE:include>
)

# C++20 для этой библиотеки
target_compile_features(core_engine PUBLIC cxx_std_20)

# Пока нет внешних зависимостей!
# Позже добавим: target_link_libraries(core_engine PUBLIC nlohmann_json)
