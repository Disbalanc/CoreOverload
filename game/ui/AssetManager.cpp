#include "AssetManager.h"
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

void AssetManager::setBasePath(const QString& path) {
    m_basePath = path;
    if (!m_basePath.endsWith('/')) m_basePath += '/';
}

const QPixmap* AssetManager::getSprite(const QString& id) {
    // 1. Ищем в кэше успешных загрузок
    auto it = m_cache.find(id);
    if (it != m_cache.end()) {
        return &it.value(); // Возвращаем адрес QPixmap, хранящегося в хеш-таблице
    }

    // 2. Ищем в кэше ошибок (чтобы не обращаться к диску повторно)
    if (m_notFoundCache.contains(id)) {
        return nullptr;
    }

    // 3. Формируем пути для поиска файла
    QStringList candidatePaths = {
        m_basePath + id + ".png",
        QCoreApplication::applicationDirPath() + "/" + m_basePath + id + ".png",
        QCoreApplication::applicationDirPath() + "/../" + m_basePath + id + ".png",
    };

    for (const QString& path : candidatePaths) {
        if (QFile::exists(path)) {
            QPixmap pixmap(path);
            if (!pixmap.isNull()) {
                // Добавляем в кэш и возвращаем указатель на сохранённый объект
                auto insertedIt = m_cache.insert(id, pixmap);
                qDebug() << "AssetManager: УСПЕШНО загружен спрайт:" << id << "из" << path;
                return &insertedIt.value();
            }
        }
    }

    // 4. Если файл не найден — запоминаем это
    m_notFoundCache.insert(id, true);
    return nullptr;
}

bool AssetManager::hasSprite(const QString& id) const {
    return m_cache.contains(id);
}

void AssetManager::clearCache() {
    m_cache.clear();
    m_notFoundCache.clear();
}
