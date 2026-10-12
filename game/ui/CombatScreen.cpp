#include "CombatScreen.h"
#include "CombatLogWidget.h"
#include "DeckViewDialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

CombatScreen::CombatScreen(engine::CombatManager& combat,
                           engine::Player& player,
                           engine::EventBus& eventBus,
                           QWidget* parent)
    : QWidget(parent)
    , m_combat(combat)
    , m_player(player)
    , m_eventBus(eventBus)
    , m_combatEndEmitted(false)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(5);

    // ── 1. Верхняя полоса ─────────────────────────────────
    auto* topBar = new QHBoxLayout();
    m_turnLabel = new QLabel("ХОД 1");
    m_turnLabel->setFont(QFont("Courier New", 11, QFont::Bold));
    m_turnLabel->setStyleSheet("color: #808080; background: transparent; padding: 4px;");
    m_turnLabel->setFixedWidth(100);
    topBar->addWidget(m_turnLabel);

    m_statusBanner = new QLabel("ВЫБЕРИТЕ КАРТУ");
    m_statusBanner->setAlignment(Qt::AlignCenter);
    m_statusBanner->setFont(QFont("Courier New", 11, QFont::Bold));
    m_statusBanner->setStyleSheet(
        "color: #00FF41; background-color: #16213E; padding: 6px; border: 1px solid #00FF41;");
    topBar->addWidget(m_statusBanner);

    root->addLayout(topBar);

    // Панель артефактов
    m_artifactBar = new ArtifactBarWidget(&m_player, this);
    m_artifactBar->setFixedHeight(50);
    root->addWidget(m_artifactBar);

    // ── 2. ОСНОВНАЯ ЗОНА: Враги + Лог справа ──────────────
    auto* middleBar = new QHBoxLayout();
    middleBar->setSpacing(10);

    // Враги (центр)
    m_enemiesContainer = new QWidget();
    m_enemiesContainer->setMinimumHeight(230);
    m_enemiesContainer->setStyleSheet("background: transparent;");
    m_enemiesLayout = new QHBoxLayout(m_enemiesContainer);
    m_enemiesLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_enemiesLayout->setSpacing(30);
    m_enemiesLayout->setContentsMargins(15, 10, 15, 10);
    middleBar->addWidget(m_enemiesContainer, 3);

    // Лог боя (справа)
    auto* logWidget = new CombatLogWidget(&m_eventBus, this);
    logWidget->setFixedWidth(280);
    middleBar->addWidget(logWidget);

    root->addLayout(middleBar, 3);

    // ── 3. HUD + Кнопки колоды ────────────────────────────
    auto* hudBar = new QHBoxLayout();
    hudBar->setSpacing(5);

    m_playerHud = new PlayerHudWidget(&m_player, this);
    m_playerHud->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hudBar->addWidget(m_playerHud, 1);

    // Кнопки просмотра колоды
    auto* deckBtnsLayout = new QVBoxLayout();
    deckBtnsLayout->setSpacing(3);

    auto* drawBtn = new QPushButton("КОЛОДА");
    drawBtn->setFixedSize(90, 35);
    drawBtn->setFont(QFont("Courier New", 8, QFont::Bold));
    drawBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 1px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }"
    );
    connect(drawBtn, &QPushButton::clicked, [this]() {
        DeckViewDialog dlg(m_player, DeckViewDialog::DrawPile, this);
        dlg.exec();
    });
    deckBtnsLayout->addWidget(drawBtn);

    auto* discardBtn = new QPushButton("СБРОС");
    discardBtn->setFixedSize(90, 35);
    discardBtn->setFont(QFont("Courier New", 8, QFont::Bold));
    discardBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #E94560; border: 1px solid #E94560; }"
        "QPushButton:hover { background-color: #E94560; color: #0A0A0F; }"
    );
    connect(discardBtn, &QPushButton::clicked, [this]() {
        DeckViewDialog dlg(m_player, DeckViewDialog::DiscardPile, this);
        dlg.exec();
    });
    deckBtnsLayout->addWidget(discardBtn);

    hudBar->addLayout(deckBtnsLayout);

    root->addLayout(hudBar);

    // ── 4. Рука карт + кнопка хода ────────────────────────
    auto* bottomBar = new QHBoxLayout();
    bottomBar->setContentsMargins(0, 0, 0, 0);
    bottomBar->setSpacing(10);

    m_handWidget = new HandWidget(this);
    m_handWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_handWidget, &HandWidget::cardClicked, this, &CombatScreen::onCardClicked);
    bottomBar->addWidget(m_handWidget, 1);

    m_endTurnBtn = new QPushButton("КОНЕЦ\nХОДА");
    m_endTurnBtn->setFixedSize(100, 65);
    m_endTurnBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    m_endTurnBtn->setStyleSheet(
        "QPushButton { background-color: #E94560; color: #FFFFFF; border: 2px solid #FFFFFF; }"
        "QPushButton:hover { background-color: #FF6B35; }"
        "QPushButton:disabled { background-color: #333; color: #888; border-color: #555; }"
    );
    connect(m_endTurnBtn, &QPushButton::clicked, this, &CombatScreen::onEndTurnClicked);
    bottomBar->addWidget(m_endTurnBtn);

    root->addLayout(bottomBar);

    // ── Подписка на конец боя ─────────────────────────────
    m_subscriptions.push_back(m_eventBus.subscribe(
        engine::EventType::CombatEnded,
        [this](const engine::GameEvent& e) {
            bool won = e.getBool("player_won", false);
            QMetaObject::invokeMethod(this, [this, won]() {
                m_endTurnBtn->setEnabled(false);
                if (won) {
                    updateStatusBanner("✓ СИСТЕМЫ УНИЧТОЖЕНЫ!", "#00FF41");
                } else {
                    updateStatusBanner("✗ КРИТИЧЕСКИЙ СБОЙ!", "#E94560");
                }
                refreshUI();

                if (!m_combatEndEmitted) {
                    m_combatEndEmitted = true;
                    QTimer::singleShot(2000, this, [this, won]() {
                        emit combatFinished(won);
                    });
                }
            }, Qt::QueuedConnection);
        }
    ));

    // ToastNotification — поверх всего, в правом верхнем углу
    m_toastWidget = new ToastNotification(this);
    m_toastWidget->setGeometry(0, 60, width(), 300);
    m_toastWidget->raise();
    m_toastWidget->show();

    // Подписка на события для тостов
    m_subscriptions.push_back(m_eventBus.subscribe(
        engine::EventType::CardDrawn,
        [this](const engine::GameEvent& e) {
            int count = e.getInt("count");
            QMetaObject::invokeMethod(this, [this, count]() {
                m_toastWidget->showMessage(
                    QString("↓ Взято карт: %1").arg(count), "#FFD700", 2000);
            }, Qt::QueuedConnection);
        }));

    m_subscriptions.push_back(m_eventBus.subscribe(
        engine::EventType::DeckShuffled,
        [this](const engine::GameEvent& e) {
            int count = e.getInt("count");
            QMetaObject::invokeMethod(this, [this, count]() {
                m_toastWidget->showMessage(
                    QString("🔄 Перетасовка: %1 карт").arg(count), "#00D9FF", 2500);
            }, Qt::QueuedConnection);
        }));

    m_subscriptions.push_back(m_eventBus.subscribe(
        engine::EventType::CardPlayed,
        [this](const engine::GameEvent& e) {
            std::string name = e.getString("card_name");
            QMetaObject::invokeMethod(this, [this, name]() {
                m_toastWidget->showMessage(
                    QString("▶ %1").arg(QString::fromStdString(name)), "#00FF41", 1500);
            }, Qt::QueuedConnection);
        }));

    // Отложенный refreshUI, когда виджеты получат корректный размер
    QTimer::singleShot(50, this, &CombatScreen::refreshUI);
}

