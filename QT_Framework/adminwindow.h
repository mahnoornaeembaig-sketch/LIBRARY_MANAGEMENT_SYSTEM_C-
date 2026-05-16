#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <functional>
#include "admin.h"

class AdminWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AdminWindow(Administrator &admin, QWidget *parent = nullptr);

private slots:
    // Catalog
    void onAllBooks();
    void onAvailableBooks();
    void onAddBook();
    void onUpdateBook();
    void onRemoveBook();
    // Search
    void onSearchBooks();
    void onSearchByAuthor();
    void onSearchByGenre();
    void onSearchByYear();
    // Operations
    void onIssueBook();
    void onReturnBook();
    void onPlaceHold();
    // Users
    void onAddUser();
    void onShowUsers();
    void onViewUserProfile();
    void onViewBorrowHistory();
    void onUpgradeMembership();
    // Reports
    void onAdminProfile();
    void onOverdueReport();
    void onCustomerReport();
    void onCurrentlyIssued();
    void onHoldsReport();
    // System
    void onSaveLogout();

private:
    Administrator &admin;
    QTableWidget  *bookTable;

    void setupUI();
    void refreshBookTable();
    void showBookResultsDialog(const QString &title,
                               std::function<bool(Resource*)> filter);
};

#endif // ADMINWINDOW_H
