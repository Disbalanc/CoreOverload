#include "EnemyWidget.h"
#include "SoundManager.h"
#include <QPainter>
#include <QMouseEvent>

EnemyWidget::EnemyWidget(const engine::Enemy* enemy, int enemyIndex, QWidget* parent)
    : QWidget(parent)
    , m_enemy(enemy)
    , m_enemyIndex(enemyIndex)
{
    setFixedSize(160, 220);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);

    // ДОБАВИТЬ:
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void EnemyWidget::setTargetable(bool targetable) {
    m_isTargetable = targetable;
    update();
}

void EnemyWidget::enterEvent(QEvent*) {
    m_isHovered = true;
    update();
}

void EnemyWidget::leaveEvent(QEvent*) {
    m_isHovered = false;
    update();
}

void EnemyWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_enemy->isAlive()) {
        SoundManager::instance().playSound("hit");
        emit enemyClicked(m_enemyIndex);
    }
}

void EnemyWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);

        // ДОБАВИТЬ: явно рисуем фон
        p.fillRect(rect(), QColor("#0A0A0F"));

        if (!m_enemy->isAlive()) {
        // Отрисовка уничтоженного дрона
        p.setFont(QFont("Courier New", 12, QFont::Bold));
        p.setPen(QColor("#808080"));
        p.drawText(rect(), Qt::AlignCenter, "☠ ДЕАКТИВИРОВАН");
        return;
    }

    // Подсветка при выборе цели
    if (m_isTargetable) {
        p.setPen(QPen(QColor("#00FF41"), m_isHovered ? 2 : 1, Qt::DashLine));
        p.drawRect(rect().adjusted(2, 2, -2, -2));
    }

    // 1. Индикатор НАМЕРЕНИЯ (Intent) сверху
    const auto& intent = m_enemy->getIntent();
    QRect intentRect(width() / 2 - 45, 5, 90, 26);
    p.fillRect(intentRect, QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#E94560"), 1));
    p.drawRect(intentRect);

    p.setFont(QFont("Courier New", 9, QFont::Bold));
    QString intentText = "???";
    if (intent.type == engine::IntentType::Attack) {
        p.setPen(QColor("#E94560"));
        intentText = QString("⚔ %1").arg(intent.value);
    } else if (intent.type == engine::IntentType::Defend) {
        p.setPen(QColor("#00D9FF"));
        intentText = QString("🛡 %1").arg(intent.value);
    } else if (intent.type == engine::IntentType::Debuff) {
        p.setPen(QColor("#533483"));
        intentText = QString("☣ УТЕЧКА %1").arg(intent.value);
    }
    p.drawText(intentRect, Qt::AlignCenter, intentText);

    // 2. Спрайт пиксельного робота
    QRect spriteRect(width() / 2 - 35, 40, 70, 70);
    drawPixelDrone(p, spriteRect);

    // 3. Имя врага
    QRect nameRect(0, 115, width(), 20);
    p.setFont(QFont("Courier New", 9, QFont::Bold));
    p.setPen(QColor("#FFFFFF"));
    p.drawText(nameRect, Qt::AlignCenter, QString::fromStdString(m_enemy->getName()));

    // 4. Полоска HP (Здоровья / Целостности)
    QRect hpBarBg(20, 140, width() - 40, 14);
    p.fillRect(hpBarBg, QColor("#1A1A2E"));
    p.setPen(QPen(QColor("#0A0A0F"), 1));
    p.drawRect(hpBarBg);

    float hpPercent = static_cast<float>(m_enemy->getHp()) / m_enemy->getMaxHp();
    QRect hpBarFill(21, 141, static_cast<int>((width() - 42) * hpPercent), 12);
    p.fillRect(hpBarFill, QColor("#E94560"));

    p.setFont(QFont("Courier New", 8, QFont::Bold));
    p.setPen(QColor("#FFFFFF"));
    p.drawText(hpBarBg, Qt::AlignCenter,
               QString("%1 / %2").arg(m_enemy->getHp()).arg(m_enemy->getMaxHp()));

    // 5. Полоска Файрвола (Брони), если она есть
    if (m_enemy->getBlock() > 0) {
        QRect blockRect(20, 160, width() - 40, 16);
        p.fillRect(blockRect, QColor("#0F3460"));
        p.setPen(QPen(QColor("#00D9FF"), 1));
        p.drawRect(blockRect);

        p.setPen(QColor("#00D9FF"));
        p.drawText(blockRect, Qt::AlignCenter, QString("🛡 ФАЙРВОЛ: %1").arg(m_enemy->getBlock()));
    }

    // 6. Статусы (Утечка и т.д.)
    int leak = m_enemy->getStatusStacks("leak");
    if (leak > 0) {
        QRect statusRect(20, 185, width() - 40, 16);
        p.fillRect(statusRect, QColor("#533483"));
        p.setPen(QColor("#FFFFFF"));
        p.setFont(QFont("Courier New", 7, QFont::Bold));
        p.drawText(statusRect, Qt::AlignCenter, QString("☣ УТЕЧКА x%1").arg(leak));
    }
}

void EnemyWidget::drawPixelDrone(QPainter& p, const QRect& r) {
    // Процедурная отрисовка кибер-дрона
    p.fillRect(r, QColor("#16213E"));
    p.setPen(QPen(QColor("#E94560"), 2));
    p.drawRect(r);

    // Сенсорный визор дрона
    QRect eye(r.left() + 15, r.top() + 25, r.width() - 30, 12);
    p.fillRect(eye, QColor("#E94560"));

    // Неоновый зрачок сканера
    QRect pupil(eye.center().x() - 4, eye.top() + 2, 8, 8);
    p.fillRect(pupil, QColor("#00FF41"));

    // Процессорные линии по бокам
    p.setPen(QPen(QColor("#00D9FF"), 1));
    p.drawLine(r.left() + 5, r.bottom() - 10, r.right() - 5, r.bottom() - 10);
}
