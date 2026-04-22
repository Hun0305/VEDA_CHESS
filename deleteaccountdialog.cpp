#include "deleteaccountdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

DeleteAccountDialog::DeleteAccountDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("회원 탈퇴");
    setFixedSize(380, 380); // 비밀번호 창보다 입력칸이 적어 높이를 줄였습니다.

    if (parent) this->setStyleSheet(parent->styleSheet());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    idInput = new QLineEdit(this); idInput->setPlaceholderText("아이디 입력");
    pwInput = new QLineEdit(this); pwInput->setPlaceholderText("비밀번호 입력");
    pwInput->setEchoMode(QLineEdit::Password);

    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");

    QPushButton *deleteBtn = new QPushButton("탈퇴하기", this);
    deleteBtn->setMinimumHeight(40);
    // 탈퇴는 위험한 작업이므로 버튼 색상을 붉은 계열로 강조 (선택 사항)
    deleteBtn->setStyleSheet("background-color: #d63031; color: white; border-radius: 5px;");
    connect(deleteBtn, &QPushButton::clicked, this, &DeleteAccountDialog::handleDeleteAccount);

    layout->addWidget(new QLabel("탈퇴할 계정 정보를 입력해 주세요."));
    layout->addWidget(idInput);
    layout->addWidget(pwInput);
    layout->addWidget(statusLabel);
    layout->addStretch();
    layout->addWidget(deleteBtn);
}

void DeleteAccountDialog::handleDeleteAccount() {
    QString id = idInput->text().trimmed();
    QString pw = pwInput->text();

    if (id.isEmpty() || pw.isEmpty()) {
        statusLabel->setText("모든 항목을 입력해 주세요.");
        return;
    }

    QSqlQuery query;
    // 1. 기존 정보 확인 (입력한 계정이 실제로 존재하는지)
    query.prepare("SELECT * FROM users WHERE id = :id AND password = :pw");
    query.bindValue(":id", id);
    query.bindValue(":pw", pw);
    query.exec();

    if (query.next()) {
        // 2. 정보가 일치하면 정말 탈퇴할 것인지 팝업창 띄우기
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "회원 탈퇴 확인", "정말로 탈퇴하시겠습니까?\n이 작업은 되돌릴 수 없습니다.",
                                      QMessageBox::Yes | QMessageBox::No);

        // 3. YES를 눌렀을 때만 삭제 진행 (데이터 삭제 - DELETE)
        if (reply == QMessageBox::Yes) {
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM users WHERE id = :id");
            deleteQuery.bindValue(":id", id);

            if (deleteQuery.exec()) {

                // ==========================================
                // 추가된 코드: 파일 내의 삭제 찌꺼기를 완전히 청소
                QSqlQuery vacuumQuery;
                vacuumQuery.exec("VACUUM");
                // ==========================================

                QMessageBox::information(this, "탈퇴 완료", "회원 탈퇴가 정상적으로 처리되었습니다.");
                accept(); // 창 닫기
            } else {
                statusLabel->setText("DB 삭제 오류가 발생했습니다.");
            }
        }
    } else {
        statusLabel->setText("아이디 또는 비밀번호가 일치하지 않습니다.");
    }
}