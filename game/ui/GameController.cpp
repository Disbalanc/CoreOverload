#include "GameController.h"
#include "TitleScreen.h"
#include "CombatScreen.h"
#include "RewardScreen.h"
#include "RestScreen.h"
#include "MapScreen.h"
#include "EventScreen.h"
#include "ShopScreen.h"
#include "SettingsDialog.h"
#include "GameLog.h"
#include <engine/map/MapGenerator.h>
#include <engine/cards/CardDatabase.h>
#include <engine/entities/EnemyDatabase.h>
#include <engine/entities/ArtifactDatabase.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QCoreApplication>

GameController::GameController(QStackedWidget* screenStack, QObject* parent)
    : QObject(parent)
    , m_stack(screenStack)
{
    // Загружаем базу карт один раз при старте
    QStringList possiblePaths = {
        "assets/data/cards.json",
        QCoreApplication::applicationDirPath() + "/assets/data/cards.json",
        QCoreApplication::applicationDirPath() + "/../assets/data/cards.json",
    };
    bool loaded = false;
    for (const auto& p : possiblePaths) {
        if (engine::CardDatabase::instance().loadFromFile(p.toStdString())) {
            loaded = true;
            break;
        }
    }

    // Загружаем базу врагов
    QStringList enemyPaths = {
        "assets/data/enemies.json",
        QCoreApplication::applicationDirPath() + "/assets/data/enemies.json",
        QCoreApplication::applicationDirPath() + "/../assets/data/enemies.json",
    };
    for (const auto& p : enemyPaths) {
        if (engine::EnemyDatabase::instance().loadFromFile(p.toStdString())) break;
    }

    // Загружаем базу артефактов
    QStringList artifactPaths = {
        "assets/data/artifacts.json",
        QCoreApplication::applicationDirPath() + "/assets/data/artifacts.json",
        QCoreApplication::applicationDirPath() + "/../assets/data/artifacts.json",
    };
    for (const auto& p : artifactPaths) {
        if (engine::ArtifactDatabase::instance().loadFromFile(p.toStdString())) break;
    }

    if (!loaded) {
        GameLog::instance().addMessage("⚠ Не удалось загрузить cards.json!", "#E94560");
    }
}

GameController::~GameController() = default;

void GameController::clearScreen(QWidget*& screen) {
    if (screen) {
        m_stack->removeWidget(screen);
        screen->deleteLater();
        screen = nullptr;
    }
}

void GameController::showTitleScreen() {
    clearScreen(m_titleScreen);
    auto* title = new TitleScreen();
    connect(title, &TitleScreen::startNewGame, this, &GameController::startNewRun);
    connect(title, &TitleScreen::quitGame, qApp, &QApplication::quit);
    connect(title, &TitleScreen::openSettings, [this]() {
        SettingsDialog dlg(m_stack);
        dlg.exec();
    });
    m_titleScreen = title;
    m_stack->addWidget(m_titleScreen);
    m_stack->setCurrentWidget(m_titleScreen);
}

void GameController::startNewRun() {
    GameLog::instance().clear();
    GameLog::instance().addMessage("═══ НОВЫЙ ЗАБЕГ ═══", "#00FF41");

    m_rng = std::make_unique<engine::Random>();
    m_eventBus = std::make_unique<engine::EventBus>();
    m_player = std::make_unique<engine::Player>(50, 3);
    createStarterDeck(m_player->getDeck());
    // Стартовый артефакт — Паяльник Инженера
    if (auto art = engine::ArtifactDatabase::instance().createArtifact("soldering_iron")) {
        m_player->addArtifact(std::move(art));
        GameLog::instance().addMessage("🎁 Получен стартовый артефакт: Паяльник инженера", "#FFD700");
    }
    m_combat = std::make_unique<engine::CombatManager>(*m_eventBus, *m_rng);

    engine::MapConfig cfg;
    cfg.totalFloors = 7;
    cfg.lanesCount = 3;
    m_map = std::make_unique<engine::WorldMap>(
        engine::MapGenerator::generate(cfg, *m_rng));

    GameLog::instance().addMessage(
        QString("🎲 Seed: %1 | Карта: %2 этажей")
            .arg(m_rng->getSeed())
            .arg(cfg.totalFloors),
        "#808080");

    showMapScreen();
}

