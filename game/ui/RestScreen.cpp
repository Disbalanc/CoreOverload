#include "RestScreen.h"
#include "GameLog.h"
#include "CardWidget.h"
#include <engine/cards/CardDatabase.h>
#include <QVBoxLayout>
#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <QPainter>

RestScreen::RestScreen(engine::Player& player, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(60, 50, 60, 50);
    root->setSpacing(20);
    root->setAlignment(Qt::AlignCenter);

    m_title = new QLabel("🔧 ТЕРМИНАЛ ОТЛАДКИ");
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setFont(QFont("Courier New", 16, QFont::Bold));
    m_title->setStyleSheet("color: #00D9FF; background-color: #16213E; padding: 12px; border: 2px solid #00D9FF;");
    root->addWidget(m_title);

    auto* desc = new QLabel(
        "Безопасная зона. Вы можете использовать время для одного действия:"
    );
    desc->setAlignment(Qt::AlignCenter);
    desc->setFont(QFont("Courier New", 10));
    desc->setStyleSheet("color: #C0C0C0; background: transparent;");
    root->addWidget(desc);

    root->addStretch();

    QString btnStyle =
        "QPushButton { background-color: #0F3460; color: %1; border: 2px solid %1; padding: 10px; }"
        "QPushButton:hover { background-color: %1; color: #0A0A0F; }"
        "QPushButton:disabled { background-color: #1A1A1A; color: #555; border-color: #333; }";

    m_repairBtn = new QPushButton("⚕ ВОССТАНОВИТЬ ЦЕЛОСТНОСТЬ (+15 HP)");
    m_repairBtn->setFixedSize(500, 55);
    m_repairBtn->setFont(QFont("Courier New", 11, QFont::Bold));
    m_repairBtn->setStyleSheet(btnStyle.arg("#00FF41"));
    connect(m_repairBtn, &QPushButton::clicked, this, &RestScreen::onRepair);
    root->addWidget(m_repairBtn, 0, Qt::AlignCenter);

    m_upgradeBtn = new QPushButton("⚡ УЛУЧШИТЬ КАРТУ (APGRADE++)");
    m_upgradeBtn->setFixedSize(500, 55);
    m_upgradeBtn->setFont(QFont("Courier New", 11, QFont::Bold));
    m_upgradeBtn->setStyleSheet(btnStyle.arg("#FFD700"));
    connect(m_upgradeBtn, &QPushButton::clicked, this, &RestScreen::onUpgrade);
    root->addWidget(m_upgradeBtn, 0, Qt::AlignCenter);

    m_skipBtn = new QPushButton("ПРОДОЛЖИТЬ БЕЗ ДЕЙСТВИЙ");
    m_skipBtn->setFixedSize(500, 45);
    m_skipBtn->setFont(QFont("Courier New", 10));
    m_skipBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #808080; border: 1px solid #808080; }"
        "QPushButton:hover { background-color: #808080; color: #0A0A0F; }"
    );
    connect(m_skipBtn, &QPushButton::clicked, this, &RestScreen::onSkip);
    root->addWidget(m_skipBtn, 0, Qt::AlignCenter);

    root->addStretch();

    GameLog::instance().addMessage("🔧 Прибытие в терминал отладки", "#00D9FF");
}

void RestScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));
    p.setPen(QPen(QColor(0, 217, 255, 8)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }
}

void RestScreen::onRepair() {
    if (m_actionTaken) return;
    m_actionTaken = true;

    int before = m_player.getHp();
    m_player.heal(15);
    int gained = m_player.getHp() - before;

    GameLog::instance().addMessage(
        QString("⚕ Восстановлено +%1 HP (%2/%3)")
            .arg(gained)
            .arg(m_player.getHp())
            .arg(m_player.getMaxHp()),
        "#00FF41");

    emit restFinished();
}

void RestScreen::onUpgrade() {
    if (m_actionTaken) return;
    showUpgradeSelector();
}

void RestScreen::onSkip() {
    GameLog::instance().addMessage("🔧 Покинули терминал без действий", "#808080");
    emit restFinished();
}

