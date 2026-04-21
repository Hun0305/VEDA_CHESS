#include "gameview.h"
#include "loginwindow.h"
#include <QApplication>

GameView *game; // 전역 변수 유지

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    LoginWindow login;
    if (login.exec() == QDialog::Accepted) {
        game = new GameView();
        game->displayMainMenu(); // 기존 mdziubich 소스의 메뉴 호출
        game->show();
        return a.exec();
    }

    return 0;
}