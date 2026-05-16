#include "adminwindow.h"
#include "addbookdialog.h"
#include "adduserdialog.h"
#include "overduedialog.h"
#include "userstabledialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────

AdminWindow::AdminWindow(Administrator &adm, QWidget *parent)
    : QMainWindow(parent), admin(adm)
{
    setWindowTitle("Admin Panel — " + QString::fromStdString(admin.getAdminName()));
    setMinimumSize(1080, 680);
    setupUI();
    refreshBookTable();
}

// ── Helper: show a filtered book list in a modal table dialog ────────────────

void AdminWindow::showBookResultsDialog(const QString &title,
                                        std::function<bool(Resource*)> filter)
{
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.setMinimumSize(800, 450);

    auto *table = new QTableWidget(&dlg);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(
        {"ID", "Title", "Author", "Genre", "Year", "Available", "Total Copies"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);

    for (auto &r : admin.getResources()) {
        if (!filter(r.get())) continue;
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::number(r->getResourceID())));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r->getTitle())));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r->getAuthor())));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(r->getGenre())));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(r->getPublishYear())));
        auto *avItem = new QTableWidgetItem(
            QString::number(r->getTotalAvailableCopies()));
        avItem->setForeground(r->checkAvailability() ? QColor("#1a7a1a") : Qt::red);
        table->setItem(row, 5, avItem);
        table->setItem(row, 6, new QTableWidgetItem(QString::number(r->getMaxCopies())));
    }

    auto *countLabel = new QLabel(
        table->rowCount() == 0
            ? "No matching books found."
            : QString("%1 book(s) found.").arg(table->rowCount()), &dlg);
    countLabel->setStyleSheet("font-weight: bold; padding: 4px;");

    auto *closeBtn = new QPushButton("Close", &dlg);
    auto *layout   = new QVBoxLayout(&dlg);
    layout->addWidget(countLabel);
    layout->addWidget(table);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

// ── Build the UI ─────────────────────────────────────────────────────────────

void AdminWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setSpacing(8);

    // ── Scrollable sidebar ───────────────────────────────────────────────────
    QWidget *sideContent = new QWidget();
    auto *sidebar = new QVBoxLayout(sideContent);
    sidebar->setSpacing(6);
    sidebar->setContentsMargins(4, 4, 4, 4);

    auto makeBtn = [&](const QString &label) -> QPushButton* {
        auto *b = new QPushButton(label, sideContent);
        b->setMinimumHeight(27);
        return b;
    };

    // Catalog group
    auto *catalogBox = new QGroupBox("Catalog", sideContent);
    auto *catL       = new QVBoxLayout(catalogBox);
    catL->setSpacing(3);
    auto *allBooksBtn   = makeBtn("All Books");
    auto *availBtn      = makeBtn("Available Books");
    auto *addBookBtn    = makeBtn("Add Book");
    auto *updateBookBtn = makeBtn("Update Book");
    auto *removeBookBtn = makeBtn("Remove Book");
    catL->addWidget(allBooksBtn);
    catL->addWidget(availBtn);
    catL->addWidget(addBookBtn);
    catL->addWidget(updateBookBtn);
    catL->addWidget(removeBookBtn);

    // Search group
    auto *searchBox = new QGroupBox("Search", sideContent);
    auto *searchL   = new QVBoxLayout(searchBox);
    searchL->setSpacing(3);
    auto *searchBtn        = makeBtn("Title / Author");
    auto *searchAuthorBtn  = makeBtn("By Author");
    auto *searchGenreBtn   = makeBtn("By Genre");
    auto *searchYearBtn    = makeBtn("By Year");
    searchL->addWidget(searchBtn);
    searchL->addWidget(searchAuthorBtn);
    searchL->addWidget(searchGenreBtn);
    searchL->addWidget(searchYearBtn);

    // Operations group
    auto *opsBox = new QGroupBox("Operations", sideContent);
    auto *opsL   = new QVBoxLayout(opsBox);
    opsL->setSpacing(3);
    auto *issueBtn  = makeBtn("Issue Book");
    auto *returnBtn = makeBtn("Return Book");
    auto *holdBtn   = makeBtn("Place Hold");
    opsL->addWidget(issueBtn);
    opsL->addWidget(returnBtn);
    opsL->addWidget(holdBtn);

    // Users group
    auto *userBox = new QGroupBox("Users", sideContent);
    auto *userL   = new QVBoxLayout(userBox);
    userL->setSpacing(3);
    auto *addUserBtn     = makeBtn("Add User");
    auto *showUsersBtn   = makeBtn("All Users");
    auto *profileBtn     = makeBtn("View User Profile");
    auto *borrowHistBtn  = makeBtn("Borrow History");
    auto *upgradeBtn     = makeBtn("Upgrade Membership");
    userL->addWidget(addUserBtn);
    userL->addWidget(showUsersBtn);
    userL->addWidget(profileBtn);
    userL->addWidget(borrowHistBtn);
    userL->addWidget(upgradeBtn);

    // Reports group
    auto *reportBox = new QGroupBox("Reports", sideContent);
    auto *repL      = new QVBoxLayout(reportBox);
    repL->setSpacing(3);
    auto *adminProfBtn = makeBtn("Admin Profile");
    auto *overdueBtn   = makeBtn("Overdue Report");
    auto *customerBtn  = makeBtn("Customer Report");
    auto *issuedBtn    = makeBtn("Currently Issued");
    repL->addWidget(adminProfBtn);
    repL->addWidget(overdueBtn);
    repL->addWidget(customerBtn);
    repL->addWidget(issuedBtn);

    // Logout button
    auto *logoutBtn = new QPushButton("Save & Logout", sideContent);
    logoutBtn->setStyleSheet(
        "background-color: #e74c3c; color: white; font-weight: bold; padding: 8px;");

    sidebar->addWidget(catalogBox);
    sidebar->addWidget(searchBox);
    sidebar->addWidget(opsBox);
    sidebar->addWidget(userBox);
    sidebar->addWidget(reportBox);
    sidebar->addStretch();
    sidebar->addWidget(logoutBtn);

    auto *scroll = new QScrollArea(this);
    scroll->setWidget(sideContent);
    scroll->setWidgetResizable(true);
    scroll->setFixedWidth(205);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ── Main book table ──────────────────────────────────────────────────────
    bookTable = new QTableWidget(this);
    bookTable->setColumnCount(6);
    bookTable->setHorizontalHeaderLabels(
        {"ID", "Title", "Author", "Genre", "Year", "Copies"});
    bookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookTable->setAlternatingRowColors(true);

    root->addWidget(scroll);
    root->addWidget(bookTable, 1);

    // ── Wire signals ─────────────────────────────────────────────────────────
    connect(allBooksBtn,    &QPushButton::clicked, this, &AdminWindow::onAllBooks);
    connect(availBtn,       &QPushButton::clicked, this, &AdminWindow::onAvailableBooks);
    connect(addBookBtn,     &QPushButton::clicked, this, &AdminWindow::onAddBook);
    connect(updateBookBtn,  &QPushButton::clicked, this, &AdminWindow::onUpdateBook);
    connect(removeBookBtn,  &QPushButton::clicked, this, &AdminWindow::onRemoveBook);
    connect(searchBtn,      &QPushButton::clicked, this, &AdminWindow::onSearchBooks);
    connect(searchAuthorBtn,&QPushButton::clicked, this, &AdminWindow::onSearchByAuthor);
    connect(searchGenreBtn, &QPushButton::clicked, this, &AdminWindow::onSearchByGenre);
    connect(searchYearBtn,  &QPushButton::clicked, this, &AdminWindow::onSearchByYear);
    connect(issueBtn,       &QPushButton::clicked, this, &AdminWindow::onIssueBook);
    connect(returnBtn,      &QPushButton::clicked, this, &AdminWindow::onReturnBook);
    connect(holdBtn,        &QPushButton::clicked, this, &AdminWindow::onPlaceHold);
    connect(addUserBtn,     &QPushButton::clicked, this, &AdminWindow::onAddUser);
    connect(showUsersBtn,   &QPushButton::clicked, this, &AdminWindow::onShowUsers);
    connect(profileBtn,     &QPushButton::clicked, this, &AdminWindow::onViewUserProfile);
    connect(borrowHistBtn,  &QPushButton::clicked, this, &AdminWindow::onViewBorrowHistory);
    connect(upgradeBtn,     &QPushButton::clicked, this, &AdminWindow::onUpgradeMembership);
    connect(adminProfBtn,   &QPushButton::clicked, this, &AdminWindow::onAdminProfile);
    connect(overdueBtn,     &QPushButton::clicked, this, &AdminWindow::onOverdueReport);
    connect(customerBtn,    &QPushButton::clicked, this, &AdminWindow::onCustomerReport);
    connect(issuedBtn,      &QPushButton::clicked, this, &AdminWindow::onCurrentlyIssued);
    connect(logoutBtn,      &QPushButton::clicked, this, &AdminWindow::onSaveLogout);
}

