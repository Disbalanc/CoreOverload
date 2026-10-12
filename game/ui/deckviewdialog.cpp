#include "DeckViewDialog.h"
#include "CardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

DeckViewDialog::DeckViewDialog(const engine::Player& player, PileType pile, QWidget* parent)
    : QDialog(parent)
{
    QString title;
    std::vector<const engine::Card*> cards;

    switch (pile) {
        case DrawPile:
            title = "КОЛОДА ВЗЯТИЯ (ПЕРЕМЕШАНО)";
            for (auto* c : player.getDeck().getDrawPile()) cards.push_back(c);
            break;
        case DiscardPile:
            title = "СБРОС";
            for (auto* c : player.getDeck().getDiscardPile()) cards.push_back(c);
            break;
        case FullDeck:
            title = "ВСЯ КОЛОДА";
            for (const auto& c : player.getDeck().getMasterDeck()) cards.push_back(c.get());
            break;
    }

    setWindowTitle(title);
    setStyleSheet("QDialog { background-color: #0A0A0F; }");
    resize(900, 650);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(15, 15, 15, 15);

    auto* titleLabel = new QLabel(QString("%1 — %2 карт").arg(title).arg(cards.size()));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Courier New", 12, QFont::Bold));
    titleLabel->setStyleSheet("color: #00FF41; background-color: #16213E; padding: 8px; border: 1px solid #00FF41;");
    root->addWidget(titleLabel);

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { background: #0A0A0F; border: 1px solid #16213E; }"
        "QScrollBar:vertical { background: #1A1A2E; width: 12px; }"
        "QScrollBar::handle:vertical { background: #00FF41; min-height: 20px; }"
    );

    auto* content = new QWidget();
    content->setStyleSheet("background: #0A0A0F;");

    int cardW = CardWidget::CARD_WIDTH;
    int cardH = CardWidget::CARD_HEIGHT;
    int cols = 5;
    int spacing = 15;

    // Задаём размер контента
    int totalRows = (static_cast<int>(cards.size()) + cols - 1) / cols;
    content->setMinimumSize(cols * (cardW + spacing) + 40,
                            std::max(400, totalRows * (cardH + spacing) + 40));

    // Ручное позиционирование карт (чтобы они не прыгали в угол)
    for (size_t i = 0; i < cards.size(); ++i) {
        auto* cw = new CardWidget(cards[i], -1, content);
        cw->setPlayable(true);
        int row = static_cast<int>(i) / cols;
        int col = static_cast<int>(i) % cols;
        int x = 20 + col * (cardW + spacing);
        int y = 20 + row * (cardH + spacing);
        cw->setBasePosition(QPoint(x, y));
        cw->move(x, y);
        cw->show();
    }

    scroll->setWidget(content);
    root->addWidget(scroll);

    auto* closeBtn = new QPushButton("ЗАКРЫТЬ");
    closeBtn->setFixedSize(200, 40);
    closeBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 2px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    root->addWidget(closeBtn, 0, Qt::AlignCenter);
}
