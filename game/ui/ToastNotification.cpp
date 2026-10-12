#include "ToastNotification.h"
#include <QPainter>

ToastNotification::ToastNotification(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(5);
    m_layout->setAlignment(Qt::AlignTop | Qt::AlignRight);
}

void ToastNotification::paintEvent(QPaintEvent*) {
    // Прозрачный фон
}

void ToastNotification::showMessage(const QString& text, const QString& colorHex, int durationMs) {
    auto* label = new QLabel(text, this);
    label->setFont(QFont("Courier New", 10, QFont::Bold));
    label->setStyleSheet(QString(
        "background-color: rgba(10, 10, 15, 220);"
        "color: %1;"
        "border: 2px solid %1;"
        "padding: 8px 12px;"
    ).arg(colorHex));
    label->setAlignment(Qt::AlignCenter);
    label->show();

    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(durationMs);

    auto* item = new ToastItem{label, timer};
    m_items.push_back(item);
    m_layout->addWidget(label);

    connect(timer, &QTimer::timeout, this, [this, item]() {
        removeItem(item);
    });
    timer->start();

    // Ограничим количество одновременных тостов
    while (m_items.size() > 5) {
        removeItem(m_items.front());
    }
}

void ToastNotification::removeItem(ToastItem* item) {
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it == m_items.end()) return;

    m_items.erase(it);
    item->label->deleteLater();
    item->timer->deleteLater();
    delete item;
}
