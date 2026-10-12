#pragma once

#include <QWidget>
#include <engine/entities/Player.h>

class ArtifactBarWidget : public QWidget {
    Q_OBJECT

public:
    explicit ArtifactBarWidget(const engine::Player* player, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    const engine::Player* m_player;
    int m_hoveredIndex = -1;
};
