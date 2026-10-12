#pragma once
// ============================================================
// CombatLogWidget.h — Журнал игровых и боевых событий (Консоль)
// ============================================================

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <engine/core/EventBus.h>
#include <engine/core/Types.h>
#include <vector>

class CombatLogWidget : public QWidget {
    Q_OBJECT

public:
    // Передаём указатель на шину событий. Если nullptr — просто отображаем глобальный лог
    explicit CombatLogWidget(engine::EventBus* eventBus, QWidget* parent = nullptr);
    ~CombatLogWidget() override;

    // Обновить текстовое поле на основе всех записей из GameLog
    void refreshFromGameLog();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void subscribeToEvents();
    void appendEntry(const QString& time, const QString& text, const QString& color);

    engine::EventBus* m_eventBus;
    QTextEdit* m_logText = nullptr;
    std::vector<engine::SubscriptionId> m_subs;
};
