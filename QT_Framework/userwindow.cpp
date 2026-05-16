#include "userwindow.h"
#include <algorithm>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>

UserWindow::UserWindow(User* user, Administrator& adm, QWidget* parent)
    : QMainWindow(parent), currentUser(user), admin(adm)
{
    setWindowTitle("User Panel — " +
                   QString::fromStdString(user->getFirstName() + " " + user->getLastName()));
    setMinimumSize(1000, 650);
    setupUI();
    refreshBorrowTable();
    refreshAccountInfo();
}

// ─────────────────────────────────────────────────────────────
//  UI CONSTRUCTION
// ─────────────────────────────────────────────────────────────
void UserWindow::setupUI()
{
    QWidget*     central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* root    = new QHBoxLayout(central);

    // ── SIDEBAR ──────────────────────────────────────────────
    QWidget*     sideWidget = new QWidget(this);
    QVBoxLayout* sidebar    = new QVBoxLayout(sideWidget);
    sidebar->setSpacing(6);

    // Account info
    QGroupBox*   infoBox    = new QGroupBox("My Account", this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoBox);
    balanceLabel     = new QLabel("Balance: $0",   this);
    fineLabel        = new QLabel("Fine Due: $0",  this);
    borrowCountLabel = new QLabel("Borrows: 0/2",  this);
    membershipLabel  = new QLabel("Tier: Basic",   this);
    membershipLabel->setStyleSheet("font-weight: bold; color: #7f8c8d;");
    infoLayout->addWidget(membershipLabel);
    infoLayout->addWidget(balanceLabel);
    infoLayout->addWidget(fineLabel);
    infoLayout->addWidget(borrowCountLabel);

    // ── Group: Profile ────────────────────────────────────────
    QGroupBox*   profileBox    = new QGroupBox("Profile", this);
    QVBoxLayout* profileLayout = new QVBoxLayout(profileBox);
    QPushButton* profileBtn    = new QPushButton("My Profile",       this);
    QPushButton* updateBtn     = new QPushButton("Update Profile",   this);
    QPushButton* statementBtn  = new QPushButton("Account Statement",this);
    profileLayout->addWidget(profileBtn);
    profileLayout->addWidget(updateBtn);
    profileLayout->addWidget(statementBtn);

    // ── Group: Books ──────────────────────────────────────────
    QGroupBox*   booksBox    = new QGroupBox("Books", this);
    QVBoxLayout* booksLayout = new QVBoxLayout(booksBox);
    QPushButton* browseBtn   = new QPushButton("Browse Available Books", this);
    QPushButton* searchBtn   = new QPushButton("Search Books",           this);
    QPushButton* borrowBtn   = new QPushButton("Borrow Book",            this);
    QPushButton* returnBtn   = new QPushButton("Return Book",            this);
    QPushButton* holdBtn     = new QPushButton("Place Hold",             this);
    booksLayout->addWidget(browseBtn);
    booksLayout->addWidget(searchBtn);
    booksLayout->addWidget(borrowBtn);
    booksLayout->addWidget(returnBtn);
    booksLayout->addWidget(holdBtn);

    // ── Group: My Records ─────────────────────────────────────
    QGroupBox*   recBox       = new QGroupBox("My Records", this);
    QVBoxLayout* recLayout    = new QVBoxLayout(recBox);
    QPushButton* historyBtn   = new QPushButton("Borrow History",      this);
    QPushButton* currBorrBtn  = new QPushButton("Currently Borrowed",  this);
    QPushButton* holdsBtn     = new QPushButton("My Holds",            this);
    recLayout->addWidget(historyBtn);
    recLayout->addWidget(currBorrBtn);
    recLayout->addWidget(holdsBtn);

    // ── Group: Finance ────────────────────────────────────────
    QGroupBox*   finBox     = new QGroupBox("Finance", this);
    QVBoxLayout* finLayout  = new QVBoxLayout(finBox);
    QPushButton* payBtn     = new QPushButton("Pay Fine",     this);
    QPushButton* depositBtn = new QPushButton("Deposit Balance", this);
    finLayout->addWidget(payBtn);
    finLayout->addWidget(depositBtn);

    // ── Upgrade + Logout ──────────────────────────────────────
    QPushButton* upgradeBtn = new QPushButton("Upgrade Membership", this);
    upgradeBtn->setStyleSheet("background-color:#f39c12; color:white; font-weight:bold; padding:6px;");

    QPushButton* logoutBtn = new QPushButton("Logout", this);
    logoutBtn->setStyleSheet("background-color:#e74c3c; color:white; font-weight:bold; padding:8px;");

    sidebar->addWidget(infoBox);
    sidebar->addWidget(profileBox);
    sidebar->addWidget(booksBox);
    sidebar->addWidget(recBox);
    sidebar->addWidget(finBox);
    sidebar->addWidget(upgradeBtn);
    sidebar->addStretch();
    sidebar->addWidget(logoutBtn);

    // Make sidebar scrollable for small screens
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidget(sideWidget);
    scroll->setWidgetResizable(true);
    scroll->setFixedWidth(220);
    scroll->setFrameShape(QFrame::NoFrame);

    // ── BORROW TABLE ──────────────────────────────────────────
    borrowTable = new QTableWidget(this);
    borrowTable->setColumnCount(5);
    borrowTable->setHorizontalHeaderLabels(
        {"Record ID", "Book ID", "Issue Date", "Due Date", "Status"});
    borrowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowTable->setAlternatingRowColors(true);

    root->addWidget(scroll);
    root->addWidget(borrowTable, 1);

    // ── SIGNALS ───────────────────────────────────────────────
    connect(profileBtn,   &QPushButton::clicked, this, &UserWindow::onMyProfile);
    connect(updateBtn,    &QPushButton::clicked, this, &UserWindow::onUpdateProfile);
    connect(statementBtn, &QPushButton::clicked, this, &UserWindow::onAccountStatement);
    connect(browseBtn,    &QPushButton::clicked, this, &UserWindow::onBrowseBooks);
    connect(searchBtn,    &QPushButton::clicked, this, &UserWindow::onSearchBooks);
    connect(borrowBtn,    &QPushButton::clicked, this, &UserWindow::onBorrowBook);
    connect(returnBtn,    &QPushButton::clicked, this, &UserWindow::onReturnBook);
    connect(holdBtn,      &QPushButton::clicked, this, &UserWindow::onPlaceHold);
    connect(historyBtn,   &QPushButton::clicked, this, &UserWindow::onViewHistory);
    connect(currBorrBtn,  &QPushButton::clicked, this, &UserWindow::onCurrentlyBorrowed);
    connect(holdsBtn,     &QPushButton::clicked, this, &UserWindow::onMyHolds);
    connect(payBtn,       &QPushButton::clicked, this, &UserWindow::onPayFine);
    connect(depositBtn,   &QPushButton::clicked, this, &UserWindow::onDeposit);
    connect(upgradeBtn,   &QPushButton::clicked, this, &UserWindow::onUpgradeMembership);
    connect(logoutBtn,    &QPushButton::clicked, this, &UserWindow::onLogout);
}

