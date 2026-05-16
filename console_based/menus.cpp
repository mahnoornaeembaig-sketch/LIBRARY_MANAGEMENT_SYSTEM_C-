#include "menus.h"
#include <cstdlib>
#include "admin.h"

/* ============================================================
  INPUT HELPERS
 ============================================================ */

int readInt(const string& prompt) {
    int val;
    cout << Color::CYAN << prompt << Color::RESET;
    if (!(cin >> val)) {
        cin.clear(); cin.ignore(1000, '\n');
        throw InvalidInputException();
    }
    return val;
}

string readLine(const string& prompt) {
    string val;
    cin.ignore();
    cout << Color::CYAN << prompt << Color::RESET;
    getline(cin, val);
    return val;
}

/* ============================================================
  ADMIN MENU DISPLAY
============================================================ */
void showAdminMenuOptions(const string& adminName) {
    cout << "\n";
    printDoubleLine(54);
    cout << Color::BG_BLUE << Color::BWHITE << Color::BOLD
         << "          ADMIN PANEL  " << adminName << "          "
         << Color::RESET << "\n";
    printDoubleLine(54);

    cout << Color::BCYAN << "  CATALOG\n" << Color::RESET;
    cout << Color::BWHITE << "   1" << Color::RESET << Color::DIM << "  Add Book\n"              << Color::RESET;
    cout << Color::BWHITE << "   2" << Color::RESET << Color::DIM << "  All Books\n"             << Color::RESET;
    cout << Color::BWHITE << "   3" << Color::RESET << Color::DIM << "  Available Books\n"       << Color::RESET;
    cout << Color::BWHITE << "   4" << Color::RESET << Color::DIM << "  Search (title/author)\n" << Color::RESET;
    cout << Color::BWHITE << "   5" << Color::RESET << Color::DIM << "  Search by Author\n"      << Color::RESET;
    cout << Color::BWHITE << "   6" << Color::RESET << Color::DIM << "  Search by Genre\n"       << Color::RESET;
    cout << Color::BWHITE << "   7" << Color::RESET << Color::DIM << "  Search by Year\n"        << Color::RESET;
    cout << Color::BWHITE << "   8" << Color::RESET << Color::DIM << "  Update Book\n"           << Color::RESET;
    cout << Color::BWHITE << "   9" << Color::RESET << Color::DIM << "  Remove Book\n"           << Color::RESET;

    cout << Color::BCYAN  << "  USERS\n" << Color::RESET;
    cout << Color::BWHITE << "  10" << Color::RESET << Color::DIM << "  Add User\n"              << Color::RESET;
    cout << Color::BWHITE << "  11" << Color::RESET << Color::DIM << "  All Users\n"             << Color::RESET;
    cout << Color::BWHITE << "  12" << Color::RESET << Color::DIM << "  Issue Book\n"            << Color::RESET;
    cout << Color::BWHITE << "  13" << Color::RESET << Color::DIM << "  Return Book\n"           << Color::RESET;
    cout << Color::BWHITE << "  14" << Color::RESET << Color::DIM << "  Place Hold\n"            << Color::RESET;
    cout << Color::BWHITE << "  15" << Color::RESET << Color::DIM << "  View User Profile\n"     << Color::RESET;
    cout << Color::BWHITE << "  16" << Color::RESET << Color::DIM << "  View Borrow History\n"   << Color::RESET;
    cout << Color::BWHITE << "  17" << Color::RESET << Color::DIM << "  Upgrade Membership\n"    << Color::RESET;
    cout << Color::BCYAN  << "  REPORTS\n" << Color::RESET;
    cout << Color::BWHITE << "  18" << Color::RESET << Color::DIM << "  Admin Profile\n"         << Color::RESET;
    cout << Color::BWHITE << "  19" << Color::RESET << Color::DIM << "  Overdue Report\n"        << Color::RESET;
    cout << Color::BWHITE << "  20" << Color::RESET << Color::DIM << "  Customer Report\n"       << Color::RESET;
    cout << Color::BWHITE << "  21" << Color::RESET << Color::DIM << "  Currently Issued\n"      << Color::RESET;

    cout << Color::BRED   << "  22  Save & Logout\n" << Color::RESET;
    printLine('=', 54);
    cout << Color::BYELLOW << "  Choice: " << Color::RESET;
}

