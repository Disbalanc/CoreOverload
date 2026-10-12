#include "CombatLogWidget.h"
#include "GameLog.h"
#include <QPainter>
#include <QLabel>
#include <QScrollBar>
#include <QTime>
#include <QDebug>

CombatLogWidget::CombatLogWidget(engine::EventBus* eventBus, QWidget* parent)
    : QWidget(parent)
    , m_eventBus(eventBus)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(3);

    auto* title = new QLabel("► МОНИТОР СИСТЕМЫ");
    title->setFont(QFont("Courier New", 9, QFont::Bold));
    title->setStyleSheet("color: #00FF41; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    m_logText = new QTextEdit();
    m_logText->setReadOnly(true);
    m_logText->setFont(QFont("Courier New", 8));
    m_logText->setStyleSheet(
        "QTextEdit {"
        "  background-color: #0F1923;"
        "  color: #C0C0C0;"
        "  border: 1px solid #16213E;"
        "  padding: 5px;"
        "}"
        "QScrollBar:vertical {"
        "  background: #1A1A2E;"
        "  width: 10px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #00FF41;"
        "  min-height: 20px;"
        "}"
    );
    layout->addWidget(m_logText);

    // Загружаем текущую историю логов
    refreshFromGameLog();

    // Подписываемся на обновления глобального лога
    connect(&GameLog::instance(), &GameLog::messageAdded, this,
        [this](const GameLog::Entry& e) {
            appendEntry(e.time, e.text, e.color);
        });

    // Если шина событий передана, подключаем игровой мониторинг
    if (m_eventBus) {
        subscribeToEvents();
    }
}

CombatLogWidget::~CombatLogWidget() {
    if (m_eventBus) {
        for (auto id : m_subs) {
            m_eventBus->unsubscribe(id);
        }
    }
}

void CombatLogWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));
    p.setPen(QPen(QColor("#16213E"), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void CombatLogWidget::refreshFromGameLog() {
    m_logText->clear();
    for (const auto& e : GameLog::instance().getEntries()) {
        appendEntry(e.time, e.text, e.color);
    }
}

void CombatLogWidget::appendEntry(const QString& time, const QString& text, const QString& color) {
    QString html = QString("<span style='color:%1;'>[%2] %3</span><br>")
        .arg(color).arg(time).arg(text.toHtmlEscaped());
    m_logText->moveCursor(QTextCursor::End);
    m_logText->insertHtml(html);
    m_logText->verticalScrollBar()->setValue(
        m_logText->verticalScrollBar()->maximum());
}

void CombatLogWidget::subscribeToEvents() {
    if (!m_eventBus) return;

    // 1. Урон по врагам
    m_subs.push_back(m_eventBus->subscribe(
        engine::EventType::DamageDealt,
        [](const engine::GameEvent& e) {
            int amt = e.getInt("amount");
            bool piercing = e.getBool("piercing", false);
            std::string src = e.getString("source");

            QString msg;
            if (src == "leak") {
                msg = QString("☣ Утечка нанесла %1 урона врагу").arg(amt);
            } else if (piercing) {
                msg = QString("⚡ Проникающий эксплойт: %1 урона в обход брони").arg(amt);
            } else {
                msg = QString("⚔ Нанесено %1 урона вражеской системе").arg(amt);
            }
            GameLog::instance().addMessage(msg, "#E94560");
        }
    ));

    // 2. Урон по игроку
    m_subs.push_back(m_eventBus->subscribe(
        engine::EventType::DamageReceived,
        [](const engine::GameEvent& e) {
            int amt = e.getInt("amount", 0);
            std::string src = e.getString("source");
            if (amt == 0) return;

            QString msg;
            if (src == "self") {
                msg = QString("⚠ Перегрев при разгоне: получено %1 урона").arg(amt);
            } else {
                msg = QString("💥 Безопасность завода атакует! Утрачено %1 HP").arg(amt);
            }
            GameLog::instance().addMessage(msg, "#FF6B35");
        }
    ));

    // 3. Получение файрвола (брони)
    m_subs.push_back(m_eventBus->subscribe(
        engine::EventType::BlockGained,
        [](const engine::GameEvent& e) {
            int amt = e.getInt("amount");
            GameLog::instance().addMessage(
                QString("🛡 Обновление конфигурации файрвола: +%1").arg(amt),
                "#00D9FF");
        }
    ));

    // 4. Розыгрыш карт
    m_subs.push_back(m_eventBus->subscribe(
        engine::EventType::CardPlayed,
        [](const engine::GameEvent& e) {
            std::string name = e.getString("card_name");
            GameLog::instance().addMessage(
                QString("▶ Исполнение: '%1'").arg(QString::fromStdString(name)),
                "#00FF41");
        }
    ));

    // 5. Перетасовка колоды
    m_subs.push_back(m_eventBus->subscribe(
        engine::EventType::DeckShuffled,
        [](const engine::GameEvent& e) {
            int count = e.getInt("count");
            GameLog::instance().addMessage(
                QString("🔄 Переполнение стека сброса. %1 скриптов возвращено в память").arg(count),
                "#00D9FF");
        }
    ));
}
