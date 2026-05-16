#include "admin.h"
#include "menus.h"
#include "date.h"
#include "user.h"
#include "resource.h"
#include "records.h"
#include "globals.h"
#include "logger.h"
#include <cstdlib>
#include <iostream>
using namespace std;

// Static member defined here
int User::nextRecordID = 0;
int User::nextHoldID   = 0;

/* ============================================================
  MAIN
   ============================================================*/

int main() {
    Administrator admin;
    admin.loadAll();
    Logger::log("=== System started ===", Logger::INFO);

    int choice = 0;
    do {
        printDoubleLine(54);
        cout << Color::BG_MAG << Color::BWHITE << Color::BOLD
             << "        LIBRARY MANAGEMENT SYSTEM         "
             << Color::RESET << "\n";
        printDoubleLine(54);
        cout << Color::BWHITE << "   1" << Color::RESET
             << Color::CYAN     << "  Login\n"                << Color::RESET;
        cout << Color::BWHITE << "   2" << Color::RESET
             << Color::BMAGENTA << "  Register New Account\n" << Color::RESET;
        cout << Color::BWHITE << "   3" << Color::RESET
             << Color::RED      << "  Exit\n"                 << Color::RESET;
        printLine('=', 54);
        cout << Color::BYELLOW << "  Choice: " << Color::RESET;
        if (!(cin >> choice)) { cin.clear(); cin.ignore(1000, '\n'); choice = -1; }

        if (choice == 1) {
            string uname, pass;

            //  Username Validation 
            try {
                cout << Color::CYAN << "  Username: " << Color::RESET;
                cin >> uname;

                if (uname.empty())
                    throw InvalidInputException();

                bool allDigits = true;
                for (char c : uname)
                    if (!isdigit(c)) { allDigits = false; break; }
                if (allDigits)
                    throw InvalidInputException();

                bool hasLetter = false;
                for (char c : uname)
                    if (isalpha(c)) { hasLetter = true; break; }
                if (!hasLetter)
                    throw InvalidInputException();

                for (char c : uname)
                    if (isspace(c)) throw InvalidInputException();
            }
            catch (InvalidInputException&) {
                printError("Invalid username!");
                printInfo("Username must:");
                cout << Color::DIM
                     << "    - Contain at least one letter\n"
                     << "    - Not be all numbers\n"
                     << "    - Have no spaces\n"
                     << Color::RESET;
                Logger::log("Invalid username entered at login.", Logger::WARNING);
                continue;
            }

            //  Password Validation 
            try {
                cout << Color::CYAN << "  Password: " << Color::RESET;
                cin >> pass;

                if (pass.empty())
                    throw InvalidInputException();
                if (pass.length() < 6)
                    throw InvalidInputException();

                bool hasLetter = false, hasDigit = false;
                for (char c : pass) {
                    if (isalpha(c)) hasLetter = true;
                    if (isdigit(c)) hasDigit  = true;
                }
                if (!hasLetter || !hasDigit)
                    throw InvalidInputException();
            }
            catch (InvalidInputException&) {
                printError("Invalid password!");
                printInfo("Password must:");
                cout << Color::DIM
                     << "    - Be at least 6 characters long\n"
                     << "    - Contain at least one letter\n"
                     << "    - Contain at least one number\n"
                     << Color::RESET;
                Logger::log("Invalid password entered at login.", Logger::WARNING);
                continue;
            }

            //  Login Check 
            try {
                if (uname == admin.getAdminName() && admin.verifyCredentials(pass)) {
                    printSuccess("Admin login successful.");
                    Logger::log("Admin logged in.", Logger::ACTION);
                    adminMenu(admin);
                    // ── Clear screen after admin session ends ──
                    #ifdef _WIN32
                        system("cls");
                    #else
                        system("clear");
                    #endif
                }
                else {
                    User* u = admin.findUserByUsername(uname); // throws NotFoundException
                    if (u->verifyPassword(pass)) {
                        printSuccess("Welcome, " + u->getFirstName() + "!");
                        Logger::logUser(u->getUserID(), "Logged in.");
                        userMenu(u, admin);
                        // ── Clear screen after user session ends ──
                        #ifdef _WIN32
                            system("cls");
                        #else
                            system("clear");
                        #endif
                    } else {
                        printError("Incorrect password. Please try again.");
                        Logger::log("Failed login for: " + uname, Logger::WARNING);
                    }
                }
            }
            catch (NotFoundException&) {
                printError("Account not found.");
                printInfo("Please register first using option 2.");
                Logger::log("Login attempt for unknown user: " + uname, Logger::WARNING);
            }
            catch (exception& e) {
                printError(e.what());
            }
        }
        else if (choice == 2) {
            cin.ignore();
            admin.addUser();
            Logger::log("Self-registration from main menu.", Logger::ACTION);
        }
        else if (choice == 3) {
            admin.saveAll();
            Logger::log("=== System exited cleanly ===", Logger::INFO);
            printSuccess("All data saved. Goodbye!");
        }
        else {
            printError("Invalid choice. Enter 1-3.");
        }

    } while (choice != 3);

    return 0;
}