/* ============================================================
  ADMIN HANDLERS
============================================================ */

void handleAddBook(Administrator& admin) {
    cin.ignore();
    admin.addBook();
}

void handleSearchBooks(Administrator& admin) {
    string kw = readLine("Keyword: ");
    admin.searchBooks(kw);
}

void handleSearchByAuthor(Administrator& admin) {
    string a = readLine("Author: ");
    admin.searchByAuthor(a);
}

void handleSearchByGenre(Administrator& admin) {
    string g = readLine("Genre: ");
    admin.searchByGenre(g);
}

void handleSearchByYear(Administrator& admin) {
    int yr = readInt("Year: ");
    admin.searchByYear(yr);
}

void handleUpdateBook(Administrator& admin) {
    int id = readInt("Book ID to update: ");
    cin.ignore();
    admin.updateResource(id);
}

void handleRemoveBook(Administrator& admin) {
    int id = readInt("Book ID to remove: ");
    admin.removeResource(id);
}

void handleIssueBook(Administrator& admin) {
    int uid = readInt("User ID: ");
    int bid = readInt("Book ID: ");
    admin.issueBook(uid, bid);
}

void handleReturnBookAdmin(Administrator& admin) {
    int uid = readInt("User ID: ");
    int bid = readInt("Book ID: ");
    admin.returnBook(uid, bid);
}

void handlePlaceHoldAdmin(Administrator& admin) {
    int uid = readInt("User ID: ");
    int bid = readInt("Book ID: ");
    admin.placeHold(uid, bid);
}

void handleViewUserProfile(Administrator& admin) {
    int uid = readInt("User ID: ");
    admin.findUser(uid)->show();
}

void handleViewBorrowHistory(Administrator& admin) {
    int uid = readInt("User ID: ");
    admin.findUser(uid)->viewBorrowHistory();
}

/* ============================================================
  ADMIN MENU LOOP
============================================================ */
void adminMenu(Administrator& admin) {
    int ch = 0;
    do {
        showAdminMenuOptions(admin.getAdminName());
        if (!(cin >> ch)) { cin.clear(); cin.ignore(1000, '\n'); ch = -1; }

        try {
            switch (ch) {
                case  1: handleAddBook(admin);               break;
                case  2: admin.showAllBooks();               break;
                case  3: admin.showAvailableBooks();         break;
                case  4: handleSearchBooks(admin);           break;
                case  5: handleSearchByAuthor(admin);        break;
                case  6: handleSearchByGenre(admin);         break;
                case  7: handleSearchByYear(admin);          break;
                case  8: handleUpdateBook(admin);            break;
                case  9: handleRemoveBook(admin);            break;
                case 10: cin.ignore(); admin.addUser();      break;
                case 11: admin.showAllUsers();               break;
                case 12: handleIssueBook(admin);             break;
                case 13: handleReturnBookAdmin(admin);       break;
                case 14: handlePlaceHoldAdmin(admin);        break;
                case 15: handleViewUserProfile(admin);       break;
                case 16: handleViewBorrowHistory(admin);     break;
                case 17: {
                    int uid = readInt("User ID: ");
                    admin.upgradeMembership(uid);
                } break;
                case 18: admin.showProfile();                break;
                case 19: admin.printOverdueReport();         break;
                case 20: admin.printCustomerReport();        break;
                case 21: admin.printCurrentlyIssuedReport(); break;
                case 22:
                    admin.saveAll();
                    printSuccess("Data saved. Logging out.");
                    Logger::log("Admin logged out.", Logger::ACTION);
                    break;
                default: printError("Invalid choice. Enter 1-22."); break;
            }
        }
        catch (exception& e) { printError(e.what()); }

    } while (ch != 22);
}

