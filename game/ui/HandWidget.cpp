#include "HandWidget.h"
#include <QPainter>
#include <QResizeEvent>

HandWidget::HandWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(CardWidget::CARD_HEIGHT + 40);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void HandWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));
}

void HandWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // КЛЮЧЕВОЕ: при любом изменении размера пересчитываем позиции карт
    layoutCards();
}

void HandWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // При первом показе тоже раскладываем
    layoutCards();
}

void HandWidget::rebuild(const engine::Player& player, int selectedIndex) {
    m_selectedIndex = selectedIndex;

    // Удаляем старые виджеты карт
    for (auto* cw : m_cardWidgets) {
        cw->deleteLater();
    }
    m_cardWidgets.clear();

    // Создаём новые виджеты
    const auto& hand = player.getDeck().getHand();
    for (size_t i = 0; i < hand.size(); ++i) {
        auto* cw = new CardWidget(hand[i], static_cast<int>(i), this);
        if (static_cast<int>(i) == selectedIndex) {
            cw->setSelected(true);
        }
        if (hand[i]->getCurrentCost() > player.getEnergy()) {
            cw->setPlayable(false);
        }
        connect(cw, &CardWidget::cardClicked, this, &HandWidget::cardClicked);

        // Начальная позиция: за пределами экрана, чтобы не мелькали в углу
        cw->move(-500, -500);
        cw->show();
        m_cardWidgets.push_back(cw);
    }

    // Раскладываем, только если виджет уже имеет размер
    if (width() > 0 && height() > 0) {
        layoutCards();
    }
}

void HandWidget::layoutCards() {
    int count = static_cast<int>(m_cardWidgets.size());
    if (count == 0) return;
    if (width() <= 0) return; // Защита от нулевого размера

    int cardW = CardWidget::CARD_WIDTH;
    int spacing = 15;
    int totalW = count * cardW + (count - 1) * spacing;

    // Если карт слишком много — уменьшаем spacing
    int availableWidth = width() - 40;
    if (totalW > availableWidth && count > 1) {
        spacing = std::max(-50, (availableWidth - count * cardW) / (count - 1));
        totalW = count * cardW + (count - 1) * spacing;
    }

    int startX = (width() - totalW) / 2;
    int baseY = 30;

    for (int i = 0; i < count; ++i) {
        QPoint pos(startX + i * (cardW + spacing), baseY);
        m_cardWidgets[i]->setBasePosition(pos);
    }
}