CombatScreen::~CombatScreen() {
    for (auto id : m_subscriptions) {
        m_eventBus.unsubscribe(id);
    }
}

void CombatScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));

    p.setPen(QPen(QColor(0, 255, 65, 6)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }
}

void CombatScreen::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        cancelCardSelection();
    }
    QWidget::mousePressEvent(event);
}

void CombatScreen::cancelCardSelection() {
    if (m_selectedCardIndex != -1) {
        m_selectedCardIndex = -1;
        updateStatusBanner("Выбор отменён.", "#808080");
        refreshUI();
    }
}

void CombatScreen::updateStatusBanner(const QString& text, const QString& colorHex) {
    m_statusBanner->setText(text);
    m_statusBanner->setStyleSheet(
        QString("color: %1; background-color: #16213E; padding: 6px; border: 1px solid %1;")
            .arg(colorHex));
}

void CombatScreen::refreshUI() {
     if (m_artifactBar) m_artifactBar->update();
    m_turnLabel->setText(QString("ХОД %1").arg(m_combat.getTurnNumber()));
    m_playerHud->update();

    // Враги
    QLayoutItem* item;
    while ((item = m_enemiesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const auto& enemies = m_combat.getEnemies();
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto* ew = new EnemyWidget(enemies[i].get(), static_cast<int>(i), this);

        if (m_selectedCardIndex != -1) {
            const auto& hand = m_player.getDeck().getHand();
            if (m_selectedCardIndex < static_cast<int>(hand.size())) {
                if (hand[m_selectedCardIndex]->getTarget() == engine::CardTarget::SingleEnemy) {
                    ew->setTargetable(true);
                }
            }
        }

        connect(ew, &EnemyWidget::enemyClicked, this, &CombatScreen::onEnemyClicked);
        m_enemiesLayout->addWidget(ew);
    }

    // Рука
    m_handWidget->rebuild(m_player, m_selectedCardIndex);

    m_endTurnBtn->setEnabled(m_combat.isPlayerTurn() && !m_combat.isCombatOver());
}

void CombatScreen::onCardClicked(int handIndex) {
    if (m_combat.isCombatOver() || !m_combat.isPlayerTurn()) return;

    const auto& hand = m_player.getDeck().getHand();
    if (handIndex >= static_cast<int>(hand.size())) return;

    const auto* card = hand[handIndex];

    if (card->getCurrentCost() > m_player.getEnergy()) {
        updateStatusBanner("⚠ НЕДОСТАТОЧНО ЦПУ!", "#FF6B35");
        return;
    }

    if (m_selectedCardIndex == handIndex) {
        cancelCardSelection();
        return;
    }

    if (card->getTarget() == engine::CardTarget::Self ||
        card->getTarget() == engine::CardTarget::AllEnemies ||
        card->getTarget() == engine::CardTarget::None)
    {
        m_selectedCardIndex = -1;
        m_combat.playCard(handIndex, -1);
        refreshUI();
        return;
    }

    m_selectedCardIndex = handIndex;
    updateStatusBanner(
        QString("ВЫБЕРИТЕ ЦЕЛЬ для '%1' [ПКМ - отмена]")
            .arg(QString::fromStdString(card->getName())),
        "#00D9FF");
    refreshUI();
}

void CombatScreen::onEnemyClicked(int enemyIndex) {
    if (m_combat.isCombatOver() || m_selectedCardIndex == -1) return;

    m_combat.playCard(m_selectedCardIndex, enemyIndex);
    m_selectedCardIndex = -1;
    refreshUI();
}

void CombatScreen::onEndTurnClicked() {
    if (m_combat.isCombatOver() || !m_combat.isPlayerTurn()) return;

    cancelCardSelection();
    m_combat.endPlayerTurn();
    refreshUI();
}

void CombatScreen::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_toastWidget) {
        m_toastWidget->setGeometry(0, 60, width(), 300);
        m_toastWidget->raise();
    }
}
