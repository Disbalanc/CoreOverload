#include "PlayerHudWidget.h"
#include <QPainter>

PlayerHudWidget::PlayerHudWidget(const engine::Player* player, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
{
    setMinimumHeight(75);
    setMaximumHeight(85);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void PlayerHudWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Фон HUD — тёмная панель
    p.fillRect(rect(), QColor("#0F1923"));
    p.setPen(QPen(QColor("#16213E"), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    int w = width();
    int marginX = 15;

    // ── 1. Энерго-сфера ЦПУ (слева) ──────────────────────
    QRect energyBox(marginX, 10, 55, 55);
    p.fillRect(energyBox, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#00FF41"), 2));
    p.drawRect(energyBox);

    p.setFont(QFont("Courier New", 16, QFont::Bold));
    p.setPen(QColor("#00FF41"));
    p.drawText(energyBox.adjusted(0, -6, 0, 0), Qt::AlignCenter,
               QString("%1/%2").arg(m_player->getEnergy()).arg(m_player->getMaxEnergy()));

    p.setFont(QFont("Courier New", 7, QFont::Bold));
    p.drawText(energyBox.adjusted(0, 18, 0, 0), Qt::AlignCenter, "ЦПУ");

    // ── 2. Полоса HP (центр) ──────────────────────────────
    int barStartX = marginX + 70;
    int barWidth = w / 2 - barStartX;
    if (barWidth < 100) barWidth = 100;

    // HP бар
    QRect hpBg(barStartX, 12, barWidth, 22);
    p.fillRect(hpBg, QColor("#1A1A2E"));
    p.setPen(QPen(QColor("#333333"), 1));
    p.drawRect(hpBg);

    float hpPct = static_cast<float>(m_player->getHp()) / m_player->getMaxHp();
    QColor hpColor = hpPct > 0.5f ? QColor("#00FF41") : (hpPct > 0.25f ? QColor("#FF6B35") : QColor("#E94560"));
    QRect hpFill(hpBg.x() + 1, hpBg.y() + 1,
                 static_cast<int>((hpBg.width() - 2) * hpPct), hpBg.height() - 2);
    p.fillRect(hpFill, hpColor);

    p.setFont(QFont("Courier New", 9, QFont::Bold));
    p.setPen(QColor("#FFFFFF"));
    p.drawText(hpBg, Qt::AlignCenter,
               QString("ЦЕЛОСТНОСТЬ: %1/%2").arg(m_player->getHp()).arg(m_player->getMaxHp()));

    // Файрвол бар
    QRect blockBg(barStartX, 40, barWidth, 18);
    p.fillRect(blockBg, QColor("#16213E"));
    p.setPen(QPen(QColor("#00D9FF"), 1));
    p.drawRect(blockBg);

    p.setFont(QFont("Courier New", 8, QFont::Bold));
    p.setPen(QColor("#00D9FF"));
    p.drawText(blockBg, Qt::AlignCenter,
               QString("ФАЙРВОЛ: %1").arg(m_player->getBlock()));

    // ── 3. Инфо колоды (справа) ───────────────────────────
    int infoX = w / 2 + 20;
    int infoW = w - infoX - marginX;
    if (infoW < 80) infoW = 80;

    // Колода взятия
    QRect drawBox(infoX, 12, infoW / 2 - 5, 26);
    p.fillRect(drawBox, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#00FF41"), 1));
    p.drawRect(drawBox);

    p.setFont(QFont("Courier New", 8, QFont::Bold));
    p.setPen(QColor("#00FF41"));
    p.drawText(drawBox, Qt::AlignCenter,
               QString("КОЛОДА: %1").arg(m_player->getDeck().getDrawPile().size()));

    // Сброс
    QRect discBox(infoX + infoW / 2 + 5, 12, infoW / 2 - 5, 26);
    p.fillRect(discBox, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#E94560"), 1));
    p.drawRect(discBox);

    p.setPen(QColor("#E94560"));
    p.drawText(discBox, Qt::AlignCenter,
               QString("СБРОС: %1").arg(m_player->getDeck().getDiscardPile().size()));

    // Статусы игрока
    int leakStacks = m_player->getStatusStacks("leak");
    if (leakStacks > 0) {
        QRect statusBox(infoX, 44, infoW, 18);
        p.fillRect(statusBox, QColor("#533483"));
        p.setPen(QColor("#FFFFFF"));
        p.setFont(QFont("Courier New", 7, QFont::Bold));
        p.drawText(statusBox, Qt::AlignCenter,
                   QString("УТЕЧКА x%1").arg(leakStacks));
    }
}
