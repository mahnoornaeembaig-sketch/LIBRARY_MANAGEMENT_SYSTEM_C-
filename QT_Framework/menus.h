#ifndef MENUS_H
#define MENUS_H

#include "admin.h"

// ============================================================
//  INPUT HELPERS
//  Centralise the repeated cin-check + error pattern so
//  every menu case stays to one clean line.
// ============================================================
int    readInt (const string& prompt);
string readLine(const string& prompt);

// ============================================================
//  ADMIN MENU HANDLERS
//  One function per menu action keeps adminMenu() readable.
// ============================================================
void showAdminMenuOptions(const string& adminName);

void handleAddBook          (Administrator& admin);
void handleSearchBooks      (Administrator& admin);
void handleSearchByAuthor   (Administrator& admin);
void handleSearchByGenre    (Administrator& admin);
void handleSearchByYear     (Administrator& admin);
void handleUpdateBook       (Administrator& admin);
void handleRemoveBook       (Administrator& admin);
void handleIssueBook        (Administrator& admin);
void handleReturnBookAdmin  (Administrator& admin);
void handlePlaceHoldAdmin   (Administrator& admin);
void handleViewUserProfile  (Administrator& admin);
void handleViewBorrowHistory(Administrator& admin);

// ============================================================
//  USER MENU HANDLERS
// ============================================================
void showUserMenuOptions(User* user);

void handleBorrowBook    (User* user, Administrator& admin);
void handleReturnBookUser(User* user, Administrator& admin);
void handlePlaceHoldUser (User* user, Administrator& admin);
void handlePayFine       (User* user);
void handleSearchBooksUser(Administrator& admin);

// ============================================================
//  MAIN MENU LOOPS
// ============================================================
void adminMenu(Administrator& admin);
void userMenu (User* user, Administrator& admin);

#endif // MENUS_H
