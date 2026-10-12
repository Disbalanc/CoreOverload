// ============================================================
// SoundManager.h — Управление аудио-эффектами игры
// ============================================================

#include <QString>
#include <QHash>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#include <QMediaPlayer>
#else
#include <QSoundEffect>
#endif

class SoundManager {
public:
    static SoundManager& instance();

    // Проиграть короткий звуковой эффект
    // ID звуков: "click", "hit", "shield", "victory", "defeat"
    void playSound(const QString& soundId);

    // Запустить фоновую музыку (loop)
    void playMusic(const QString& musicPath);
    void stopMusic();

private:
    SoundManager();
    ~SoundManager() = default;

    QString m_audioPath = "assets/audio/";

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMediaPlayer* m_musicPlayer = nullptr;
    QAudioOutput* m_audioOutput = nullptr;
    QHash<QString, QMediaPlayer*> m_sounds;
#else
    QHash<QString, QSoundEffect*> m_sounds;
    QSoundEffect* m_music = nullptr;
#endif
};
