#pragma once

#include <QDialog>
#include <QSlider>
#include <QCheckBox>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Глобальные настройки
    static int getMusicVolume();
    static int getSfxVolume();
    static bool getScanlinesEnabled();

private slots:
    void onApply();

private:
    QSlider* m_musicSlider = nullptr;
    QSlider* m_sfxSlider = nullptr;
    QCheckBox* m_scanlinesCheck = nullptr;

    static int s_musicVolume;
    static int s_sfxVolume;
    static bool s_scanlinesEnabled;
};
