#include "TitleScreen.h"
#include <QPainter>
#include <QApplication>

TitleScreen::TitleScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    // Пустое пространство сверху
    layout->addStretch(2);

    // Заголовок
    auto* title = new QLabel("CORE OVERLOAD");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Courier New", 42, QFont::Bold));
    title->setStyleSheet("color: #E94560; background: transparent;");
    layout->addWidget(title);

    // Подзаголовок
    auto* subtitle = new QLabel("КРЕМНИЕВЫЙ БУНТ");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(QFont("Courier New", 16, QFont::Bold));
    subtitle->setStyleSheet("color: #00FF41; background: transparent;");
    layout->addWidget(subtitle);

    // Версия
    auto* version = new QLabel("v0.1.0 // deck.builder" );
    version->setAlignment(Qt::AlignCenter);
    version->setFont(QFont("Courier New", 9));
    version->setStyleSheet("color: #808080; background: transparent;");
    layout->addWidget(version);

    layout->addStretch(1);

    // Стиль кнопок
    QString btnStyle =
        "QPushButton {"
        "  background-color: #16213E;"
        "  color: #00FF41;"
        "  border: 2px solid #00FF41;"
        "  padding: 12px;"
        "  font-family: 'Courier New';"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 350px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #00FF41;"
        "  color: #0A0A0F;"
        "}";

    // Кнопка новой игры
    auto* newGameBtn = new QPushButton("[ ИНИЦИАЛИЗИРОВАТЬ НОВЫЙ ЗАБЕГ ]");
    newGameBtn->setStyleSheet(btnStyle);
    connect(newGameBtn, &QPushButton::clicked, this, &TitleScreen::startNewGame);
    layout->addWidget(newGameBtn, 0, Qt::AlignCenter);

    // Кнопка настроек
    auto* settingsBtn = new QPushButton("[ ПАРАМЕТРЫ СИСТЕМЫ ]");
    QString settingsBtnStyle = btnStyle;
    settingsBtnStyle.replace("#00FF41", "#00D9FF");
    settingsBtn->setStyleSheet(settingsBtnStyle);
    connect(settingsBtn, &QPushButton::clicked, this, &TitleScreen::openSettings);
    layout->addWidget(settingsBtn, 0, Qt::AlignCenter);

    // Кнопка выхода
    auto* quitBtn = new QPushButton("[ ЗАВЕРШИТЬ ПРОЦЕСС ]");
    QString quitBtnStyle = btnStyle;
    quitBtnStyle.replace("#00FF41", "#E94560");
    quitBtn->setStyleSheet(quitBtnStyle);
    connect(quitBtn, &QPushButton::clicked, this, &TitleScreen::quitGame);
    layout->addWidget(quitBtn, 0, Qt::AlignCenter);

    layout->addStretch(2);

    // Авторские права внизу
    auto* credits = new QLabel("// Powered by Core Engine // C++20 + Qt //");
    credits->setAlignment(Qt::AlignCenter);
    credits->setFont(QFont("Courier New", 8));
    credits->setStyleSheet("color: #555555; background: transparent;");
    layout->addWidget(credits);
}

void TitleScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));

    // Декоративные линии сканирования (scanlines)
    p.setPen(QPen(QColor(0, 255, 65, 8)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }

    // Декоративная рамка
    p.setPen(QPen(QColor("#16213E"), 2));
    p.drawRect(rect().adjusted(10, 10, -10, -10));
}
