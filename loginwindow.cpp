#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QMessageBox>
#include <QSqlError>

LoginWindow::LoginWindow(QWidget *parent) : QDialog(parent), ui(new Ui::LoginWindow) {
    ui->setupUi(this);
    initDatabase();
    applyStyles();
    ui->stackedWidget->setCurrentIndex(0); // 초기 화면: 로그인
}

LoginWindow::~LoginWindow() { delete ui; }

void LoginWindow::applyStyles() {
    this->setStyleSheet("QDialog { background-color: #2C2833; } QLabel { color: white; }");
    QString inputStyle = "QLineEdit { padding: 10px; border-radius: 5px; background: #3D3946; color: white; border: 1px solid #555; }";
    QString btnStyle = "QPushButton { padding: 12px; background-color: #5C5470; color: white; border-radius: 5px; font-weight: bold; } "
                       "QPushButton:hover { background-color: #352F44; }";

    ui->idLineEdit->setStyleSheet(inputStyle); ui->pwLineEdit->setStyleSheet(inputStyle);
    ui->regIdLineEdit->setStyleSheet(inputStyle); ui->regPwLineEdit->setStyleSheet(inputStyle);
    ui->loginButton->setStyleSheet(btnStyle); ui->registerButton->setStyleSheet(btnStyle);
    ui->toRegisterButton->setStyleSheet("color: #DBD8E3; border: none; background: none; text-decoration: underline;");
    ui->backButton->setStyleSheet("color: #DBD8E3; border: none; background: none; text-decoration: underline;");
}

void LoginWindow::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("chess_players.db");
    if (!db.open()) return;

    QSqlQuery query;
    // password 컬럼이 포함된 테이블 생성
    query.exec("CREATE TABLE IF NOT EXISTS users (id TEXT PRIMARY KEY, password TEXT, wins INTEGER, losses INTEGER)");
}

void LoginWindow::on_toRegisterButton_clicked() { ui->stackedWidget->setCurrentIndex(1); }
void LoginWindow::on_backButton_clicked() { ui->stackedWidget->setCurrentIndex(0); }

void LoginWindow::on_loginButton_clicked() {
    QString id = ui->idLineEdit->text();
    QString pw = ui->pwLineEdit->text();

    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE id = :id AND password = :pw");
    query.bindValue(":id", id);
    query.bindValue(":pw", pw);
    query.exec();

    if (query.next()) {
        loggedInId = id;
        accept();
    } else {
        ui->statusLabel->setStyleSheet("color: #E94560;");
        ui->statusLabel->setText("ID 또는 비밀번호가 틀렸습니다.");
    }
}

void LoginWindow::on_registerButton_clicked() {
    QString id = ui->regIdLineEdit->text();
    QString pw = ui->regPwLineEdit->text();

    if (id.isEmpty() || pw.isEmpty()) {
        ui->regStatusLabel->setText("모든 정보를 입력해주세요.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO users (id, password, wins, losses) VALUES (:id, :pw, 0, 0)");
    query.bindValue(":id", id);
    query.bindValue(":pw", pw);

    if (query.exec()) {
        QMessageBox::information(this, "성공", "회원가입 완료! 로그인 해주세요.");
        ui->stackedWidget->setCurrentIndex(0);
    } else {
        ui->regStatusLabel->setStyleSheet("color: #E94560;");
        ui->regStatusLabel->setText("이미 존재하는 ID입니다.");
    }
}

QString LoginWindow::getLoggedInId() const { return loggedInId; }