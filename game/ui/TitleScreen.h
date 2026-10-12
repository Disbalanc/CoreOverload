#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class TitleScreen : public QWidget {
    Q_OBJECT

public:
    explicit TitleScreen(QWidget* parent = nullptr);

signals:
    void startNewGame();
    void openSettings();
    void quitGame();

protected:
    void paintEvent(QPaintEvent* event) override;
};
