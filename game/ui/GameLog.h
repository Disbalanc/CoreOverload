#pragma once
// ============================================================
// GameLog — Глобальный лог забега (singleton).
// Переживает все экраны. Логи не теряются между боями.
// ============================================================

#include <QString>
#include <QObject>
#include <QDateTime>
#include <vector>

class GameLog : public QObject {
    Q_OBJECT

public:
    struct Entry {
        QString time;
        QString text;
        QString color;
    };

    static GameLog& instance();

    void addMessage(const QString& text, const QString& color = "#C0C0C0");
    void clear();

    const std::vector<Entry>& getEntries() const { return m_entries; }

signals:
    void messageAdded(const Entry& entry);

private:
    GameLog() = default;
    std::vector<Entry> m_entries;
};