void RestScreen::showUpgradeSelector() {
    // Создаём диалог выбора карты для улучшения
    QDialog dlg(this);
    dlg.setWindowTitle("ВЫБЕРИТЕ КАРТУ ДЛЯ УЛУЧШЕНИЯ");
    dlg.setStyleSheet("QDialog { background-color: #0A0A0F; }");
    dlg.resize(900, 600);

    auto* layout = new QVBoxLayout(&dlg);

    auto* title = new QLabel("КЛИКНИТЕ НА КАРТУ, ЧТОБЫ УЛУЧШИТЬ ЕЁ");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Courier New", 12, QFont::Bold));
    title->setStyleSheet("color: #FFD700; background-color: #16213E; padding: 8px; border: 1px solid #FFD700;");
    layout->addWidget(title);

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { background: #0A0A0F; border: 1px solid #16213E; }"
        "QScrollBar:vertical { background: #1A1A2E; width: 10px; }"
        "QScrollBar::handle:vertical { background: #FFD700; min-height: 20px; }"
    );

    auto* content = new QWidget();
    content->setStyleSheet("background: #0A0A0F;");

    // Собираем все НЕ улучшенные карты
    auto& master = m_player.getDeck().getMasterDeck();
    int cardW = CardWidget::CARD_WIDTH;
    int cardH = CardWidget::CARD_HEIGHT;
    int cols = 5;
    int spacing = 15;

    int totalRows = (static_cast<int>(master.size()) + cols - 1) / cols;
    content->setMinimumSize(cols * (cardW + spacing) + 40,
                            totalRows * (cardH + spacing) + 40);

    int visibleIdx = 0;
    for (size_t i = 0; i < master.size(); ++i) {
        if (master[i]->isUpgraded() || master[i]->getRarity() == engine::CardRarity::Curse) {
            continue;
        }

        auto* cw = new CardWidget(master[i].get(), visibleIdx, content);
        int row = visibleIdx / cols;
        int col = visibleIdx % cols;
        int x = 20 + col * (cardW + spacing);
        int y = 20 + row * (cardH + spacing);
        cw->setBasePosition(QPoint(x, y));
        cw->move(x, y);
        cw->show();

        std::string cardId = master[i]->getId();
        // Отрезаем числовой суффикс
        auto underscore = cardId.rfind('_');
        std::string templateId = (underscore != std::string::npos) ? cardId.substr(0, underscore) : cardId;
        engine::Card* cardPtr = master[i].get();

        connect(cw, &CardWidget::cardClicked, [this, templateId, cardPtr, &dlg]() {
            auto* tmpl = engine::CardDatabase::instance().getTemplate(templateId);
            if (tmpl) {
                cardPtr->upgradeFromTemplate(
                    tmpl->upgradedName.empty() ? tmpl->name : tmpl->upgradedName,
                    tmpl->upgradedDescription,
                    tmpl->upgradedCost,
                    tmpl->upgradedEffects
                );
                GameLog::instance().addMessage(
                    QString("⚡ Улучшена карта: %1 → %1+")
                        .arg(QString::fromStdString(tmpl->name)),
                    "#FFD700");
                m_actionTaken = true;
                dlg.accept();
            }
        });

        visibleIdx++;
    }

    if (visibleIdx == 0) {
        auto* empty = new QLabel("Все карты уже улучшены или не могут быть улучшены.", content);
        empty->setStyleSheet("color: #808080; padding: 50px;");
        empty->setAlignment(Qt::AlignCenter);
    }

    scroll->setWidget(content);
    layout->addWidget(scroll);

    auto* cancelBtn = new QPushButton("ОТМЕНА");
    cancelBtn->setFixedSize(200, 40);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #808080; border: 1px solid #808080; }"
        "QPushButton:hover { background-color: #808080; color: #0A0A0F; }"
    );
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    layout->addWidget(cancelBtn, 0, Qt::AlignCenter);

    if (dlg.exec() == QDialog::Accepted && m_actionTaken) {
        emit restFinished();
    }
}
