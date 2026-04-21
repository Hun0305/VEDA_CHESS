#include "changepassworddialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("비밀번호 변경");
    setFixedSize(350, 450);

    // 부모 창(LoginWindow)의 스타일을 가져와 통일감 부여
    if (parent) this->setStyleSheet(parent->styleSheet());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    idInput = new QLineEdit(this); idInput->setPlaceholderText("아이디 입력");
    currentPwInput = new QLineEdit(this); currentPwInput->setPlaceholderText("현재 비밀번호");
    currentPwInput->setEchoMode(QLineEdit::Password);

    newPwInput = new QLineEdit(this); newPwInput->setPlaceholderText("새 비밀번호");
    newPwInput->setEchoMode(QLineEdit::Password);

    confirmPwInput = new QLineEdit(this); confirmPwInput->setPlaceholderText("새 비밀번호 확인");
    confirmPwInput->setEchoMode(QLineEdit::Password);

    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;"); // 에러 메시지는 강조

    QPushButton *changeBtn = new QPushButton("비밀번호 수정하기", this);
    changeBtn->setMinimumHeight(40);
    connect(changeBtn, &QPushButton::clicked, this, &ChangePasswordDialog::handleChangePassword);

    layout->addWidget(new QLabel("계정 정보를 확인 후 변경해 주세요."));
    layout->addWidget(idInput);
    layout->addWidget(currentPwInput);
    layout->addWidget(new QLabel("새로운 비밀번호를 입력해 주세요."));
    layout->addWidget(newPwInput);
    layout->addWidget(confirmPwInput);
    layout->addWidget(statusLabel);
    layout->addStretch();
    layout->addWidget(changeBtn);
}

void ChangePasswordDialog::handleChangePassword() {
    QString id = idInput->text().trimmed();
    QString oldPw = currentPwInput->text();
    QString newPw = newPwInput->text();
    QString confirmPw = confirmPwInput->text();

    if (id.isEmpty() || oldPw.isEmpty() || newPw.isEmpty()) {
        statusLabel->setText("모든 항목을 입력해 주세요.");
        return;
    }

    if (newPw != confirmPw) {
        statusLabel->setText("새 비밀번호 확인이 일치하지 않습니다.");
        return;
    }

    QSqlQuery query;
    // 1. 기존 정보 확인 (검색/조회)
    query.prepare("SELECT * FROM users WHERE id = :id AND password = :oldPw");
    query.bindValue(":id", id);
    query.bindValue(":oldPw", oldPw);
    query.exec();

    if (query.next()) {
        // 2. 일치하면 비밀번호 업데이트 (데이터 수정 - UPDATE)
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET password = :newPw WHERE id = :id");
        updateQuery.bindValue(":newPw", newPw);
        updateQuery.bindValue(":id", id);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "성공", "비밀번호가 성공적으로 변경되었습니다.");
            accept();
        } else {
            statusLabel->setText("DB 수정 오류가 발생했습니다.");
        }
    } else {
        statusLabel->setText("아이디 또는 현재 비밀번호가 틀립니다.");
    }
}