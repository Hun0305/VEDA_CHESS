#ifndef DELETEACCOUNTDIALOG_H
#define DELETEACCOUNTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class DeleteAccountDialog : public QDialog {
    Q_OBJECT
public:
    explicit DeleteAccountDialog(QWidget *parent = nullptr);

private slots:
    void handleDeleteAccount(); // 탈퇴 버튼 클릭 시 실행될 함수

private:
    QLineEdit *idInput;
    QLineEdit *pwInput;
    QLabel *statusLabel;
};

#endif // DELETEACCOUNTDIALOG_H