// ─────────────────────────────────────────────────────────────
//  REFRESH HELPERS
// ─────────────────────────────────────────────────────────────
void UserWindow::refreshBorrowTable()
{
    borrowTable->setRowCount(0);
    for (auto& rec : currentUser->getBorrowHistory()) {
        int row = borrowTable->rowCount();
        borrowTable->insertRow(row);
        borrowTable->setItem(row, 0, new QTableWidgetItem(QString::number(rec->getRecordID())));
        borrowTable->setItem(row, 1, new QTableWidgetItem(QString::number(rec->getResourceID())));
        borrowTable->setItem(row, 2, new QTableWidgetItem(
                                         QString::fromStdString(rec->getIssueDate().toString())));
        borrowTable->setItem(row, 3, new QTableWidgetItem(
                                         QString::fromStdString(rec->getDueDate().toString())));

        QString status = QString::fromStdString(rec->getStatus());
        QTableWidgetItem* si = new QTableWidgetItem(status);
        if      (rec->isActive())  si->setForeground(Qt::darkGreen);
        else if (rec->isOverdue()) si->setForeground(Qt::red);
        else                       si->setForeground(Qt::gray);
        borrowTable->setItem(row, 4, si);
    }
}

void UserWindow::refreshAccountInfo()
{
    Account*    acc = currentUser->getAccount();
    Membership* mem = currentUser->getMembership();

    balanceLabel->setText("Balance: $" + QString::number((int)acc->getBalance()));

    double fine = acc->getFinesDue();
    fineLabel->setText("Fine Due: $" + QString::number((int)fine));
    fineLabel->setStyleSheet(fine > 0 ? "color: red;" : "color: green;");

    borrowCountLabel->setText("Borrows: " +
                              QString::number(currentUser->getCurrentBorrows()) + "/" +
                              QString::number(mem->getMaxBooks()));

    QString tier = QString::fromStdString(mem->getLevel());
    membershipLabel->setText("Tier: " + tier);
    if      (tier == "Gold")   membershipLabel->setStyleSheet("font-weight:bold; color:#d4ac0d;");
    else if (tier == "Silver") membershipLabel->setStyleSheet("font-weight:bold; color:#aab7b8;");
    else                       membershipLabel->setStyleSheet("font-weight:bold; color:#7f8c8d;");
}

