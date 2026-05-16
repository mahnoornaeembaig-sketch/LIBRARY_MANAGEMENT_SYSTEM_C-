#ifndef USERWINDOW_H
#define USERWINDOW_H
#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include "user.h"
#include "admin.h"

class UserWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit UserWindow(User* user, Administrator& admin, QWidget* parent = nullptr);

signals:
    void sig_logout();

private slots:
    // ── already existed ──
    void onBorrowBook();
    void onReturnBook();
    void onPlaceHold();
    void onPayFine();
    void onDeposit();
    void onViewHistory();
    void onLogout();
    void onUpgradeMembership();

    // ── newly added ──
    void onMyProfile();
    void onBrowseBooks();
    void onCurrentlyBorrowed();
    void onMyHolds();
    void onAccountStatement();
    void onSearchBooks();
    void onUpdateProfile();

private:
    User*          currentUser;
    Administrator& admin;
    QTableWidget*  borrowTable;
    QLabel*        balanceLabel;
    QLabel*        fineLabel;
    QLabel*        borrowCountLabel;
    QLabel*        membershipLabel;       // ← new: shows tier in sidebar

    void setupUI();
    void refreshBorrowTable();
    void refreshAccountInfo();
    void showBookTable(const QString& title,
                       const vector<Resource*>& books); // ← shared helper
};
#endif