void AdminWindow::refreshBookTable() {
    bookTable->setRowCount(0);
    for (auto &r : admin.getResources()) {
        int row = bookTable->rowCount();
        bookTable->insertRow(row);
        bookTable->setItem(row, 0, new QTableWidgetItem(QString::number(r->getResourceID())));
        bookTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r->getTitle())));
        bookTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r->getAuthor())));
        bookTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(r->getGenre())));
        bookTable->setItem(row, 4, new QTableWidgetItem(QString::number(r->getPublishYear())));
        auto *copItem = new QTableWidgetItem(
            QString::number(r->getTotalAvailableCopies()));
        if (r->getTotalAvailableCopies() == 0) copItem->setForeground(Qt::red);
        bookTable->setItem(row, 5, copItem);
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  CATALOG SLOTS
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onAllBooks() {
    showBookResultsDialog("All Books — Catalog",
                          [](Resource*) { return true; });
}

void AdminWindow::onAvailableBooks() {
    showBookResultsDialog("Available Books",
                          [](Resource *r) { return r->checkAvailability(); });
}

void AdminWindow::onAddBook() {
    AddBookDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        try {
            admin.addBookDirect(dlg.getBook());
            refreshBookTable();
            QMessageBox::information(this, "Success", "Book added successfully!");
        } catch (exception &e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void AdminWindow::onUpdateBook() {
    bool ok;
    int id = QInputDialog::getInt(this, "Update Book",
                                  "Enter Book ID to update:", 1, 1, 99999, 1, &ok);
    if (!ok) return;

    Resource *r = nullptr;
    try { r = admin.findBook(id); }
    catch (exception &e) { QMessageBox::critical(this, "Not Found", e.what()); return; }

    Book *b = dynamic_cast<Book*>(r);
    if (!b) { QMessageBox::warning(this, "Error", "Resource is not a Book."); return; }

    // Build an inline edit dialog pre-populated with current values
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Update Book #%1").arg(id));
    dlg.setMinimumWidth(460);

    auto *form       = new QFormLayout();
    auto *titleEdit  = new QLineEdit(QString::fromStdString(b->getTitle()),    &dlg);
    auto *authorEdit = new QLineEdit(QString::fromStdString(b->getAuthor()),   &dlg);
    auto *catEdit    = new QLineEdit(QString::fromStdString(b->getCategory()), &dlg);
    auto *yearSpin   = new QSpinBox(&dlg);
    yearSpin->setRange(1000, 2026);
    yearSpin->setValue(b->getPublishYear());
    auto *copiesSpin = new QSpinBox(&dlg);
    copiesSpin->setRange(1, 999);
    copiesSpin->setValue(b->getMaxCopies());
    copiesSpin->setToolTip("Total library copies. Cannot be reduced below currently-borrowed count.");
    auto *pubEdit    = new QLineEdit(QString::fromStdString(b->getPublisher()),&dlg);
    auto *langEdit   = new QLineEdit(QString::fromStdString(b->getLanguage()), &dlg);
    auto *pagesSpin  = new QSpinBox(&dlg);
    pagesSpin->setRange(1, 10000);
    pagesSpin->setValue(b->getPageCount());
    auto *shelfEdit  = new QLineEdit(QString::fromStdString(b->getShelfCode()),&dlg);
    auto *genreEdit  = new QLineEdit(QString::fromStdString(b->getGenre()),    &dlg);
    auto *dueSpin    = new QSpinBox(&dlg);
    dueSpin->setRange(1, 365);
    dueSpin->setValue(b->getDueDays());
    auto *bestCheck  = new QCheckBox(&dlg);
    bestCheck->setChecked(b->getIsBestseller());

    form->addRow("Title:",        titleEdit);
    form->addRow("Author:",       authorEdit);
    form->addRow("Category:",     catEdit);
    form->addRow("Publish Year:", yearSpin);
    form->addRow("Total Copies:", copiesSpin);
    form->addRow("Publisher:",    pubEdit);
    form->addRow("Language:",     langEdit);
    form->addRow("Pages:",        pagesSpin);
    form->addRow("Shelf Code:",   shelfEdit);
    form->addRow("Genre:",        genreEdit);
    form->addRow("Due Days:",     dueSpin);
    form->addRow("Bestseller:",   bestCheck);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    auto *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->addLayout(form);
    mainLayout->addWidget(btns);

    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, [&]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", "Title cannot be empty."); return;
        }
        if (authorEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", "Author cannot be empty."); return;
        }
        dlg.accept();
    });
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;

    b->setTitle     (titleEdit->text().trimmed().toStdString());
    b->setAuthor    (authorEdit->text().trimmed().toStdString());
    b->setCategory  (catEdit->text().trimmed().toStdString());
    b->setPublishYear(yearSpin->value());
    b->setPublisher (pubEdit->text().trimmed().toStdString());
    b->setLanguage  (langEdit->text().trimmed().toStdString());
    b->setPageCount (pagesSpin->value());
    b->setShelfCode (shelfEdit->text().trimmed().toStdString());
    b->setGenre     (genreEdit->text().trimmed().toStdString());
    b->setDueDays   (dueSpin->value());
    b->setBestseller(bestCheck->isChecked());
    try {
        b->updateTotalCopies(copiesSpin->value());
    } catch (...) {
        QMessageBox::warning(this, "Copies",
                             "Could not reduce copies — copies are currently borrowed.\n"
                             "All other fields were updated.");
    }
    refreshBookTable();
    QMessageBox::information(this, "Updated", "Book updated successfully!");
    Logger::log("Admin updated book ID=" + to_string(id), Logger::ACTION);
}

