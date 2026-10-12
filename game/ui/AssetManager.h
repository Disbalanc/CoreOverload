#pragma once
// ============================================================
// AssetManager.h — Система управления спрайтами (Исправленная версия)
// ============================================================

#include <QPixmap>
#include <QString>
#include <QHash>

class AssetManager {
public:
    static AssetManager& instance();

    // Получить спрайт по ID. Возвращает nullptr, если файл не найден.
    const QPixmap* getSprite(const QString& id);

    // Проверить, есть ли спрайт
    bool hasSprite(const QString& id) const;

    // Задать базовый путь
    void setBasePath(const QString& path);

    // Очистить кэш
    void clearCache();

private:
    AssetManager() = default;

    QString m_basePath = "assets/sprites/";

    // Храним QPixmap напрямую. Копирование QPixmap в Qt очень дешёвое (ref-counted)
    QHash<QString, QPixmap> m_cache;
    QHash<QString, bool> m_notFoundCache;
};
