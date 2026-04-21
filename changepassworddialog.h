#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class ChangePasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangePasswordDialog(QWidget *parent = nullptr);

private slots:
    void handleChangePassword(); // 변경 버튼 클릭 시 실행될 함수

private:
    QLineEdit *idInput;
    QLineEdit *currentPwInput;
    QLineEdit *newPwInput;
    QLineEdit *confirmPwInput;
    QLabel *statusLabel;
};

#endif // CHANGEPASSWORDDIALOG_H