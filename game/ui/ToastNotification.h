#pragma once
// ============================================================
// ToastNotification — всплывающее уведомление в углу боя
// ============================================================

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <vector>

class ToastNotification : public QWidget {
    Q_OBJECT

public:
    explicit ToastNotification(QWidget* parent = nullptr);

    void showMessage(const QString& text, const QString& colorHex = "#00FF41", int durationMs = 2500);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct ToastItem {
        QLabel* label;
        QTimer* timer;
    };

    void repositionItems();
    void removeItem(ToastItem* item);

    QVBoxLayout* m_layout;
    std::vector<ToastItem*> m_items;
};