/* ============================================================
  USER MENU DISPLAY
============================================================ */
void showUserMenuOptions(User* user) {
    cout << "\n";
    printDoubleLine(54);
    cout << Color::BG_GREEN << Color::BWHITE << Color::BOLD
         << "   USER PANEL  " << user->getFirstName()
         << " " << user->getLastName() << "   "
         << Color::RESET << "\n";
    printDoubleLine(54);

    cout << Color::BWHITE << "   1" << Color::RESET << Color::DIM << "  My Profile\n"              << Color::RESET;
    cout << Color::BWHITE << "   2" << Color::RESET << Color::DIM << "  Browse Available Books\n"  << Color::RESET;
    cout << Color::BWHITE << "   3" << Color::RESET << Color::DIM << "  Borrow Book\n"             << Color::RESET;
    cout << Color::BWHITE << "   4" << Color::RESET << Color::DIM << "  Return Book\n"             << Color::RESET;
    cout << Color::BWHITE << "   5" << Color::RESET << Color::DIM << "  Place Hold\n"              << Color::RESET;
    cout << Color::BWHITE << "   6" << Color::RESET << Color::DIM << "  Borrow History\n"          << Color::RESET;
    cout << Color::BWHITE << "   7" << Color::RESET << Color::DIM << "  Currently Borrowed\n"      << Color::RESET;
    cout << Color::BWHITE << "   8" << Color::RESET << Color::DIM << "  My Holds\n"                << Color::RESET;
    cout << Color::BWHITE << "   9" << Color::RESET << Color::DIM << "  Pay Fine\n"                << Color::RESET;
    cout << Color::BWHITE << "  10" << Color::RESET << Color::DIM << "  Deposit Balance\n"         << Color::RESET;
    cout << Color::BWHITE << "  11" << Color::RESET << Color::DIM << "  Account Statement\n"       << Color::RESET;
    cout << Color::BWHITE << "  12" << Color::RESET << Color::DIM << "  Search Books\n"            << Color::RESET;
    cout << Color::BWHITE << "  13" << Color::RESET << Color::DIM << "  Update Profile\n"          << Color::RESET;
    cout << Color::BRED   << "  14  Logout\n" << Color::RESET;
    printLine('=', 54);
    cout << Color::BYELLOW << "  Choice: " << Color::RESET;
}

/* ============================================================
  USER HANDLERS
============================================================ */

void handleBorrowBook(User* user, Administrator& admin) {
    int bid = readInt("Book ID: ");
    user->borrowBook(admin.findBook(bid));
}

void handleReturnBookUser(User* user, Administrator& admin) {
    int bid = readInt("Book ID: ");
    admin.returnBook(user->getUserID(), bid); // single path, holds notified internally
}

void handlePlaceHoldUser(User* user, Administrator& admin) {
    int bid = readInt("Book ID: ");
    user->placeHold(admin.findBook(bid));
}

void handlePayFine(User* user) {
    double amt = 0;
    cout << Color::CYAN << "  Amount ($): " << Color::RESET;
    if (!(cin >> amt)) {
        cin.clear(); cin.ignore(1000, '\n');
        throw InvalidInputException();
    }
    user->payFine(amt);
}

void handleSearchBooksUser(Administrator& admin) {
    string kw = readLine("Keyword: ");
    admin.searchBooks(kw);
}

/* ============================================================
  USER MENU LOOP
 ============================================================*/
void userMenu(User* user, Administrator& admin) {
    int ch = 0;
    do {
        showUserMenuOptions(user);
        if (!(cin >> ch)) { cin.clear(); cin.ignore(1000, '\n'); ch = -1; }

        try {
            switch (ch) {
                case  1: user->show();                               break;
                case  2: admin.showAvailableBooks();                 break;
                case  3: handleBorrowBook(user, admin);              break;
                case  4: handleReturnBookUser(user, admin);          break;
                case  5: handlePlaceHoldUser(user, admin);           break;
                case  6: user->viewBorrowHistory();                  break;
                case  7: user->viewCurrentlyBorrowed();              break;
                case  8: user->viewHolds();                          break;
                case  9: handlePayFine(user);                        break;
                case 10: user->depositBalance();                     break;
                case 11: user->viewAccountStatement();               break;
                case 12: handleSearchBooksUser(admin);               break;
                case 13: cin.ignore(); user->updateProfile();        break;
                case 14: user->logout();                             break;
                default: printError("Invalid choice. Enter 1-14."); break;
            }
        }
        catch (exception& e) { printError(e.what()); }

    } while (ch != 14);
}