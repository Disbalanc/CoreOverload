#include "GameLog.h"

GameLog& GameLog::instance() {
    static GameLog inst;
    return inst;
}

void GameLog::addMessage(const QString& text, const QString& color) {
    Entry e;
    e.time = QDateTime::currentDateTime().toString("hh:mm:ss");
    e.text = text;
    e.color = color;
    m_entries.push_back(e);

    // Ограничим размер лога, чтобы не разрастался до бесконечности
    if (m_entries.size() > 500) {
        m_entries.erase(m_entries.begin(), m_entries.begin() + 100);
    }

    emit messageAdded(e);
}

void GameLog::clear() {
    m_entries.clear();
}