void AdminWindow::onRemoveBook() {
    bool ok;
    int id = QInputDialog::getInt(this, "Remove Book",
                                  "Enter Book ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    try {
        admin.removeResource(id);
        refreshBookTable();
        QMessageBox::information(this, "Success", "Book removed.");
    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  SEARCH SLOTS
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onSearchBooks() {
    bool ok;
    QString kw = QInputDialog::getText(this, "Search Books",
                                       "Enter title or author keyword:",
                                       QLineEdit::Normal, "", &ok);
    if (!ok || kw.trimmed().isEmpty()) return;
    string kwLow = toLower(kw.trimmed().toStdString());
    showBookResultsDialog(
        "Search: \"" + kw.trimmed() + "\"",
        [kwLow](Resource *r) {
            return toLower(r->getTitle()).find(kwLow)  != string::npos ||
                   toLower(r->getAuthor()).find(kwLow) != string::npos;
        });
}

void AdminWindow::onSearchByAuthor() {
    bool ok;
    QString author = QInputDialog::getText(this, "Search by Author",
                                           "Enter author name:",
                                           QLineEdit::Normal, "", &ok);
    if (!ok || author.trimmed().isEmpty()) return;
    string kwLow = toLower(author.trimmed().toStdString());
    showBookResultsDialog(
        "Author: \"" + author.trimmed() + "\"",
        [kwLow](Resource *r) {
            return toLower(r->getAuthor()).find(kwLow) != string::npos;
        });
}

void AdminWindow::onSearchByGenre() {
    bool ok;
    QString genre = QInputDialog::getText(this, "Search by Genre",
                                          "Enter genre:",
                                          QLineEdit::Normal, "", &ok);
    if (!ok || genre.trimmed().isEmpty()) return;
    string kwLow = toLower(genre.trimmed().toStdString());
    showBookResultsDialog(
        "Genre: \"" + genre.trimmed() + "\"",
        [kwLow](Resource *r) {
            return !r->getGenre().empty() &&
                   toLower(r->getGenre()).find(kwLow) != string::npos;
        });
}

void AdminWindow::onSearchByYear() {
    bool ok;
    int year = QInputDialog::getInt(this, "Search by Year",
                                    "Enter publish year:", 2024, 1000, 2026, 1, &ok);
    if (!ok) return;
    showBookResultsDialog(
        QString("Year: %1").arg(year),
        [year](Resource *r) { return r->getPublishYear() == year; });
}

// ══════════════════════════════════════════════════════════════════════════════
//  OPERATIONS SLOTS
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onIssueBook() {
    bool ok;
    int uid = QInputDialog::getInt(this, "Issue Book",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    int bid = QInputDialog::getInt(this, "Issue Book",
                                   "Enter Book ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    try {
        User     *u = admin.findUser(uid);
        Resource *b = admin.findBook(bid);
        u->borrowBook(b);
        refreshBookTable();
        QMessageBox::information(this, "Success",
                                 "'" + QString::fromStdString(b->getTitle()) + "' issued to " +
                                     QString::fromStdString(u->getFirstName()) + " " +
                                     QString::fromStdString(u->getLastName()) + ".");
    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void AdminWindow::onReturnBook() {
    bool ok;
    int uid = QInputDialog::getInt(this, "Return Book",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    int bid = QInputDialog::getInt(this, "Return Book",
                                   "Enter Book ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    try {
        User     *u = admin.findUser(uid);
        Resource *b = admin.findBook(bid);
        u->returnBook(b);
        admin.checkAndNotifyHolds(bid);
        refreshBookTable();
        QMessageBox::information(this, "Success", "Book returned successfully.");
    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void AdminWindow::onPlaceHold() {
    bool ok;
    int uid = QInputDialog::getInt(this, "Place Hold",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    int bid = QInputDialog::getInt(this, "Place Hold",
                                   "Enter Book ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    try {
        User     *u = admin.findUser(uid);
        Resource *b = admin.findBook(bid);
        if (b->checkAvailability()) {
            QMessageBox::information(this, "Hold",
                                     "Book is currently available — issue it directly instead.");
            return;
        }
        u->placeHold(b);
        QMessageBox::information(this, "Hold Placed",
                                 "Hold placed on '" + QString::fromStdString(b->getTitle()) +
                                     "' for " + QString::fromStdString(u->getFirstName()) + ".");
    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  USER MANAGEMENT SLOTS
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onAddUser() {
    AddUserDialog dlg(admin, this);
    dlg.exec();
}

void AdminWindow::onShowUsers() {
    UsersTableDialog dlg(admin, this);
    dlg.exec();
}

void AdminWindow::onViewUserProfile() {
    bool ok;
    int uid = QInputDialog::getInt(this, "View User Profile",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;

    User *u = nullptr;
    try { u = admin.findUser(uid); }
    catch (exception &e) { QMessageBox::critical(this, "Not Found", e.what()); return; }

    QString sep = "─────────────────────────────────────\n";
    QString info;
    info += sep;
    info += QString("USER #%1 — %2 %3\n")
                .arg(u->getUserID())
                .arg(QString::fromStdString(u->getFirstName()))
                .arg(QString::fromStdString(u->getLastName()));
    info += sep;
    info += QString("Username     : @%1\n").arg(QString::fromStdString(u->getUsername()));
    info += QString("Email        : %1\n").arg(QString::fromStdString(u->getEmail()));
    info += QString("Address      : %1\n").arg(QString::fromStdString(u->getAddress()));
    info += QString("Registered   : %1\n").arg(
        QString::fromStdString(u->getRegistrationDate()));
    info += sep;
    info += QString("Membership   : %1\n").arg(
        QString::fromStdString(u->getMembership()->getLevel()));
    info += QString("Max Books    : %1   Borrow Duration: %2 days\n")
                .arg(u->getMembership()->getMaxBooks())
                .arg(u->getMembership()->getBorrowDuration());
    info += QString("Discount     : %1%%\n")
                .arg((int)u->getMembership()->getDiscountRate());
    info += sep;
    info += QString("Active Borrows : %1 / %2\n")
                .arg(u->getCurrentBorrows())
                .arg(u->getMembership()->getMaxBooks());
    info += QString("Balance      : $%1\n")
                .arg((int)u->getAccount()->getBalance());
    info += QString("Fine Due     : $%1\n")
                .arg((int)u->getAccount()->getFinesDue());
    info += QString("Total Borrowed: %1\n")
                .arg(u->getAccount()->getTotalBorrowed());

    QMessageBox::information(this, "User Profile", info);
}

void AdminWindow::onViewBorrowHistory() {
    bool ok;
    int uid = QInputDialog::getInt(this, "Borrow History",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;

    User *u = nullptr;
    try { u = admin.findUser(uid); }
    catch (exception &e) { QMessageBox::critical(this, "Not Found", e.what()); return; }

    auto records = u->getBorrowHistory();
    if (records.empty()) {
        QMessageBox::information(this, "Borrow History",
                                 "No borrow records for this user.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Borrow History — %1 %2 (ID:%3)")
                           .arg(QString::fromStdString(u->getFirstName()))
                           .arg(QString::fromStdString(u->getLastName()))
                           .arg(uid));
    dlg.setMinimumSize(820, 440);

    auto *table = new QTableWidget(&dlg);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(
        {"Rec #", "Book ID", "Issue Date", "Due Date", "Return Date", "Status", "Fine ($)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);

    for (auto *rec : records) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::number(rec->getRecordID())));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(rec->getResourceID())));
        table->setItem(row, 2, new QTableWidgetItem(
                                   QString::fromStdString(rec->getIssueDate().toString())));
        table->setItem(row, 3, new QTableWidgetItem(
                                   QString::fromStdString(rec->getDueDate().toString())));
        // Return date only meaningful once closed
        QString retStr = rec->hasReturn()
                             ? QString::fromStdString(rec->getReturnDate().toString())
                             : "—";
        table->setItem(row, 4, new QTableWidgetItem(retStr));

        auto *statusItem = new QTableWidgetItem(
            QString::fromStdString(rec->getStatus()));
        if      (rec->isOverdue())            statusItem->setForeground(Qt::red);
        else if (rec->isActive())             statusItem->setForeground(QColor("#1a7a1a"));
        else                                  statusItem->setForeground(Qt::gray);
        table->setItem(row, 5, statusItem);

        double fine = rec->isActive() ? rec->calculateFine(Date::today())
                                      : rec->getFineAmount();
        auto *fineItem = new QTableWidgetItem("$" + QString::number((int)fine));
        if (fine > 0) fineItem->setForeground(Qt::red);
        table->setItem(row, 6, fineItem);
    }

    auto *countLabel = new QLabel(
        QString("%1 record(s)").arg(records.size()), &dlg);
    countLabel->setStyleSheet("font-weight: bold; padding: 4px;");

    auto *closeBtn = new QPushButton("Close", &dlg);
    auto *layout   = new QVBoxLayout(&dlg);
    layout->addWidget(countLabel);
    layout->addWidget(table);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void AdminWindow::onUpgradeMembership() {
    bool ok;
    int uid = QInputDialog::getInt(this, "Upgrade Membership",
                                   "Enter User ID:", 1, 1, 99999, 1, &ok);
    if (!ok) return;
    QStringList levels = {"Silver", "Gold"};
    QString level = QInputDialog::getItem(this, "Upgrade Membership",
                                          "Select new level:", levels, 0, false, &ok);
    if (!ok) return;
    try {
        User *u = admin.findUser(uid);
        u->upgradeMembership(level.toStdString());
        QMessageBox::information(this, "Success",
                                 "Membership upgraded to " + level + "!");
    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  REPORT SLOTS
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onAdminProfile() {
    QString sep = "─────────────────────────────────────\n";
    QString info;
    info += sep;
    info += "ADMIN PROFILE\n";
    info += sep;
    info += QString("ID            : %1\n").arg(admin.getAdminID());
    info += QString("Name          : %1\n").arg(
        QString::fromStdString(admin.getAdminName()));
    info += QString("Role          : %1\n").arg(
        QString::fromStdString(admin.getRole()));
    info += QString("Access Level  : %1 / 5\n").arg(admin.getAccessLevel());
    info += sep;
    info += QString("Catalog Size  : %1 book(s)\n").arg(admin.getResources().size());
    info += QString("Registered Users: %1\n").arg(admin.getUsers().size());

    QMessageBox::information(this, "Admin Profile", info);
    Logger::log("Admin viewed own profile.", Logger::ACTION);
}

void AdminWindow::onOverdueReport() {
    OverdueDialog dlg(admin, this);
    dlg.exec();
}
void AdminWindow::onHoldsReport() {
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Active Holds Report");
    dlg->setMinimumSize(700, 400);

    QTableWidget* tbl = new QTableWidget(dlg);
    tbl->setColumnCount(5);
    tbl->setHorizontalHeaderLabels({"Hold#", "User", "Book", "Queue Pos", "Status"});
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setAlternatingRowColors(true);

    for (auto& u : admin.getUsers()) {
        for (auto* h : u->getHoldList()) {
            if (h->getStatus() != "pending" && h->getStatus() != "approved") continue;

            int row = tbl->rowCount();
            tbl->insertRow(row);

            tbl->setItem(row, 0, new QTableWidgetItem(QString::number(h->getHoldID())));
            tbl->setItem(row, 1, new QTableWidgetItem(
                                     QString::fromStdString(u->getFirstName() + " " + u->getLastName())));

            QString bookTitle = "Book#" + QString::number(h->getResourceID());
            try {
                Resource* r = admin.findBook(h->getResourceID());
                bookTitle = QString::fromStdString(r->getTitle());
            } catch (...) {}
            tbl->setItem(row, 2, new QTableWidgetItem(bookTitle));
            tbl->setItem(row, 3, new QTableWidgetItem(QString::number(h->getQueuePosition())));

            QString status = QString::fromStdString(h->getStatus());
            QTableWidgetItem* si = new QTableWidgetItem(status);
            si->setForeground(h->getStatus() == "approved" ? Qt::darkGreen : Qt::darkYellow);
            tbl->setItem(row, 4, si);
        }
    }

    if (tbl->rowCount() == 0) {
        QMessageBox::information(this, "Holds Report", "No active holds.");
        delete dlg;
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(dlg);
    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(btns, &QDialogButtonBox::rejected, dlg, &QDialog::accept);
    layout->addWidget(tbl);
    layout->addWidget(btns);
    dlg->exec();
}
void AdminWindow::onCustomerReport() {
    QDialog dlg(this);
    dlg.setWindowTitle("Customer Report");
    dlg.setMinimumSize(860, 460);

    auto *table = new QTableWidget(&dlg);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels(
        {"ID", "Full Name", "Username", "Membership",
         "Borrows", "Balance ($)", "Fine Due ($)", "Total Borrowed"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);

    double totalFines = 0.0;
    for (auto &u : admin.getUsers()) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::number(u->getUserID())));
        table->setItem(row, 1, new QTableWidgetItem(
                                   QString::fromStdString(u->getFirstName() + " " + u->getLastName())));
        table->setItem(row, 2, new QTableWidgetItem(
                                   QString::fromStdString(u->getUsername())));
        table->setItem(row, 3, new QTableWidgetItem(
                                   QString::fromStdString(u->getMembership()->getLevel())));
        table->setItem(row, 4, new QTableWidgetItem(
                                   QString("%1/%2").arg(u->getCurrentBorrows())
                                       .arg(u->getMembership()->getMaxBooks())));
        table->setItem(row, 5, new QTableWidgetItem(
                                   QString::number((int)u->getAccount()->getBalance())));
        double fine = u->getAccount()->getFinesDue();
        totalFines += fine;
        auto *fineItem = new QTableWidgetItem(QString::number((int)fine));
        if (fine > 0) fineItem->setForeground(Qt::red);
        table->setItem(row, 6, fineItem);
        table->setItem(row, 7, new QTableWidgetItem(
                                   QString::number(u->getAccount()->getTotalBorrowed())));
    }

    auto *summaryLabel = new QLabel(
        QString("Users: %1   |   Total Outstanding Fines: $%2   |   %3")
            .arg(admin.getUsers().size())
            .arg((int)totalFines)
            .arg(QString::fromStdString(Date::today().toString())), &dlg);
    summaryLabel->setStyleSheet("font-weight: bold; padding: 4px;");

    auto *closeBtn = new QPushButton("Close", &dlg);
    auto *layout   = new QVBoxLayout(&dlg);
    layout->addWidget(summaryLabel);
    layout->addWidget(table);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    Logger::log("Admin viewed customer report.", Logger::ACTION);
    dlg.exec();
}

void AdminWindow::onCurrentlyIssued() {
    QDialog dlg(this);
    dlg.setWindowTitle("Currently Issued Books");
    dlg.setMinimumSize(820, 440);

    auto *table = new QTableWidget(&dlg);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(
        {"User ID", "User Name", "Rec #", "Book ID",
         "Issue Date", "Due Date", "Days Left"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);

    for (auto &u : admin.getUsers()) {
        for (auto *rec : u->getBorrowHistory()) {
            if (!rec->isActive()) continue;
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(u->getUserID())));
            table->setItem(row, 1, new QTableWidgetItem(
                                       QString::fromStdString(u->getFirstName() + " " + u->getLastName())));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(rec->getRecordID())));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(rec->getResourceID())));
            table->setItem(row, 4, new QTableWidgetItem(
                                       QString::fromStdString(rec->getIssueDate().toString())));
            table->setItem(row, 5, new QTableWidgetItem(
                                       QString::fromStdString(rec->getDueDate().toString())));
            int daysLeft = rec->getDaysRemaining();
            auto *daysItem = new QTableWidgetItem(
                daysLeft >= 0
                    ? QString::number(daysLeft) + " days"
                    : QString("OVERDUE %1d").arg(-daysLeft));
            if      (daysLeft < 0)  daysItem->setForeground(Qt::red);
            else if (daysLeft <= 3) daysItem->setForeground(QColor("#b8860b"));
            else                    daysItem->setForeground(QColor("#1a7a1a"));
            table->setItem(row, 6, daysItem);
        }
    }

    auto *summaryLabel = new QLabel(
        table->rowCount() == 0
            ? "No books currently issued."
            : QString("%1 book(s) currently out — Generated: %2")
                  .arg(table->rowCount())
                  .arg(QString::fromStdString(Date::today().toString())),
        &dlg);
    summaryLabel->setStyleSheet("font-weight: bold; padding: 4px;");

    auto *closeBtn = new QPushButton("Close", &dlg);
    auto *layout   = new QVBoxLayout(&dlg);
    layout->addWidget(summaryLabel);
    layout->addWidget(table);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    Logger::log("Admin viewed currently-issued report.", Logger::ACTION);
    dlg.exec();
}

// ══════════════════════════════════════════════════════════════════════════════
//  SYSTEM
// ══════════════════════════════════════════════════════════════════════════════

void AdminWindow::onSaveLogout() {
    admin.saveAll();
    Logger::log("Admin logged out.", Logger::ACTION);
    QMessageBox::information(this, "Saved", "All data saved. Logging out.");
    close();
    if (parentWidget()) parentWidget()->show();
}
