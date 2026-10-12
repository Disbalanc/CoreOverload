#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

int SettingsDialog::s_musicVolume = 50;
int SettingsDialog::s_sfxVolume = 70;
bool SettingsDialog::s_scanlinesEnabled = true;

int SettingsDialog::getMusicVolume() { return s_musicVolume; }
int SettingsDialog::getSfxVolume() { return s_sfxVolume; }
bool SettingsDialog::getScanlinesEnabled() { return s_scanlinesEnabled; }

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("ПАРАМЕТРЫ СИСТЕМЫ");
    setStyleSheet("QDialog { background-color: #0A0A0F; }");
    resize(500, 400);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(30, 30, 30, 30);
    root->setSpacing(20);

    auto* title = new QLabel("═══ НАСТРОЙКИ ═══");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Courier New", 14, QFont::Bold));
    title->setStyleSheet("color: #00FF41;");
    root->addWidget(title);

    // Громкость музыки
    auto* musicLabel = new QLabel(QString("♪ ГРОМКОСТЬ МУЗЫКИ: %1%").arg(s_musicVolume));
    musicLabel->setFont(QFont("Courier New", 10, QFont::Bold));
    musicLabel->setStyleSheet("color: #00D9FF;");
    root->addWidget(musicLabel);

    m_musicSlider = new QSlider(Qt::Horizontal);
    m_musicSlider->setRange(0, 100);
    m_musicSlider->setValue(s_musicVolume);
    m_musicSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: #16213E; height: 8px; }"
        "QSlider::handle:horizontal { background: #00D9FF; width: 20px; margin: -8px 0; }"
    );
    connect(m_musicSlider, &QSlider::valueChanged, [musicLabel](int v) {
        musicLabel->setText(QString("♪ ГРОМКОСТЬ МУЗЫКИ: %1%").arg(v));
    });
    root->addWidget(m_musicSlider);

    // Громкость SFX
    auto* sfxLabel = new QLabel(QString("♫ ГРОМКОСТЬ ЭФФЕКТОВ: %1%").arg(s_sfxVolume));
    sfxLabel->setFont(QFont("Courier New", 10, QFont::Bold));
    sfxLabel->setStyleSheet("color: #00D9FF;");
    root->addWidget(sfxLabel);

    m_sfxSlider = new QSlider(Qt::Horizontal);
    m_sfxSlider->setRange(0, 100);
    m_sfxSlider->setValue(s_sfxVolume);
    m_sfxSlider->setStyleSheet(m_musicSlider->styleSheet());
    connect(m_sfxSlider, &QSlider::valueChanged, [sfxLabel](int v) {
        sfxLabel->setText(QString("♫ ГРОМКОСТЬ ЭФФЕКТОВ: %1%").arg(v));
    });
    root->addWidget(m_sfxSlider);

    // Сканлайны
    m_scanlinesCheck = new QCheckBox("Эффект сканлайнов (CRT-экран)");
    m_scanlinesCheck->setChecked(s_scanlinesEnabled);
    m_scanlinesCheck->setFont(QFont("Courier New", 10));
    m_scanlinesCheck->setStyleSheet(
        "QCheckBox { color: #FFFFFF; }"
        "QCheckBox::indicator { width: 20px; height: 20px; }"
        "QCheckBox::indicator:unchecked { background-color: #16213E; border: 1px solid #808080; }"
        "QCheckBox::indicator:checked { background-color: #00FF41; border: 1px solid #00FF41; }"
    );
    root->addWidget(m_scanlinesCheck);

    root->addStretch();

    // Кнопки
    auto* btnLayout = new QHBoxLayout();

    auto* applyBtn = new QPushButton("ПРИМЕНИТЬ");
    applyBtn->setFixedSize(180, 40);
    applyBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    applyBtn->setStyleSheet(
        "QPushButton { background-color: #0F3460; color: #00FF41; border: 2px solid #00FF41; }"
        "QPushButton:hover { background-color: #00FF41; color: #0A0A0F; }"
    );
    connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApply);
    btnLayout->addWidget(applyBtn);

    auto* cancelBtn = new QPushButton("ОТМЕНА");
    cancelBtn->setFixedSize(180, 40);
    cancelBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #808080; border: 1px solid #808080; }"
        "QPushButton:hover { background-color: #808080; color: #0A0A0F; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    root->addLayout(btnLayout);
}

void SettingsDialog::onApply() {
    s_musicVolume = m_musicSlider->value();
    s_sfxVolume = m_sfxSlider->value();
    s_scanlinesEnabled = m_scanlinesCheck->isChecked();
    accept();
}
