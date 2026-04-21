#ifndef RANKINGDIALOG_H
#define RANKINGDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class RankingDialog : public QDialog {
    Q_OBJECT
public:
    explicit RankingDialog(QWidget *parent = nullptr);

private slots:
    void loadRankingData();      // DB에서 데이터를 가져와서 표를 채우는 함수
    void handleSearch();         // 검색 버튼 클릭 시 실행

private:
    QTableWidget *rankingTable;
    QLineEdit *searchEdit;
};

#endif