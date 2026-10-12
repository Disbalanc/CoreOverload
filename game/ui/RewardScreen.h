#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include <memory>
#include <engine/cards/Card.h>
#include <engine/entities/Player.h>
#include <engine/core/Random.h>
#include <engine/entities/ArtifactDatabase.h>
#include "GameLog.h"

class CardWidget;

class RewardScreen : public QWidget {
    Q_OBJECT

public:
    explicit RewardScreen(engine::Player& player, engine::Random& rng, QWidget* parent = nullptr);
    void setArtifactReward(bool enable) { m_offerArtifact = enable; }

signals:
    void rewardClaimed();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onCardSelected(int index);
    void onConfirmClicked();
    void onSkipClicked();

private:
    void generateRewards();
    void rebuildCards();

    engine::Player& m_player;
    engine::Random& m_rng;

    std::vector<std::unique_ptr<engine::Card>> m_rewardCards;
    int m_selectedRewardIndex = -1;

    QLabel* m_titleLabel = nullptr;
    QLabel* m_infoLabel = nullptr;
    QWidget* m_cardsContainer = nullptr;
    QHBoxLayout* m_cardsLayout = nullptr;
    QPushButton* m_confirmBtn = nullptr;
    QPushButton* m_skipBtn = nullptr;
    bool m_offerArtifact = false;
    std::unique_ptr<engine::Artifact> m_artifactReward;
};