// ─────────────────────────────────────────────────────────────
//  SHARED: show a table of books in a dialog
// ─────────────────────────────────────────────────────────────
void UserWindow::showBookTable(const QString& title, const vector<Resource*>& books)
{
    QDialog*     dlg    = new QDialog(this);
    dlg->setWindowTitle(title);
    dlg->setMinimumSize(700, 400);
    QVBoxLayout* layout = new QVBoxLayout(dlg);

    QTableWidget* tbl = new QTableWidget(dlg);
    tbl->setColumnCount(5);
    tbl->setHorizontalHeaderLabels({"ID","Title","Author","Copies Left","Status"});
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setAlternatingRowColors(true);

    for (auto* r : books) {
        int row = tbl->rowCount();
        tbl->insertRow(row);
        tbl->setItem(row, 0, new QTableWidgetItem(QString::number(r->getResourceID())));
        tbl->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r->getTitle())));

        Book* b = dynamic_cast<Book*>(r);
        tbl->setItem(row, 2, new QTableWidgetItem(
                                 b ? QString::fromStdString(b->getAuthor()) : "—"));

    tbl->setItem(row, 3, new QTableWidgetItem(
                         QString::number(r->getTotalAvailableCopies())));
        bool avail = r->checkAvailability();
        QTableWidgetItem* si = new QTableWidgetItem(avail ? "Available" : "Unavailable");
        si->setForeground(avail ? Qt::darkGreen : Qt::red);
        tbl->setItem(row, 4, si);
    }

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(btns, &QDialogButtonBox::rejected, dlg, &QDialog::accept);
    layout->addWidget(tbl);
    layout->addWidget(btns);
    dlg->exec();
}

// ─────────────────────────────────────────────────────────────
//  SLOT IMPLEMENTATIONS
// ─────────────────────────────────────────────────────────────

