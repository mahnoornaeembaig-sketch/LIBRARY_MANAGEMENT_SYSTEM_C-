#include "userstabledialog.h"
#include <QVBoxLayout>
#include <QHeaderView>

UsersTableDialog::UsersTableDialog(Administrator& admin, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("All Registered Users");
    setMinimumSize(800, 400);

    userTable = new QTableWidget(this);
    userTable->setColumnCount(8);
    userTable->setHorizontalHeaderLabels(
        {"ID", "First Name", "Last Name", "Username",
         "Email", "Membership", "Borrows", "Balance"});
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setAlternatingRowColors(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(userTable);

    populate(admin);
}

void UsersTableDialog::populate(Administrator& admin) {
    userTable->setRowCount(0);
    for (auto& u : admin.getUsers()) {
        int row = userTable->rowCount();
        userTable->insertRow(row);

        userTable->setItem(row, 0, new QTableWidgetItem(
                                       QString::number(u->getUserID())));
        userTable->setItem(row, 1, new QTableWidgetItem(
                                       QString::fromStdString(u->getFirstName())));
        userTable->setItem(row, 2, new QTableWidgetItem(
                                       QString::fromStdString(u->getLastName())));
        userTable->setItem(row, 3, new QTableWidgetItem(
                                       QString::fromStdString(u->getUsername())));
        userTable->setItem(row, 4, new QTableWidgetItem(
                                       QString::fromStdString(u->getEmail())));
        userTable->setItem(row, 5, new QTableWidgetItem(
                                       QString::fromStdString(u->getMembership()->getLevel())));

        QString borrows = QString::number(u->getCurrentBorrows()) + "/" +
                          QString::number(u->getMembership()->getMaxBooks());
        userTable->setItem(row, 6, new QTableWidgetItem(borrows));

        QString balance = "$" + QString::number((int)u->getAccount()->getBalance());
        QTableWidgetItem* balItem = new QTableWidgetItem(balance);
        double fine = u->getAccount()->getFinesDue();
        if (fine > 0) {
            balItem->setForeground(Qt::red);
            balItem->setText(balance + "  (Fine: $" + QString::number((int)fine) + ")");
        } else {
            balItem->setForeground(Qt::darkGreen);
        }
        userTable->setItem(row, 7, balItem);
    }
}
