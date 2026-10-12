#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>
#include <functional>
#include <engine/entities/Player.h>
#include <engine/core/Random.h>

class EventScreen : public QWidget {
    Q_OBJECT

public:
    explicit EventScreen(engine::Player& player, engine::Random& rng, QWidget* parent = nullptr);

signals:
    void eventFinished();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct EventOption {
        QString text;
        std::function<QString()> action; // Возвращает текст результата
    };

    struct EventData {
        QString title;
        QString description;
        std::vector<EventOption> options;
    };

    void setupEvent(const EventData& data);
    void onOptionSelected(int index);

    engine::Player& m_player;
    engine::Random& m_rng;

    EventData m_currentEvent;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_descLabel = nullptr;
    QLabel* m_resultLabel = nullptr;
    QVBoxLayout* m_optionsLayout = nullptr;
    QPushButton* m_continueBtn = nullptr;
};
