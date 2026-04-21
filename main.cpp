#include "gameview.h"
#include "loginwindow.h"
#include <QApplication>

GameView *game; // 전역 변수 유지

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    LoginWindow login;
    // 로그인 창을 띄우고 사용자가 로그인 버튼을 눌러 accept() 될 때까지 대기
    if (login.exec() == QDialog::Accepted) {
        QString userId = login.getLoggedInId(); // 로그인한 아이디 가져오기

        GameView *gameView = new GameView();
        gameView->setLoggedInUser(userId); // GameView에 아이디 전달
        gameView->displayMainMenu();       // 메인 메뉴 표시
        gameView->show();

        return a.exec();
    }

    return 0; // 로그인 취소 시 종료
}