// 1. My Profile
void UserWindow::onMyProfile()
{
    User* u   = currentUser;
    Account* a = u->getAccount();
    Membership* m = u->getMembership();

    QString info =
        QString("ID:           %1\n"
                "Name:         %2 %3\n"
                "Username:     %4\n"
                "Email:        %5\n"
                "Address:      %6\n"
                "Registered:   %7\n"
                "Membership:   %8\n"
                "Borrows:      %9 / %10\n"
                "Balance:      $%11\n"
                "Fine Due:     $%12")
            .arg(u->getUserID())
            .arg(QString::fromStdString(u->getFirstName()))
            .arg(QString::fromStdString(u->getLastName()))
            .arg(QString::fromStdString(u->getUsername()))
            .arg(QString::fromStdString(u->getEmail()))
            .arg(QString::fromStdString(u->getAddress()))
            .arg(QString::fromStdString(u->getRegistrationDate()))
            .arg(QString::fromStdString(m->getLevel()))
            .arg(u->getCurrentBorrows())
            .arg(m->getMaxBooks())
            .arg((int)a->getBalance())
            .arg((int)a->getFinesDue());

    QMessageBox::information(this, "My Profile", info);
}

// 2. Browse Available Books
void UserWindow::onBrowseBooks()
{
    vector<Resource*> available;
    for (const auto& res : admin.getResources())
        if (res->checkAvailability()) available.push_back(res.get());

    if (available.empty()) {
        QMessageBox::information(this, "Browse Books", "No books currently available.");
        return;
    }
    showBookTable("Browse Available Books", available);
}

// 7. Currently Borrowed
void UserWindow::onCurrentlyBorrowed()
{
    QString info;
    for (auto& rec : currentUser->getBorrowHistory()) {
        if (rec->isActive()) {
            info += QString("Record#%1 | Book#%2 | Due: %3\n")
            .arg(rec->getRecordID())
                .arg(rec->getResourceID())
                .arg(QString::fromStdString(rec->getDueDate().toString()));
        }
    }
    if (info.isEmpty()) info = "You have no books currently borrowed.";
    QMessageBox::information(this, "Currently Borrowed", info);
}

// 8. My Holds
void UserWindow::onMyHolds() {
    auto holds = currentUser->getHoldList();

    QString pendingInfo;
    QString approvedInfo;

    for (auto* h : holds) {
        QString title = "Book#" + QString::number(h->getResourceID());
        try {
            Resource* r = admin.findBook(h->getResourceID());
            title = QString::fromStdString(r->getTitle());
        } catch (...) {}

        if (h->getStatus() == "pending") {
            pendingInfo += QString("Hold#%1 | %2 | Queue position: %3\n")
            .arg(h->getHoldID())
                .arg(title)
                .arg(h->getQueuePosition());
        } else if (h->getStatus() == "approved") {
            approvedInfo += QString("Hold#%1 | %2 | READY — borrow it now!\n")
                                .arg(h->getHoldID())
                                .arg(title);
        }
    }

    QString info;
    if (!approvedInfo.isEmpty())
        info += "=== READY TO BORROW ===\n" + approvedInfo + "\n";
    if (!pendingInfo.isEmpty())
        info += "=== WAITING IN QUEUE ===\n" + pendingInfo;

    if (info.isEmpty())
        QMessageBox::information(this, "My Holds", "You have no active holds.");
    else
        QMessageBox::information(this, "My Holds", info);
}

// 11. Account Statement
void UserWindow::onAccountStatement()
{
    Account* a = currentUser->getAccount();
    QString info =
        QString("Account ID:     %1\n"
                "Balance:        $%2\n"
                "Fine Due:       $%3\n"
                "Total Borrowed: %4")
            .arg(a->getAccountID())
            .arg((int)a->getBalance())
            .arg((int)a->getFinesDue())
            .arg(a->getTotalBorrowed());
    QMessageBox::information(this, "Account Statement", info);
}

