#include "EventScreen.h"
#include "GameLog.h"
#include <QPainter>

EventScreen::EventScreen(engine::Player& player, engine::Random& rng, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
    , m_rng(rng)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(60, 40, 60, 40);
    root->setSpacing(15);
    root->setAlignment(Qt::AlignCenter);

    m_titleLabel = new QLabel();
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFont(QFont("Courier New", 16, QFont::Bold));
    m_titleLabel->setStyleSheet("color: #533483; background-color: #16213E; padding: 12px; border: 2px solid #533483;");
    root->addWidget(m_titleLabel);

    m_descLabel = new QLabel();
    m_descLabel->setWordWrap(true);
    m_descLabel->setAlignment(Qt::AlignCenter);
    m_descLabel->setFont(QFont("Courier New", 11));
    m_descLabel->setStyleSheet("color: #C0C0C0; background: transparent; padding: 20px;");
    root->addWidget(m_descLabel);

    m_optionsLayout = new QVBoxLayout();
    m_optionsLayout->setSpacing(10);
    m_optionsLayout->setAlignment(Qt::AlignCenter);
    root->addLayout(m_optionsLayout);

    m_resultLabel = new QLabel();
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setFont(QFont("Courier New", 10, QFont::Bold));
    m_resultLabel->setStyleSheet("color: #00FF41; background: transparent; padding: 15px;");
    m_resultLabel->hide();
    root->addWidget(m_resultLabel);

    m_continueBtn = new QPushButton("ПРОДОЛЖИТЬ СКАНИРОВАНИЕ СЕТИ");
    m_continueBtn->setFixedSize(400, 50);
    m_continueBtn->setFont(QFont("Courier New", 11, QFont::Bold));
    m_continueBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 2px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }"
    );
    m_continueBtn->hide();
    connect(m_continueBtn, &QPushButton::clicked, this, &EventScreen::eventFinished);
    root->addWidget(m_continueBtn, 0, Qt::AlignCenter);

    // ── Генерация случайного события ──────────────────────
    std::vector<EventData> pool;

    // Событие 1: Заброшенный терминал
    pool.push_back({
        "⚠ ЗАБРОШЕННЫЙ ТЕРМИНАЛ",
        "Вы обнаружили старый сервер со странным протоколом. На экране мигает: "
        "'ВВЕДИТЕ ПАРОЛЬ'. Попытка взлома может дать ценные данные, но защитные "
        "системы могут обнаружить вас.",
        {
            {"[ВЗЛОМАТЬ ТЕРМИНАЛ]", [this]() {
                if (m_rng.rollChance(0.6f)) {
                    m_player.heal(10);
                    return QString("✓ Взлом успешен! Найден файл с патчами. Восстановлено 10 HP.");
                } else {
                    int dmg = 8;
                    m_player.takeDamage(dmg, true);
                    return QString("✗ Сработала ловушка! Получено %1 урона.").arg(dmg);
                }
            }},
            {"[ПРОЙТИ МИМО]", []() {
                return QString("Вы осторожно обходите терминал. Ничего не произошло.");
            }}
        }
    });

    // Событие 2: Перебежчик-андроид
    pool.push_back({
        "🤖 АНДРОИД-ПЕРЕБЕЖЧИК",
        "Из тени выходит повреждённый андроид. 'Я устал служить корпорации. "
        "Возьми мою память - там код, который может помочь. Но я должен закрыться "
        "здесь навсегда...'",
        {
            {"[ПРИНЯТЬ ДАР]", [this]() {
                m_player.gainBlock(0); // Заглушка — в будущем добавить карту-награду
                return QString("✓ Загружен новый эксплойт. Проверьте терминалом отладки.");
            }},
            {"[РАЗОБРАТЬ НА ЗАПЧАСТИ]", [this]() {
                m_player.heal(15);
                return QString("✓ Разобран на компоненты. Восстановлено 15 HP.");
            }},
            {"[ИГНОРИРОВАТЬ]", []() {
                return QString("Вы оставляете андроида наедине с его цифровой смертью.");
            }}
        }
    });

    // Событие 3: Утечка охлаждающей жидкости
    pool.push_back({
        "💧 УТЕЧКА ОХЛАЖДАЮЩЕЙ ЖИДКОСТИ",
        "Трубопровод охлаждения прорвало. Пол залит едкой жидкостью. "
        "Можно пройти напролом или сделать длинный обход.",
        {
            {"[ПРОЙТИ НАПРОЛОМ]", [this]() {
                int dmg = 5;
                m_player.takeDamage(dmg, true);
                return QString("✗ Получено %1 урона от кислоты, но путь свободен.").arg(dmg);
            }},
            {"[СДЕЛАТЬ ОБХОД]", []() {
                return QString("Вы потратили время, но прошли без потерь.");
            }}
        }
    });

    // Выбираем случайное событие
    int idx = m_rng.rollInt(0, static_cast<int>(pool.size()) - 1);
    setupEvent(pool[idx]);
    GameLog::instance().addMessage(
        QString("❓ Событие: %1").arg(pool[idx].title),
        "#533483");
}

void EventScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));

    p.setPen(QPen(QColor(83, 52, 131, 10)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }
}

void EventScreen::setupEvent(const EventData& data) {
    m_currentEvent = data;
    m_titleLabel->setText(data.title);
    m_descLabel->setText(data.description);

    // Чистим старые опции
    QLayoutItem* item;
    while ((item = m_optionsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Создаём кнопки опций
    for (int i = 0; i < static_cast<int>(data.options.size()); ++i) {
        auto* btn = new QPushButton(data.options[i].text);
        btn->setFixedSize(500, 45);
        btn->setFont(QFont("Courier New", 10, QFont::Bold));
        btn->setStyleSheet(
            "QPushButton { background-color: #16213E; color: #00D9FF; border: 1px solid #00D9FF; }"
            "QPushButton:hover { background-color: #00D9FF; color: #0A0A0F; }"
        );
        connect(btn, &QPushButton::clicked, this, [this, i]() { onOptionSelected(i); });
        m_optionsLayout->addWidget(btn, 0, Qt::AlignCenter);
    }
}

void EventScreen::onOptionSelected(int index) {
    if (index < 0 || index >= static_cast<int>(m_currentEvent.options.size())) return;

    QString result = m_currentEvent.options[index].action();
    GameLog::instance().addMessage(result, "#C0C0C0");
    // Прячем опции, показываем результат
    for (int i = 0; i < m_optionsLayout->count(); ++i) {
        auto* w = m_optionsLayout->itemAt(i)->widget();
        if (w) w->hide();
    }

    m_resultLabel->setText(result);
    m_resultLabel->show();
    m_continueBtn->show();
}
