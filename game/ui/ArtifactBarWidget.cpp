#include "ArtifactBarWidget.h"
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>

ArtifactBarWidget::ArtifactBarWidget(const engine::Player* player, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
{
    setMinimumHeight(40);
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, false);
}

void ArtifactBarWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), QColor("#0F1923"));
    p.setPen(QPen(QColor("#16213E"), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    const auto& artifacts = m_player->getArtifacts();
    int iconSize = 32;
    int spacing = 6;
    int startX = 8;
    int y = (height() - iconSize) / 2;

    // Заголовок
    p.setFont(QFont("Courier New", 7, QFont::Bold));
    p.setPen(QColor("#808080"));
    p.drawText(QRect(0, 2, width(), 10), Qt::AlignLeft, "  АРТЕФАКТЫ:");

    if (artifacts.empty()) {
        p.setPen(QColor("#555555"));
        p.setFont(QFont("Courier New", 9));
        p.drawText(rect().adjusted(startX, 10, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, "  (пусто)");
        return;
    }

    for (size_t i = 0; i < artifacts.size(); ++i) {
        int x = startX + static_cast<int>(i) * (iconSize + spacing);
        QRect iconRect(x, y + 6, iconSize, iconSize);

        QColor color;
        switch (artifacts[i]->getRarity()) {
            case engine::CardRarity::Common:    color = QColor("#C0C0C0"); break;
            case engine::CardRarity::Uncommon:  color = QColor("#00D9FF"); break;
            case engine::CardRarity::Rare:      color = QColor("#FFD700"); break;
            case engine::CardRarity::Legendary: color = QColor("#E94560"); break;
            default: color = QColor("#808080"); break;
        }

        p.fillRect(iconRect, QColor("#0A0A0F"));
        p.setPen(QPen(color, 2));
        p.drawRect(iconRect);

        // Первая буква названия как иконка
        p.setFont(QFont("Courier New", 14, QFont::Bold));
        p.setPen(color);
        QString firstChar = QString::fromStdString(artifacts[i]->getName()).left(1).toUpper();
        p.drawText(iconRect, Qt::AlignCenter, firstChar);
    }
}

void ArtifactBarWidget::mouseMoveEvent(QMouseEvent* event) {
    const auto& artifacts = m_player->getArtifacts();
    int iconSize = 32;
    int spacing = 6;
    int startX = 8;
    int y = (height() - iconSize) / 2 + 6;

    int hovered = -1;
    for (size_t i = 0; i < artifacts.size(); ++i) {
        int x = startX + static_cast<int>(i) * (iconSize + spacing);
        QRect r(x, y, iconSize, iconSize);
        if (r.contains(event->pos())) {
            hovered = static_cast<int>(i);
            break;
        }
    }

    if (hovered != m_hoveredIndex) {
        m_hoveredIndex = hovered;
        if (hovered >= 0) {
            QString tooltip = QString("<b>%1</b><br>%2")
                .arg(QString::fromStdString(artifacts[hovered]->getName()))
                .arg(QString::fromStdString(artifacts[hovered]->getDescription()));
            QToolTip::showText(event->globalPos(), tooltip, this);
        } else {
            QToolTip::hideText();
        }
    }
}
