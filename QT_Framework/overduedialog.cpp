#include "overduedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>

OverdueDialog::OverdueDialog(Administrator &admin, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Overdue Report");
    setMinimumSize(700, 420);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(16, 16, 16, 16);

    // ── Title label ──────────────────────────────────────────
    QLabel *title = new QLabel("📋  Overdue Books Report", this);
    title->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #7030a0; padding: 4px;");
    layout->addWidget(title);

    // ── Summary label (filled after populate) ────────────────
    summaryLabel = new QLabel(this);
    summaryLabel->setStyleSheet("color: #a04060; font-weight: bold; font-size: 13px;");
    layout->addWidget(summaryLabel);

    // ── Table ────────────────────────────────────────────────
    table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({
        "User ID", "Name", "Book ID", "Issue Date", "Due Date", "Days Overdue", "Fine ($)"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    layout->addWidget(table);

    // ── Close button ─────────────────────────────────────────
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton *closeBtn = new QPushButton("Close", this);
    closeBtn->setFixedWidth(100);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // ── Fill table ───────────────────────────────────────────
    populate(admin);

    Logger::log("Admin viewed overdue report.", Logger::ACTION);
}

void OverdueDialog::populate(Administrator &admin) {
    table->setRowCount(0);
    int overdueCount = 0;
    double totalFines = 0.0;

    for (auto &u : admin.getUsers()) {
        // Get all borrow records for this user via borrowHistory
        // We access them through the user's public view methods indirectly —
        // instead we iterate resources and check active borrows
        string fullName = u->getFirstName() + " " + u->getLastName();

        // We need to check each user's borrow records.
        // Since borrowHistory is private, we use hasOverdueRecords()
        // and viewBorrowHistory() redirects to console.
        // So we expose records via a new getter — see note below.
        // For now we use the public canBorrow/hasOverdue checks +
        // iterate via the public getOverdueRecords() helper we add.

        for (auto &rec : u->getOverdueRecords()) {
            int daysOverdue = (int)(Date::today().toDays() - rec->getDueDate().toDays());
            double fine     = rec->calculateFine(Date::today());
            totalFines     += fine;

            int row = table->rowCount();
            table->insertRow(row);

            // Color the whole row red-ish
            auto makeItem = [](const QString &text) {
                QTableWidgetItem *item = new QTableWidgetItem(text);
                item->setForeground(QColor("#8b0000"));
                return item;
            };

            table->setItem(row, 0, makeItem(QString::number(u->getUserID())));
            table->setItem(row, 1, makeItem(QString::fromStdString(fullName)));
            table->setItem(row, 2, makeItem(QString::number(rec->getResourceID())));
            table->setItem(row, 3, makeItem(QString::fromStdString(rec->getIssueDate().toString())));
            table->setItem(row, 4, makeItem(QString::fromStdString(rec->getDueDate().toString())));
            table->setItem(row, 5, makeItem(QString::number(daysOverdue) + " days"));
            table->setItem(row, 6, makeItem("$" + QString::number((int)fine)));

            overdueCount++;
        }
    }

    if (overdueCount == 0) {
        summaryLabel->setText("✅  No overdue records — all clear!");
        summaryLabel->setStyleSheet("color: #207040; font-weight: bold; font-size: 13px;");
    } else {
        summaryLabel->setText(
            QString("⚠️  %1 overdue record(s) found   |   Total fines: $%2")
                .arg(overdueCount)
                .arg((int)totalFines));
    }
}
