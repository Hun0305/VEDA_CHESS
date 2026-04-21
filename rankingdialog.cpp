#include "rankingdialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>

RankingDialog::RankingDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("체스 월드 랭킹");
    setFixedSize(500, 600);
    if (parent) this->setStyleSheet(parent->styleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- 상단 검색바 영역 ---
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("검색할 아이디 입력...");
    QPushButton *searchBtn = new QPushButton("검색", this);
    connect(searchBtn, &QPushButton::clicked, this, &RankingDialog::handleSearch);

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchLayout);

    // --- 중앙 랭킹 테이블 (자동 스크롤 포함) ---
    rankingTable = new QTableWidget(this);
    rankingTable->setColumnCount(4);
    rankingTable->setHorizontalHeaderLabels({"순위", "아이디", "전적", "승률"});
    rankingTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 수정 불가
    rankingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 꽉 채우기

    mainLayout->addWidget(rankingTable);

    loadRankingData(); // 창이 열릴 때 데이터 로드
}

void RankingDialog::loadRankingData() {
    rankingTable->setRowCount(0);

    QSqlQuery query;
    // 승률순 정렬: (승리/전체판수) 기준 내림차순. 전적이 없으면 0으로 처리.
    QString sql = "SELECT id, wins, losses, (wins + losses) as total "
                  "FROM users "
                  "ORDER BY CASE WHEN (wins + losses) = 0 THEN 0 "
                  "ELSE (CAST(wins AS FLOAT) / (wins + losses)) END DESC, id ASC";

    if (query.exec(sql)) {
        int rank = 1;
        while (query.next()) {
            int row = rankingTable->rowCount();
            rankingTable->insertRow(row);

            QString id = query.value("id").toString();
            int wins = query.value("wins").toInt();
            int losses = query.value("losses").toInt();
            int total = query.value("total").toInt();

            // 순위 및 승률 계산
            QString rankStr = (total == 0) ? "-" : QString::number(rank++);
            QString recordStr = QString("%1승 %2패").arg(wins).arg(losses);
            QString winRateStr = (total == 0) ? "-" : QString::number((wins * 100) / total) + "%";

            rankingTable->setItem(row, 0, new QTableWidgetItem(rankStr));
            rankingTable->setItem(row, 1, new QTableWidgetItem(id));
            rankingTable->setItem(row, 2, new QTableWidgetItem(recordStr));
            rankingTable->setItem(row, 3, new QTableWidgetItem(winRateStr));
        }
    }
}

void RankingDialog::handleSearch() {
    QString targetId = searchEdit->text().trimmed();
    if (targetId.isEmpty()) return;

    bool found = false;
    for (int i = 0; i < rankingTable->rowCount(); ++i) {
        if (rankingTable->item(i, 1)->text() == targetId) {
            QString rank = rankingTable->item(i, 0)->text();
            QString record = rankingTable->item(i, 2)->text();
            QString winRate = rankingTable->item(i, 3)->text();

            QString info = QString("순위: %1\n아이디: %2\n전적: %3\n승률: %4")
                               .arg(rank).arg(targetId).arg(record).arg(winRate);

            QMessageBox::information(this, "검색 결과", info);
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::warning(this, "결과 없음", "해당 아이디를 찾을 수 없습니다.");
    }
}