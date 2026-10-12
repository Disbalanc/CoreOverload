#include "RewardScreen.h"
#include "CardWidget.h"
#include <engine/cards/CardDatabase.h>
#include <QPainter>

RewardScreen::RewardScreen(engine::Player& player, engine::Random& rng, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
    , m_rng(rng)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 30, 40, 30);
    root->setAlignment(Qt::AlignCenter);

    // Заголовок
    m_titleLabel = new QLabel("ОБНАРУЖЕНЫ ДАННЫЕ: ВЫБЕРИТЕ НОВЫЙ ПРОЦЕСС");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFont(QFont("Courier New", 14, QFont::Bold));
    m_titleLabel->setStyleSheet("color: #00FF41; background-color: #16213E; padding: 10px; border: 1px solid #00FF41;");
    root->addWidget(m_titleLabel);

    root->addStretch(1);

    // Контейнер карт — всегда по центру
    m_cardsContainer = new QWidget();
    m_cardsContainer->setFixedHeight(CardWidget::CARD_HEIGHT + 20);
    m_cardsContainer->setStyleSheet("background: transparent;");
    m_cardsLayout = new QHBoxLayout(m_cardsContainer);
    m_cardsLayout->setAlignment(Qt::AlignCenter);
    m_cardsLayout->setSpacing(25);
    root->addWidget(m_cardsContainer, 0, Qt::AlignCenter);

    // Подсказка
    m_infoLabel = new QLabel("Кликните по карте для предварительного выбора, затем подтвердите.");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setFont(QFont("Courier New", 9));
    m_infoLabel->setStyleSheet("color: #808080; background: transparent; padding: 8px;");
    root->addWidget(m_infoLabel);

    root->addStretch(1);

    // Кнопки
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);
    btnLayout->setSpacing(20);

    m_confirmBtn = new QPushButton("УСТАНОВИТЬ ВЫБРАННЫЙ ПРОЦЕСС");
    m_confirmBtn->setFixedSize(320, 45);
    m_confirmBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    m_confirmBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 2px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }"
        "QPushButton:disabled { background-color: #1A1A1A; color: #555; border-color: #333; }"
    );
    m_confirmBtn->setEnabled(false);
    connect(m_confirmBtn, &QPushButton::clicked, this, &RewardScreen::onConfirmClicked);
    btnLayout->addWidget(m_confirmBtn);

    m_skipBtn = new QPushButton("ПРОПУСТИТЬ");
    m_skipBtn->setFixedSize(180, 45);
    m_skipBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    m_skipBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #808080; border: 1px solid #808080; }"
        "QPushButton:hover { background-color: #808080; color: #0A0A0F; }"
    );
    connect(m_skipBtn, &QPushButton::clicked, this, &RewardScreen::onSkipClicked);
    btnLayout->addWidget(m_skipBtn);

    root->addLayout(btnLayout);

    generateRewards();
    rebuildCards();

    // Если включён бонус артефакта — создадим его
    if (m_offerArtifact) {
        auto& db = engine::ArtifactDatabase::instance();
        auto ids = db.getAllIds();
        if (!ids.empty()) {
            m_rng.shuffle(ids);
            m_artifactReward = db.createArtifact(ids[0]);

            // Создадим кнопку артефакта
            auto* artBtn = new QPushButton(
                QString("🎁 ПОЛУЧИТЬ АРТЕФАКТ: %1\n«%2»")
                    .arg(QString::fromStdString(m_artifactReward->getName()))
                    .arg(QString::fromStdString(m_artifactReward->getDescription())));
            artBtn->setFixedSize(500, 70);
            artBtn->setFont(QFont("Courier New", 10, QFont::Bold));
            artBtn->setStyleSheet(
                "QPushButton { background-color: #FFD700; color: #0A0A0F; border: 2px solid #FFD700; }"
                "QPushButton:hover { background-color: #FFF; }"
            );
            connect(artBtn, &QPushButton::clicked, [this]() {
                if (m_artifactReward) {
                    GameLog::instance().addMessage(
                        QString("🎁 Получен артефакт: %1")
                            .arg(QString::fromStdString(m_artifactReward->getName())),
                        "#FFD700");
                    m_player.addArtifact(std::move(m_artifactReward));
                }
                emit rewardClaimed();
            });
            layout()->addWidget(artBtn);
        }
    }
}

void RewardScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));

    p.setPen(QPen(QColor(0, 255, 65, 6)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }
}

void RewardScreen::generateRewards() {
    m_rewardCards.clear();

    // Берём из БД все карты, кроме curse
    auto& db = engine::CardDatabase::instance();
    std::vector<std::string> pool;
    for (const auto& id : db.getAllIds()) {
        auto* t = db.getTemplate(id);
        if (t && t->rarity != engine::CardRarity::Curse) {
            pool.push_back(id);
        }
    }

    m_rng.shuffle(pool);
    int count = std::min(3, static_cast<int>(pool.size()));

    for (int i = 0; i < count; ++i) {
        auto card = db.createCard(pool[i], 2000 + i);
        if (card) {
            m_rewardCards.push_back(std::move(card));
        }
    }
}

void RewardScreen::rebuildCards() {
    // Удаляем старые виджеты
    for (auto* child : m_cardsContainer->findChildren<CardWidget*>()) {
        child->deleteLater();
    }

    int count = static_cast<int>(m_rewardCards.size());
    if (count == 0) return;

    // Задаём фиксированный размер контейнера
    int cardW = CardWidget::CARD_WIDTH;
    int spacing = 30;
    int totalW = count * cardW + (count - 1) * spacing;
    m_cardsContainer->setFixedWidth(totalW + 40);
    m_cardsContainer->setFixedHeight(CardWidget::CARD_HEIGHT + 60);

    // Создаём и позиционируем карты вручную
    for (int i = 0; i < count; ++i) {
        auto* cw = new CardWidget(m_rewardCards[i].get(), i, m_cardsContainer);
        if (i == m_selectedRewardIndex) {
            cw->setSelected(true);
        }
        connect(cw, &CardWidget::cardClicked, this, &RewardScreen::onCardSelected);

        int x = 20 + i * (cardW + spacing);
        int y = 30;
        cw->setBasePosition(QPoint(x, y));
        cw->move(x, y);
        cw->show();
    }
}

void RewardScreen::onCardSelected(int index) {
    if (m_selectedRewardIndex == index) {
        // Повторный клик — отмена
        m_selectedRewardIndex = -1;
        m_confirmBtn->setEnabled(false);
        m_infoLabel->setText("Выбор отменён. Кликните по карте для выбора.");
        m_infoLabel->setStyleSheet("color: #808080; background: transparent; padding: 8px;");
    } else {
        m_selectedRewardIndex = index;
        m_confirmBtn->setEnabled(true);
        m_infoLabel->setText(
            QString("Выбран: '%1'. Нажмите УСТАНОВИТЬ или кликните снова для отмены.")
                .arg(QString::fromStdString(m_rewardCards[index]->getName())));
        m_infoLabel->setStyleSheet("color: #00D9FF; background: transparent; padding: 8px;");
    }
    rebuildCards();
}

void RewardScreen::onConfirmClicked() {
    if (m_selectedRewardIndex < 0 || m_selectedRewardIndex >= static_cast<int>(m_rewardCards.size())) return;

    m_player.getDeck().addToMasterDeck(std::move(m_rewardCards[m_selectedRewardIndex]));
    emit rewardClaimed();
}

void RewardScreen::onSkipClicked() {
    emit rewardClaimed();
}
