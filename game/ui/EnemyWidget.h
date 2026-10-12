#pragma once

#include <QWidget>
#include <engine/entities/Enemy.h>

class EnemyWidget : public QWidget {
    Q_OBJECT

public:
    explicit EnemyWidget(const engine::Enemy* enemy, int enemyIndex, QWidget* parent = nullptr);

    int getEnemyIndex() const { return m_enemyIndex; }
    void setTargetable(bool targetable);

signals:
    void enemyClicked(int enemyIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawPixelDrone(QPainter& p, const QRect& rect);

    const engine::Enemy* m_enemy;
    int m_enemyIndex;
    bool m_isHovered = false;
    bool m_isTargetable = false;
};
