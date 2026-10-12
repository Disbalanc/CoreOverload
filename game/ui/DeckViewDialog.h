#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <engine/entities/Player.h>

class DeckViewDialog : public QDialog {
    Q_OBJECT

public:
    enum PileType {
        DrawPile,
        DiscardPile,
        FullDeck
    };

    explicit DeckViewDialog(const engine::Player& player, PileType pile, QWidget* parent = nullptr);
};
