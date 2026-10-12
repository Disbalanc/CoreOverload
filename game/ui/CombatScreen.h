#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include <engine/combat/CombatManager.h>
#include <engine/core/EventBus.h>
#include "EnemyWidget.h"
#include "PlayerHudWidget.h"
#include "HandWidget.h"
#include "ToastNotification.h"
#include "ArtifactBarWidget.h"

class CombatScreen : public QWidget {
    Q_OBJECT

public:
    explicit CombatScreen(engine::CombatManager& combat,
                          engine::Player& player,
                          engine::EventBus& eventBus,
                          QWidget* parent = nullptr);
    ~CombatScreen() override;

    void refreshUI();

signals:
    void combatFinished(bool playerWon);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onCardClicked(int handIndex);
    void onEnemyClicked(int enemyIndex);
    void onEndTurnClicked();

private:
    void cancelCardSelection();
    void updateStatusBanner(const QString& text, const QString& colorHex = "#00FF41");

    engine::CombatManager& m_combat;
    engine::Player& m_player;
    engine::EventBus& m_eventBus;

    int m_selectedCardIndex = -1;

    // Верхний блок
    QLabel* m_turnLabel = nullptr;
    QLabel* m_statusBanner = nullptr;

    // Враги
    QWidget* m_enemiesContainer = nullptr;
    QHBoxLayout* m_enemiesLayout = nullptr;

    // Нижний блок: HUD
    PlayerHudWidget* m_playerHud = nullptr;

    // Рука карт
    HandWidget* m_handWidget = nullptr;

    // Кнопка завершения хода
    QPushButton* m_endTurnBtn = nullptr;

    std::vector<engine::SubscriptionId> m_subscriptions;
    bool m_combatEndEmitted = false;

    ToastNotification* m_toastWidget = nullptr;
    ArtifactBarWidget* m_artifactBar = nullptr;
};