// 12. Search Books
void UserWindow::onSearchBooks()
{
    bool ok;
    QString query = QInputDialog::getText(this, "Search Books",
                                          "Enter title or author:", QLineEdit::Normal, "", &ok);
    if (!ok || query.trimmed().isEmpty()) return;

    string q = query.toLower().toStdString();
    vector<Resource*> results;

    for (const auto& res : admin.getResources()) {
        Resource* r  = res.get();
        string title = toLower(r->getTitle());
        Book*  b     = dynamic_cast<Book*>(r);
        string auth  = b ? toLower(b->getAuthor()) : "";
        if (title.find(q) != string::npos || auth.find(q) != string::npos)
            results.push_back(r);
    }

    if (results.empty()) {
        QMessageBox::information(this, "Search Books",
                                 "No books found matching \"" + query + "\".");
        return;
    }
    showBookTable("Search Results — \"" + query + "\"", results);
}

// 13. Update Profile
void UserWindow::onUpdateProfile()
{
    bool ok;

    QString newAddr = QInputDialog::getText(this, "Update Profile",
                                            "New address (min 5 chars):",
                                            QLineEdit::Normal,
                                            QString::fromStdString(currentUser->getAddress()),
                                            &ok);
    if (!ok) return;
    if (newAddr.trimmed().length() < 5) {
        QMessageBox::warning(this, "Update Profile", "Address too short (min 5 characters).");
        return;
    }

    QString newEmail = QInputDialog::getText(this, "Update Profile",
                                             "New email:",
                                             QLineEdit::Normal,
                                             QString::fromStdString(currentUser->getEmail()),
                                             &ok);
    if (!ok) return;
    if (!isValidEmail(newEmail.toStdString())) {
        QMessageBox::warning(this, "Update Profile",
                             "Invalid email format.\nExample: user@domain.com");
        return;
    }

    currentUser->setAddress(newAddr.toStdString());
    currentUser->setEmail(newEmail.toStdString());
    QMessageBox::information(this, "Update Profile", "Profile updated successfully!");
}

// ─────────────────────────────────────────────────────────────
//  EXISTING SLOTS (unchanged)
// ─────────────────────────────────────────────────────────────
void UserWindow::onBorrowBook() {
    bool ok;
    int bid = QInputDialog::getInt(this,"Borrow Book","Enter Book ID:",1,1,99999,1,&ok);
    if (!ok) return;
    try {
        Resource* book = admin.findBook(bid);
        currentUser->borrowBook(book);
        refreshBorrowTable(); refreshAccountInfo();
        QMessageBox::information(this,"Success",
                                 "'" + QString::fromStdString(book->getTitle()) + "' borrowed!");
    } catch (exception& e) { QMessageBox::critical(this,"Error",e.what()); }
}

void UserWindow::onReturnBook() {
    bool ok;
    int bid = QInputDialog::getInt(this,"Return Book","Enter Book ID:",1,1,99999,1,&ok);
    if (!ok) return;
    try {
        Resource* book = admin.findBook(bid);
        currentUser->returnBook(book);
        admin.checkAndNotifyHolds(bid);
        refreshBorrowTable(); refreshAccountInfo();
        QMessageBox::information(this,"Success","Book returned!");
    } catch (exception& e) { QMessageBox::critical(this,"Error",e.what()); }
}

void UserWindow::onPlaceHold() {
    bool ok;
    int bid = QInputDialog::getInt(this, "Place Hold", "Enter Book ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    try {
        Resource* book = admin.findBook(bid);

        // Book is available — tell user to borrow directly
        if (book->checkAvailability()) {
            QMessageBox::information(this, "Place Hold",
                                     "'" + QString::fromStdString(book->getTitle()) +
                                         "' is available — just borrow it directly!");
            return;
        }

        // Duplicate hold guard
        for (auto* h : currentUser->getHoldList()) {
            if (h->getResourceID() == bid && h->getStatus() == "pending") {
                QMessageBox::warning(this, "Place Hold",
                                     "You already have an active hold on this book.");
                return;
            }
        }

        currentUser->placeHold(book);
        QMessageBox::information(this, "Hold Placed",
                                 "Hold placed on '" + QString::fromStdString(book->getTitle()) + "'.");

    } catch (exception& e) { QMessageBox::critical(this, "Error", e.what()); }
}

