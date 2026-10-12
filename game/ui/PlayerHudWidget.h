#pragma once

#include <QWidget>
#include <engine/entities/Player.h>

class PlayerHudWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlayerHudWidget(const engine::Player* player, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const engine::Player* m_player;
};
