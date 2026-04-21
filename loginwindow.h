#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui { class LoginWindow; }

class LoginWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    QString getLoggedInId() const;

private slots:
    void on_loginButton_clicked();
    void on_toRegisterButton_clicked(); // 페이지 전환 (로그인 -> 회원가입)
    void on_backButton_clicked();       // 페이지 전환 (회원가입 -> 로그인)
    void on_registerButton_clicked();   // 회원가입 실행

private:
    Ui::LoginWindow *ui;
    QSqlDatabase db;
    QString loggedInId;
    void initDatabase();
    void applyStyles(); // 다크 테마 디자인 적용
};

#endif