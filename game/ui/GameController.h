#pragma once

#include <QObject>
#include <QStackedWidget>
#include <memory>

#include <engine/core/EventBus.h>
#include <engine/core/Random.h>
#include <engine/entities/Player.h>
#include <engine/combat/CombatManager.h>
#include <engine/map/WorldMap.h>

class CombatScreen;
class RewardScreen;
class RestScreen;
class MapScreen;
class TitleScreen;
class EventScreen;
class ShopScreen;

class GameController : public QObject {
    Q_OBJECT

public:
    explicit GameController(QStackedWidget* screenStack, QObject* parent = nullptr);
    ~GameController() override;

    void showTitleScreen();

private slots:
    void startNewRun();
    void onNodeSelected(int nodeId);
    void onCombatFinished(bool playerWon);
    void onRewardClaimed();
    void onRestFinished();
    void onEventFinished();
    void onShopExited();

private:
    void showMapScreen();
    void showCombatScreen(const std::string& encounterId);
    void showRewardScreen();
    void showRestScreen();
    void showEventScreen();
    void showShopScreen();
    void showGameOverScreen(bool victory);
    void clearScreen(QWidget*& screen);

    std::vector<engine::EnemyConfig> createEncounter(const std::string& id);
    void createStarterDeck(engine::Deck& deck);

    QStackedWidget* m_stack;

    std::unique_ptr<engine::Random> m_rng;
    std::unique_ptr<engine::EventBus> m_eventBus;
    std::unique_ptr<engine::Player> m_player;
    std::unique_ptr<engine::CombatManager> m_combat;
    std::unique_ptr<engine::WorldMap> m_map;

    QWidget* m_titleScreen = nullptr;
    QWidget* m_mapScreen = nullptr;
    QWidget* m_combatScreen = nullptr;
    QWidget* m_rewardScreen = nullptr;
    QWidget* m_restScreen = nullptr;
    QWidget* m_eventScreen = nullptr;
    QWidget* m_shopScreen = nullptr;
    QWidget* m_gameOverScreen = nullptr;
};
