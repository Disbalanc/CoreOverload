#include "CardWidget.h"
#include "AssetManager.h"
#include "SoundManager.h"
#include <QPainter>
#include <QMouseEvent>

CardWidget::CardWidget(const engine::Card* card, int handIndex, QWidget* parent)
    : QWidget(parent)
    , m_card(card)
    , m_handIndex(handIndex)
{
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);

    // КЛЮЧЕВОЕ: явно рисуем фон в paintEvent, иначе будет белый прямоугольник
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void CardWidget::setSelected(bool selected) {
    m_isSelected = selected;
    update();
}

void CardWidget::setPlayable(bool playable) {
    m_isPlayable = playable;
    update();
}

void CardWidget::setBasePosition(const QPoint& pos) {
    m_basePosition = pos;
    if (!m_isHovered) {
        move(pos);
    }
}

void CardWidget::enterEvent(QEvent*) {
    m_isHovered = true;
    raise(); // Поднимаем над соседями по Z-order
    // Физически двигаем виджет вверх
    move(m_basePosition.x(), m_basePosition.y() - 25);
    emit cardHoverChanged(m_handIndex, true);
    update();
}

void CardWidget::leaveEvent(QEvent*) {
    m_isHovered = false;
    move(m_basePosition); // Возвращаем на базу
    emit cardHoverChanged(m_handIndex, false);
    update();
}

void CardWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        SoundManager::instance().playSound("click");
        emit cardClicked(m_handIndex);
    }
}

void CardWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    QRect cardRect(0, 0, width(), height());

    // Цвета в зависимости от типа и состояния
    QColor borderColor = QColor("#533483");
    QColor bgColor = QColor("#16213E");

    if (m_card->getType() == engine::CardType::Attack) {
        borderColor = QColor("#E94560");
    } else if (m_card->getType() == engine::CardType::Skill) {
        borderColor = QColor("#00D9FF");
    }

    if (!m_isPlayable) {
        bgColor = QColor("#1A1A1A");
        borderColor = QColor("#555555");
    } else if (m_isSelected) {
        borderColor = QColor("#00FF41");
        bgColor = QColor("#0F3460");
    } else if (m_isHovered) {
        bgColor = QColor("#1F2A4E");
    }

    // 1. Фон карты (теперь ВСЕГДА заполняется)
    p.fillRect(cardRect, bgColor);

    // 2. Двойная рамка (пиксельный стиль)
    p.setPen(QPen(borderColor, 2));
    p.drawRect(cardRect.adjusted(0, 0, -1, -1));
    p.setPen(QPen(QColor("#0A0A0F"), 1));
    p.drawRect(cardRect.adjusted(2, 2, -3, -3));

    // 3. Сфера стоимости ЦПУ (круглая, слева вверху)
    QRect costRect(4, 4, 26, 26);
    p.fillRect(costRect, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#00FF41"), 2));
    p.drawRect(costRect);

    p.setFont(QFont("Courier New", 12, QFont::Bold));
    p.setPen(QColor("#00FF41"));
    p.drawText(costRect, Qt::AlignCenter, QString::number(m_card->getCurrentCost()));

    // 4. Название карты
    QRect titleRect(34, 6, cardRect.width() - 38, 22);
    p.setFont(QFont("Courier New", 8, QFont::Bold));
    p.setPen(m_isPlayable ? QColor("#FFFFFF") : QColor("#777777"));
    p.drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft,
               QString::fromStdString(m_card->getName()));

    // 5. Разделительная полоса
    p.setPen(QPen(borderColor, 1));
    p.drawLine(6, 34, cardRect.width() - 7, 34);

    // 6. Арт карты — попытка загрузить спрайт, иначе процедурный
    QRect artRect(10, 40, cardRect.width() - 20, 80);

    QString spriteId = "card_" + QString::fromStdString(m_card->getId());
    // Убираем числовой суффикс для одинаковых карт (ddos_0, ddos_1 → ddos)
    spriteId.remove(QRegExp("_\\d+$"));

    const QPixmap* sprite = AssetManager::instance().getSprite(spriteId);
    if (sprite) {
        p.drawPixmap(artRect, *sprite);
    } else {
        drawProceduralArt(p, artRect);
    }

    // 7. Описание эффектов
    QRect descRect(6, 125, cardRect.width() - 12, 55);
    p.setFont(QFont("Courier New", 7));
    p.setPen(m_isPlayable ? QColor("#C0C0C0") : QColor("#555555"));
    p.drawText(descRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
               QString::fromStdString(m_card->getDescription()));

    // 8. Бейдж типа внизу
    QRect typeRect(4, cardRect.height() - 16, cardRect.width() - 8, 12);
    p.setFont(QFont("Courier New", 6, QFont::Bold));
    p.setPen(QColor("#888888"));
    QString typeStr = (m_card->getType() == engine::CardType::Attack)
                      ? "[EXPLOIT.exe]" : "[UTILITY.dll]";
    p.drawText(typeRect, Qt::AlignCenter, typeStr);
}

void CardWidget::drawProceduralArt(QPainter& p, const QRect& r) {
    // Процедурный fallback: пиксельный паттерн по типу карты
    p.fillRect(r, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#00FF41"), 1));
    p.drawRect(r);

    if (m_card->getType() == engine::CardType::Attack) {
        // Красная "молния" атаки
        p.setPen(QPen(QColor("#E94560"), 3));
        QPoint center = r.center();
        p.drawLine(center.x() - 15, center.y() - 20, center.x() + 5, center.y());
        p.drawLine(center.x() + 5, center.y(), center.x() - 5, center.y());
        p.drawLine(center.x() - 5, center.y(), center.x() + 15, center.y() + 20);
    } else {
        // Синий "щит" защиты
        p.setPen(QPen(QColor("#00D9FF"), 2));
        p.setBrush(QColor("#16213E"));
        QPoint c = r.center();
        QPolygon shield;
        shield << QPoint(c.x(), c.y() - 20)
               << QPoint(c.x() + 18, c.y() - 10)
               << QPoint(c.x() + 15, c.y() + 15)
               << QPoint(c.x(), c.y() + 22)
               << QPoint(c.x() - 15, c.y() + 15)
               << QPoint(c.x() - 18, c.y() - 10);
        p.drawPolygon(shield);
    }
}
