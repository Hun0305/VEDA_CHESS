#include "gameview.h"
#include "loginwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // main.cpp의 styleSheet 부분을 아래 코드로 완전히 교체하세요.
    QString styleSheet =
        // 수정할 코드 (폰트 목록에 궁서, 바탕, 명조체 추가)
        "QWidget { background-color: #2D1B14; color: #F5F5DC; font-family: 'Georgia', 'Batang', '궁서', 'Nanum Myeongjo', serif; font-size: 15px; }"
        "QPushButton { "
        "   background-color: #5D4037; border: 2px solid #D4AF37; border-radius: 2px; "
        "   font-weight: bold; padding: 10px 15px; color: #D4AF37; "
        "}"
        "QPushButton:hover { background-color: #8D6E63; color: #FFFFFF; }"
        "QPushButton:pressed { background-color: #3E2723; }"
        "QLineEdit, QComboBox { "
        "   background-color: #F5F5DC; border: 1px solid #A1887F; "
        "   border-radius: 0px; padding: 8px; color: #3E2723; font-style: italic; "
        "   min-height: 30px; "
        "}"
        "QLineEdit:focus { border: 2px solid #D4AF37; }"
        "QTableWidget { "
        "   background-color: #F5F5DC; color: #3E2723; "
        "   gridline-color: #D4AF37; border: 2px solid #D4AF37; "
        "}"
        "QHeaderView::section { "
        "   background-color: #D4AF37; color: #2D1B14; font-weight: bold; "
        "   padding: 5px; border: 1px solid #2D1B14; "
        "}"
        "QLabel { background: transparent; color: #F5F5DC; }";
    a.setStyleSheet(styleSheet);

    LoginWindow login;
    if (login.exec() == QDialog::Accepted) {
        QString userId = login.getLoggedInId();
        GameView *gameView = new GameView();
        gameView->setLoggedInUser(userId);
        gameView->displayMainMenu();
        gameView->show();
        return a.exec();
    }
    return 0;
}