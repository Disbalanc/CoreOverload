#include "MapScreen.h"
#include "GameLog.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>

MapScreen::MapScreen(engine::WorldMap& map, engine::Player& player, QWidget* parent)
    : QWidget(parent)
    , m_map(map)
    , m_player(player)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);

    auto* titleLabel = new QLabel("СЕТЕВАЯ КАРТА ЗАВОДА — АКТ 1: ЛИТОГРАФИЯ");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Courier New", 14, QFont::Bold));
    titleLabel->setStyleSheet("color: #00FF41; background-color: #16213E; padding: 10px; border: 1px solid #00FF41;");
    mainLayout->addWidget(titleLabel);

    auto* hpLabel = new QLabel(
        QString("ЦЕЛОСТНОСТЬ: %1/%2 | КАРТ В КОЛОДЕ: %3")
            .arg(m_player.getHp())
            .arg(m_player.getMaxHp())
            .arg(m_player.getDeck().getMasterDeck().size())
    );
    hpLabel->setAlignment(Qt::AlignCenter);
    hpLabel->setFont(QFont("Courier New", 10));
    hpLabel->setStyleSheet("color: #E94560; padding: 5px;");
    mainLayout->addWidget(hpLabel);

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { background: #0A0A0F; border: 1px solid #16213E; }");

    auto* content = new QWidget();
    content->setStyleSheet("background: #0A0A0F;");
    m_buttonsLayout = new QVBoxLayout(content);
    m_buttonsLayout->setAlignment(Qt::AlignCenter);

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);

    rebuild();
    GameLog::instance().addMessage("🗺 Открыта сетевая карта", "#808080");
}

void MapScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));
}

void MapScreen::rebuild() {
    // Чистим layout
    QLayoutItem* item;
    while ((item = m_buttonsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    const auto& floors = m_map.getFloors();

    // Рисуем сверху вниз (от босса к старту)
    for (int f = static_cast<int>(floors.size()) - 1; f >= 0; --f) {
        auto* rowLabel = new QLabel(QString("── ЭТАЖ %1 ──").arg(f));
        rowLabel->setAlignment(Qt::AlignCenter);
        rowLabel->setFont(QFont("Courier New", 9, QFont::Bold));
        rowLabel->setStyleSheet("color: #808080; margin-top: 10px;");
        m_buttonsLayout->addWidget(rowLabel);

        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->setSpacing(15);

        for (int nodeId : floors[f]) {
            const auto* node = m_map.getNode(nodeId);
            if (!node) continue;

            QString icon;
            QString color = "#808080";
            switch (node->type) {
                case engine::NodeType::Battle:      icon = "⚔ БОЙ";       color = "#E94560"; break;
                case engine::NodeType::EliteBattle: icon = "💀 ЭЛИТА";    color = "#FF6B35"; break;
                case engine::NodeType::Rest:        icon = "🔧 ОТДЫХ";    color = "#00D9FF"; break;
                case engine::NodeType::Event:       icon = "❓ ПОРТ";     color = "#533483"; break;
                case engine::NodeType::Shop:        icon = "💾 РЫНОК";    color = "#FFD700"; break;
                case engine::NodeType::BossBattle:  icon = "👑 БОСС";     color = "#E94560"; break;
            }

            auto* btn = new QPushButton(icon);
            btn->setFixedSize(130, 50);
            btn->setFont(QFont("Courier New", 10, QFont::Bold));

            if (node->visited) {
                btn->setEnabled(false);
                btn->setStyleSheet("background-color: #0A0A0F; color: #444444; border: 1px solid #333333;");
                btn->setText("✔ " + icon);
            } else if (node->available) {
                btn->setStyleSheet(QString(
                    "QPushButton { background-color: #0F3460; color: %1; border: 2px solid %1; }"
                    "QPushButton:hover { background-color: %1; color: #0A0A0F; }"
                ).arg(color));
                int id = node->id;
                connect(btn, &QPushButton::clicked, this, [this, id]() {
                    emit nodeSelected(id);
                });
            } else {
                btn->setEnabled(false);
                btn->setStyleSheet("background-color: #16213E; color: #555555; border: 1px solid #333333;");
            }

            row->addWidget(btn);
        }

        auto* rowContainer = new QWidget();
        rowContainer->setLayout(row);
        rowContainer->setStyleSheet("background: transparent;");
        m_buttonsLayout->addWidget(rowContainer);
    }
}