void UserWindow::onPayFine() {
    double fine    = currentUser->getAccount()->getFinesDue();
    double balance = currentUser->getAccount()->getBalance();

    if (fine <= 0) {
        QMessageBox::information(this, "No Fine", "No outstanding fines!"); return;
    }

    // warn upfront if balance is insufficient
    if (balance <= 0) {
        QMessageBox::warning(this, "Insufficient Balance",
                             "You have no balance to pay your fine.\n"
                             "Please deposit funds first.");
        return;
    }

    double maxPayable = std::min(fine, balance);   // can't pay more than you have

    bool ok;
    double amt = QInputDialog::getDouble(this, "Pay Fine",
                                         QString("Fine Due: $%1\nYour Balance: $%2\nAmount to pay:")
                                             .arg((int)fine).arg((int)balance),
                                         maxPayable, 0.01, maxPayable, 2, &ok);

    if (!ok) return;

    try {
        currentUser->payFine(amt);
        refreshAccountInfo();
        QMessageBox::information(this, "Success", "Fine of $" +
                                                      QString::number((int)amt) + " paid successfully!");
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void UserWindow::onDeposit() {
    bool ok;
    double amt = QInputDialog::getDouble(this,"Deposit","Amount ($):",10,0.01,99999,2,&ok);
    if (!ok) return;
    try {
        currentUser->getAccount()->deposit(amt);
        refreshAccountInfo();
        QMessageBox::information(this,"Success","Deposited $"+QString::number((int)amt));
    } catch (exception& e) { QMessageBox::critical(this,"Error",e.what()); }
}

void UserWindow::onViewHistory() {
    QString info;
    for (auto& rec : currentUser->getBorrowHistory()) {
        info += QString("Record#%1 | Book#%2 | %3 → %4 | %5\n")
                    .arg(rec->getRecordID()).arg(rec->getResourceID())
                    .arg(QString::fromStdString(rec->getIssueDate().toString()))
                    .arg(QString::fromStdString(rec->getDueDate().toString()))
                    .arg(QString::fromStdString(rec->getStatus()));
    }
    if (info.isEmpty()) info = "No borrow history.";
    QMessageBox::information(this,"Borrow History",info);
}

void UserWindow::onLogout() {
    currentUser->logout();
    emit sig_logout();
}

void UserWindow::onUpgradeMembership() {
    string current = currentUser->getMembership()->getLevel();
    if (current == "Gold") {
        QMessageBox::information(this,"Membership","Already at Gold — highest tier!"); return;
    }
    QStringList options;
    if (current == "Basic")
        options << "Silver ($10) — 3 books, 21 days, 10% off"
                << "Gold   ($25) — 4 books, 30 days, 20% off";
    else
        options << "Gold ($25) — 4 books, 30 days, 20% off";

    bool ok;
    QString choice = QInputDialog::getItem(this,"Upgrade Membership",
                                           "Current: " + QString::fromStdString(current) + "\nSelect tier:",
                                           options, 0, false, &ok);
    if (!ok) return;

    double cost     = choice.contains("Gold") ? 25.0 : 10.0;
    string newLevel = choice.contains("Gold") ? "Gold" : "Silver";
    double balance  = currentUser->getAccount()->getBalance();

    if (balance < cost) {
        QMessageBox::warning(this,"Insufficient Balance",
                             "Need $" + QString::number((int)cost) +
                                 " but have $" + QString::number((int)balance) + ". Deposit first.");
        return;
    }
    currentUser->getAccount()->setBalance(balance - cost);
    currentUser->getMembership()->upgrade(newLevel);
    refreshAccountInfo();
    QMessageBox::information(this,"Upgraded!",
                             "Now " + QString::fromStdString(newLevel) +
                                 "! $" + QString::number((int)cost) + " deducted.");
}