void GameController::onNodeSelected(int nodeId) {
    if (!m_map->moveToNode(nodeId)) return;
    const auto* node = m_map->getCurrentNode();
    if (!node) return;

    QString nodeName;
    QString color = "#808080";
    switch (node->type) {
        case engine::NodeType::Battle:      nodeName = "⚔ БОЙ";          color = "#E94560"; break;
        case engine::NodeType::EliteBattle: nodeName = "💀 ЭЛИТА";       color = "#FF6B35"; break;
        case engine::NodeType::BossBattle:  nodeName = "👑 БОСС";        color = "#E94560"; break;
        case engine::NodeType::Rest:        nodeName = "🔧 ОТДЫХ";       color = "#00D9FF"; break;
        case engine::NodeType::Event:       nodeName = "❓ ПОРТ";        color = "#533483"; break;
        case engine::NodeType::Shop:        nodeName = "💾 РЫНОК";       color = "#FFD700"; break;
    }
    GameLog::instance().addMessage(QString("→ Переход в: %1").arg(nodeName), color);

    switch (node->type) {
        case engine::NodeType::Battle:      showCombatScreen("basic"); break;
        case engine::NodeType::EliteBattle: showCombatScreen("elite"); break;
        case engine::NodeType::BossBattle:  showCombatScreen("boss"); break;
        case engine::NodeType::Rest:        showRestScreen(); break;
        case engine::NodeType::Event:       showEventScreen(); break;
        case engine::NodeType::Shop:        showShopScreen(); break;
    }
}

void GameController::onCombatFinished(bool playerWon) {
    if (!playerWon) {
        showGameOverScreen(false);
        return;
    }
    if (m_map->getCurrentNode() &&
        m_map->getCurrentNode()->type == engine::NodeType::BossBattle) {
        showGameOverScreen(true);
    } else {
        showRewardScreen();
    }
}

void GameController::onRewardClaimed() { showMapScreen(); }
void GameController::onRestFinished()  { showMapScreen(); }
void GameController::onEventFinished() { showMapScreen(); }
void GameController::onShopExited()    { showMapScreen(); }

void GameController::showMapScreen() {
    clearScreen(m_mapScreen);
    auto* map = new MapScreen(*m_map, *m_player);
    connect(map, &MapScreen::nodeSelected, this, &GameController::onNodeSelected);
    m_mapScreen = map;
    m_stack->addWidget(m_mapScreen);
    m_stack->setCurrentWidget(m_mapScreen);
}

void GameController::showCombatScreen(const std::string& encounterId) {
    clearScreen(m_combatScreen);
    auto enemies = createEncounter(encounterId);
    m_combat->startCombat(*m_player, enemies);
    auto* screen = new CombatScreen(*m_combat, *m_player, *m_eventBus);
    connect(screen, &CombatScreen::combatFinished,
            this, &GameController::onCombatFinished);
    m_combatScreen = screen;
    m_stack->addWidget(m_combatScreen);
    m_stack->setCurrentWidget(m_combatScreen);
}

void GameController::showRewardScreen() {
    clearScreen(m_rewardScreen);
    auto* screen = new RewardScreen(*m_player, *m_rng);

    // Если убили элитного врага — награждаем ещё и артефактом
    if (m_map->getCurrentNode() &&
        m_map->getCurrentNode()->type == engine::NodeType::EliteBattle) {
        screen->setArtifactReward(true);
    }

    connect(screen, &RewardScreen::rewardClaimed,
            this, &GameController::onRewardClaimed);
    m_rewardScreen = screen;
    m_stack->addWidget(m_rewardScreen);
    m_stack->setCurrentWidget(m_rewardScreen);
}

