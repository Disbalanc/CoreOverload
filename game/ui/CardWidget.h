#pragma once

#include <QWidget>
#include <engine/cards/Card.h>

class CardWidget : public QWidget {
    Q_OBJECT

public:
    explicit CardWidget(const engine::Card* card, int handIndex, QWidget* parent = nullptr);

    static constexpr int CARD_WIDTH = 140;
    static constexpr int CARD_HEIGHT = 200;

    int getHandIndex() const { return m_handIndex; }
    const engine::Card* getCard() const { return m_card; }

    void setSelected(bool selected);
    bool isSelected() const { return m_isSelected; }

    void setPlayable(bool playable);
    bool isPlayable() const { return m_isPlayable; }

    // Базовая позиция (куда карта возвращается после отведения мыши)
    void setBasePosition(const QPoint& pos);

signals:
    void cardClicked(int handIndex);
    void cardHoverChanged(int handIndex, bool hovered);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawProceduralArt(QPainter& p, const QRect& iconRect);

    const engine::Card* m_card;
    int m_handIndex;
    bool m_isHovered = false;
    bool m_isSelected = false;
    bool m_isPlayable = true;

    QPoint m_basePosition;
};
