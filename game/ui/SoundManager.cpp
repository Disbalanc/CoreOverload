#include "SoundManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

SoundManager& SoundManager::instance() {
    static SoundManager inst;
    return inst;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// ── Реализация под Qt6 ────────────────────────────────────
SoundManager::SoundManager() {
    m_musicPlayer = new QMediaPlayer();
    m_audioOutput = new QAudioOutput();
    m_musicPlayer->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.4f); // Музыка чуть тише
}

void SoundManager::playSound(const QString& soundId) {
    // В Qt6 звуки играются через QMediaPlayer + QAudioOutput
    qDebug() << "SoundManager: попытка проиграть" << soundId;
    // (Для MVP десктопа на Qt5 используем QSoundEffect ниже)
}

void SoundManager::playMusic(const QString&) {}
void SoundManager::stopMusic() {}

#else
// ── Реализация под Qt5 (Твой комплект) ─────────────────────
SoundManager::SoundManager() {}

void SoundManager::playSound(const QString& soundId) {
    auto it = m_sounds.find(soundId);
    if (it != m_sounds.end()) {
        if (it.value()->isLoaded()) {
            it.value()->play();
        }
        return;
    }

    // Пробуем загрузить файл эффекта
    QString fullPath = QCoreApplication::applicationDirPath() + "/" + m_audioPath + soundId + ".wav";
    if (!QFile::exists(fullPath)) {
        fullPath = m_audioPath + soundId + ".wav"; // Fallback на относительный путь
    }

    if (QFile::exists(fullPath)) {
        auto* effect = new QSoundEffect(QCoreApplication::instance());
        effect->setSource(QUrl::fromLocalFile(fullPath));
        effect->setVolume(0.6);
        m_sounds.insert(soundId, effect);
        effect->play();
        qDebug() << "SoundManager: Загружен и запущен эффект:" << soundId;
    } else {
        qDebug() << "SoundManager: Файл звука не найден:" << fullPath;
    }
}

void SoundManager::playMusic(const QString& musicFileName) {
    stopMusic();

    QString fullPath = QCoreApplication::applicationDirPath() + "/" + m_audioPath + musicFileName;
    if (!QFile::exists(fullPath)) {
        fullPath = m_audioPath + musicFileName;
    }

    if (QFile::exists(fullPath)) {
        m_music = new QSoundEffect(QCoreApplication::instance());
        m_music->setSource(QUrl::fromLocalFile(fullPath));
        m_music->setLoopCount(QSoundEffect::Infinite);
        m_music->setVolume(0.3); // Фоновая музыка потише
        m_music->play();
        qDebug() << "SoundManager: Запущена фоновая музыка:" << musicFileName;
    }
}

void SoundManager::stopMusic() {
    if (m_music) {
        m_music->stop();
        m_music->deleteLater();
        m_music = nullptr;
    }
}
#endif