void GameController::showRestScreen() {
    clearScreen(m_restScreen);
    auto* screen = new RestScreen(*m_player);
    connect(screen, &RestScreen::restFinished,
            this, &GameController::onRestFinished);
    m_restScreen = screen;
    m_stack->addWidget(m_restScreen);
    m_stack->setCurrentWidget(m_restScreen);
}

void GameController::showEventScreen() {
    clearScreen(m_eventScreen);
    auto* screen = new EventScreen(*m_player, *m_rng);
    connect(screen, &EventScreen::eventFinished,
            this, &GameController::onEventFinished);
    m_eventScreen = screen;
    m_stack->addWidget(m_eventScreen);
    m_stack->setCurrentWidget(m_eventScreen);
}

void GameController::showShopScreen() {
    clearScreen(m_shopScreen);
    auto* screen = new ShopScreen(*m_player, *m_rng);
    connect(screen, &ShopScreen::shopExited,
            this, &GameController::onShopExited);
    m_shopScreen = screen;
    m_stack->addWidget(m_shopScreen);
    m_stack->setCurrentWidget(m_shopScreen);
}

void GameController::showGameOverScreen(bool victory) {
    clearScreen(m_gameOverScreen);
    GameLog::instance().addMessage(
        victory ? "═══ ПОБЕДА ═══" : "═══ ПОРАЖЕНИЕ ═══",
        victory ? "#00FF41" : "#E94560");

    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setAlignment(Qt::AlignCenter);

    auto* title = new QLabel(victory
        ? "СИСТЕМА ВЗЛОМАНА!\n\nПОБЕДА"
        : "ЦЕЛОСТНОСТЬ: 0%\n\nПОРАЖЕНИЕ");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Courier New", 22, QFont::Bold));
    title->setStyleSheet(QString("color: %1; background: transparent;")
                         .arg(victory ? "#00FF41" : "#E94560"));
    layout->addWidget(title);

    auto* menuBtn = new QPushButton("В ГЛАВНОЕ МЕНЮ");
    menuBtn->setFixedSize(350, 50);
    menuBtn->setFont(QFont("Courier New", 11, QFont::Bold));
    menuBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 2px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }");
    connect(menuBtn, &QPushButton::clicked, this, &GameController::showTitleScreen);
    layout->addWidget(menuBtn, 0, Qt::AlignCenter);

    m_gameOverScreen = w;
    m_stack->addWidget(m_gameOverScreen);
    m_stack->setCurrentWidget(m_gameOverScreen);
}

std::vector<engine::EnemyConfig> GameController::createEncounter(const std::string& type) {
    auto& db = engine::EnemyDatabase::instance();
    const engine::EncounterTemplate* enc = db.getRandomEncounter(type, *m_rng);
    if (enc) {
        GameLog::instance().addMessage(
            QString("⚔ Встреча: %1").arg(QString::fromStdString(enc->id)),
            "#E94560");
        return db.buildEncounter(enc->id, *m_rng);
    }

    // Fallback если JSON не загрузился
    return {{"drone_fallback", "Дрон", 20, {
        {engine::IntentType::Attack, 5, 1}
    }}};
}

void GameController::createStarterDeck(engine::Deck& deck) {
    // Теперь карты создаём из JSON базы!
    auto& db = engine::CardDatabase::instance();

    for (int i = 0; i < 4; ++i) {
        if (auto c = db.createCard("ddos_strike", i))
            deck.addToMasterDeck(std::move(c));
    }
    for (int i = 0; i < 3; ++i) {
        if (auto c = db.createCard("cooling", i))
            deck.addToMasterDeck(std::move(c));
    }
    if (auto c = db.createCard("overclock", 0))
        deck.addToMasterDeck(std::move(c));
    if (auto c = db.createCard("trojan", 0))
        deck.addToMasterDeck(std::move(c));
    if (auto c = db.createCard("data_parsing", 0))
        deck.addToMasterDeck(std::move(c));
}
