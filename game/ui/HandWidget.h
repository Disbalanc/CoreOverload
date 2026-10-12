#pragma once

#include <QWidget>
#include <vector>
#include "CardWidget.h"
#include <engine/entities/Player.h>

class HandWidget : public QWidget {
    Q_OBJECT

public:
    explicit HandWidget(QWidget* parent = nullptr);

    void rebuild(const engine::Player& player, int selectedIndex);

signals:
    void cardClicked(int handIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void layoutCards();

    std::vector<CardWidget*> m_cardWidgets;
    int m_selectedIndex = -1;
};
