#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>

#include "ui/GameController.h"
#include "ui/AssetManager.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    AssetManager::instance().setBasePath("assets/sprites/");

    QMainWindow window;
    window.setWindowTitle("Core Overload: Кремниевый Бунт");
    window.setMinimumSize(1024, 680);
    window.resize(1200, 780);
    window.setStyleSheet("QMainWindow { background-color: #0A0A0F; }");

    auto* stack = new QStackedWidget(&window);
    stack->setStyleSheet("background-color: #0A0A0F;");
    window.setCentralWidget(stack);

    GameController controller(stack);
    controller.showTitleScreen();

    window.show();
    return app.exec();
}
