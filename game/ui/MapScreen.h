#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <engine/map/WorldMap.h>
#include <engine/entities/Player.h>

class MapScreen : public QWidget {
    Q_OBJECT

public:
    explicit MapScreen(engine::WorldMap& map, engine::Player& player, QWidget* parent = nullptr);

signals:
    void nodeSelected(int nodeId);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void rebuild();

    engine::WorldMap& m_map;
    engine::Player& m_player;
    QVBoxLayout* m_buttonsLayout = nullptr;
};
