#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <engine/entities/Player.h>

class RestScreen : public QWidget {
    Q_OBJECT

public:
    explicit RestScreen(engine::Player& player, QWidget* parent = nullptr);

signals:
    void restFinished();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onRepair();
    void onUpgrade();
    void onSkip();

private:
    void showUpgradeSelector();

    engine::Player& m_player;
    QLabel* m_title = nullptr;
    QPushButton* m_repairBtn = nullptr;
    QPushButton* m_upgradeBtn = nullptr;
    QPushButton* m_skipBtn = nullptr;
    bool m_actionTaken = false;
};
