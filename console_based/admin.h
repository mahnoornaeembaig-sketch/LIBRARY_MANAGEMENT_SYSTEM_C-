#ifndef ADMIN_H
#define ADMIN_H

#include <vector>
#include <memory>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "globals.h"
#include "date.h"
#include "logger.h"
#include "resource.h"
#include "user.h"
using namespace std;

/*============================================================
  ADMINISTRATOR
  Manages the resource catalog and user list.
  Provides all admin-level operations: CRUD on books/users,
  issuing/returning, holds, and report generation.
============================================================*/
class Administrator {
private:
    int    adminID;
    string adminName;
    string password;
    int    accessLevel;
    string role;
    string phoneNo;
    string dateJoined;
    string shiftTiming;

    vector<unique_ptr<Resource>> resources;
    vector<unique_ptr<User>>     users;

 /*borrowCopy() happens exactly once: inside User::borrowBook()
     at actual borrow time. Pre-decrementing here caused the holds to see the book as unavailable and skip notifying the next user in line.*/
    void checkAndNotifyHolds(int resourceID) {
        HoldRecord* earliest = nullptr;
        for (auto& u : users) {
            HoldRecord* h = u->getActiveHoldForResource(resourceID);
            if (h && (!earliest || h->getQueuePosition() < earliest->getQueuePosition()))
                earliest = h;
        }
        if (earliest) {
            earliest->markApproved();
            earliest->notifyUser(); // The copy is decremented when the user actually borrows.
        }
    }

public:
    Administrator(int id = 1,
                  const string& name   = "Admin",
                  const string& pass   = "admin123",
                  int access           = 5,
                  const string& r      = "Library Manager",
                  const string& phone  = "0300-0000000",
                  const string& joined = "",
                  const string& shift  = "09:00-17:00")
        : adminID(id), adminName(name), password(pass),
          accessLevel(access), role(r), phoneNo(phone),
          shiftTiming(shift)
    {
        dateJoined = joined.empty() ? Date::today().toString() : joined;
    }

    ~Administrator() = default;

    bool verifyCredentials(const string& pass) const { return password == pass; }

    // Book Management 

    void addBook() {
        try {
            auto b = make_unique<Book>();
            b->input();
            for (auto& r : resources) {
                Book* existing = dynamic_cast<Book*>(r.get());
                if (existing && *existing == *b) {
                    printError("A book with this ISBN already exists!");
                    Logger::log("Duplicate ISBN attempted: " + b->getISBN(), Logger::WARNING);
                    return;
                }
            }
            string t = b->getTitle();
            resources.push_back(move(b));
            printSuccess("Book added: " + t);
            Logger::log("Admin added: " + t, Logger::ACTION);
        }
        catch (exception& e) { printError(e.what()); }
    }

    void updateResource(int id) {
        try {
            Resource* r = findBook(id);
            printInfo("Re-enter updated details for resource #" + to_string(id));
            r->input();
            printSuccess("Resource updated.");
            Logger::log("Admin updated resource ID=" + to_string(id), Logger::ACTION);
        }
        catch (exception& e) { printError(e.what()); }
    }

    void removeResource(int id) {
        for (auto it = resources.begin(); it != resources.end(); ++it) {
            if ((*it)->getResourceID() == id) {
                for (auto& u : users) {
                    if (u->hasActiveBorrowForResource(id)) {
                        printError("Cannot remove: copies are currently borrowed!");
                        return;
                    }
                }
                string t = (*it)->getTitle();
                resources.erase(it);
                printSuccess("Resource '" + t + "' removed.");
                Logger::log("Admin removed: " + t, Logger::ACTION);
                return;
            }
        }
        throw NotFoundException();
    }

    void showAvailableBooks() const {
        bool found = false;
        printHeader("AVAILABLE BOOKS");
        for (auto& r : resources)
            if (r->checkAvailability()) { r->display(); found = true; }
        if (!found) printInfo("No books currently available.");
    }

    void showAllBooks() const {
        if (resources.empty()) { printInfo("No books in catalog."); return; }
        printHeader("ALL BOOKS IN CATALOG");
        cout << Color::DIM << "  Total: " << resources.size()
             << " book(s)\n" << Color::RESET;
        for (auto& r : resources) r->display();
    }

    Resource* findBook(int id) {
        for (auto& r : resources)
            if (r->getResourceID() == id) return r.get();
        throw NotFoundException();
    }

    void searchBooks(const string& keyword) const {
        string kw = toLower(keyword);
        bool found = false;
        printHeader("SEARCH RESULTS");
        for (auto& r : resources) {
            if (toLower(r->getTitle()).find(kw)  != string::npos ||
                toLower(r->getAuthor()).find(kw) != string::npos) {
                r->display(); found = true;
            }
        }
        if (!found) printInfo("No books found for: '" + keyword + "'");
    }

    void searchByAuthor(const string& author) const {
        string kw = toLower(author);
        bool found = false;
        printHeader("SEARCH BY AUTHOR");
        for (auto& r : resources)
            if (toLower(r->getAuthor()).find(kw) != string::npos) {
                r->display(); found = true;
            }
        if (!found) printInfo("No books by author: '" + author + "'");
    }

    void searchByGenre(const string& genre) const {
        string kw = toLower(genre);
        bool found = false;
        printHeader("SEARCH BY GENRE");
        for (auto& r : resources) {
            string g = toLower(r->getGenre());
            if (!g.empty() && g.find(kw) != string::npos) {
                r->display(); found = true;
            }
        }
        if (!found) printInfo("No books in genre: '" + genre + "'");
    }

    void searchByYear(int year) const {
        bool found = false;
        printHeader("SEARCH BY YEAR");
        for (auto& r : resources)
            if (r->getPublishYear() == year) { r->display(); found = true; }
        if (!found) printInfo("No books for year: " + to_string(year));
    }

    // User Management 
    void addUser() {
        int id;
        string fname, lname, uname, pass, addr, mail;
        double initialBalance = 0.0;
        printHeader("REGISTER NEW USER");

        while (true) {
            cout << Color::CYAN << "  User ID       : " << Color::RESET;
            if (cin >> id && id > 0) { cin.ignore(); break; }
            printError("Invalid ID! Must be a positive number.");
            cin.clear(); cin.ignore(1000, '\n');
        }
        for (auto& u : users) {
            if (u->getUserID() == id) { printError("User ID already exists!"); return; }
        }

        while (true) {
            cout << Color::CYAN << "  First Name    : " << Color::RESET;
            getline(cin, fname);
            if (fname.empty() || !isValidName(fname)) {
                printError("Invalid first name! Letters only."); continue;
            }
            if (fname.length() > 20) {
                printError("First name too long! Maximum 20 characters."); continue;
            }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Last Name     : " << Color::RESET;
            getline(cin, lname);
            if (lname.empty() || !isValidName(lname)) {
                printError("Invalid last name! Letters only."); continue;
            }
            if (lname.length() > 20) {
                printError("Last name too long! Maximum 20 characters."); continue;
            }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Username      : " << Color::RESET;
            getline(cin, uname);
            if (!isValidUsername(uname)) {
                printError("Invalid username!");
                printInfo("Username must be 3-15 characters with at least one letter and no spaces.");
                continue;
            }
            bool exists = false;
            for (auto& u : users)
                if (u->getUsername() == uname) { exists = true; break; }
            if (exists) { printError("Username already taken!"); continue; }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Password      : " << Color::RESET;
            getline(cin, pass);
            if (pass.length() > 20) {
                printError("Password too long! Maximum 20 characters."); continue;
            }
            if (!isValidPassword(pass)) {
                printError("Weak password!");
                printInfo("Password must be 6-20 characters with letters & numbers.");
                continue;
            }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Address       : " << Color::RESET;
            getline(cin, addr);
            if (addr.empty()) { printError("Address cannot be empty!"); continue; }
            if (addr.length() < 5) {
                printError("Address too short! Must be at least 5 characters."); continue;
            }
            bool allSpaces = true;
            for (char c : addr)
                if (!isspace(c)) { allSpaces = false; break; }
            if (allSpaces) { printError("Address cannot be all spaces!"); continue; }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Email         : " << Color::RESET;
            getline(cin, mail);
            if (!isValidEmail(mail)) {
                printError("Invalid email format!");
                printInfo("Example: username@domain.com — no spaces or consecutive dots.");
                continue;
            }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  Initial Balance: " << Color::RESET;
            if (cin >> initialBalance && initialBalance >= 0 && initialBalance <= 10000) {
                cin.ignore(); break;
            }
            printError("Invalid amount! Enter a value between $0 and $10000.");
            cin.clear(); cin.ignore(1000, '\n');
        }

        users.push_back(make_unique<User>(
            id, fname, lname, uname, pass, addr, mail, "", initialBalance));
        printSuccess("User '" + uname + "' registered successfully!");
        Logger::log("Admin registered user: " + uname, Logger::ACTION);
    }

    User* findUser(int id) {
        for (auto& u : users)
            if (u->getUserID() == id) return u.get();
        throw NotFoundException();
    }

    User* findUserByUsername(const string& uname) {
        for (auto& u : users)
            if (u->getUsername() == uname) return u.get();
        throw NotFoundException();
    }

    void showAllUsers() const {
        if (users.empty()) { printInfo("No users registered."); return; }
        printHeader("ALL REGISTERED USERS");
        for (auto& u : users) u->show();
    }

    void issueBook(int userID, int bookID) {
        try {
            User*     u = findUser(userID);
            Resource* b = findBook(bookID);
            u->borrowBook(b);
        }
        catch (exception& e) { printError(e.what()); }
    }

    void returnBook(int userID, int bookID) {
        try {
            User*     u = findUser(userID);
            Resource* b = findBook(bookID);
            u->returnBook(b);
            checkAndNotifyHolds(bookID);
        }
        catch (exception& e) { printError(e.what()); }
    }

    void upgradeMembership(int userID) {
        try {
            User* u = findUser(userID);
            printInfo("Current level: " + u->getMembership()->getLevel());
            cout << Color::CYAN << "  New level (Silver/Gold): " << Color::RESET;
            string level;
            cin >> level;
            cin.ignore();
            u->upgradeMembership(level);
        }
        catch (exception& e) { printError(e.what()); }
    }


    void placeHold(int userID, int bookID) {
        try {
            User*     u = findUser(userID);
            Resource* b = findBook(bookID);
            u->placeHold(b);
        }
        catch (exception& e) { printError(e.what()); }
    }

    // Reports
    void printOverdueReport() const {
        printHeader("OVERDUE REPORT");
        bool anyOverdue = false;
        for (auto& u : users) {
            if (u->hasOverdueRecords()) { u->printOverdueRecords(); anyOverdue = true; }
        }
        if (!anyOverdue) printSuccess("No overdue records — all clear!");
        cout << Color::DIM << "  Generated by: " << adminName
             << " on " << Date::today().toString() << "\n" << Color::RESET;
        Logger::log("Admin generated overdue report.", Logger::ACTION);
    }

    void printCustomerReport() const {
        printHeader("CUSTOMER REPORT");
        cout << Color::BYELLOW << "  Total Users : " << users.size()
             << "   Total Books: " << resources.size() << Color::RESET << "\n";
        printLine();
        for (auto& u : users) {
            cout << Color::BCYAN << Color::BOLD
                 << "  " << u->getFirstName() << " " << u->getLastName()
                 << " (ID:" << u->getUserID() << ")" << Color::RESET
                 << "  @" << u->getUsername()
                 << Color::DIM << "  Borrows:" << u->getCurrentBorrows()
                 << "/" << u->getMembership()->getMaxBooks() << Color::RESET << "\n";
            cout << Color::GREEN << "    Balance: $" << (int)u->getAccount()->getBalance()
                 << Color::RESET;
            double fine = u->getAccount()->getFinesDue();
            if (fine > 0)
                cout << Color::RED << "   Fine Due: $" << (int)fine << Color::RESET;
            cout << "\n";
            u->viewBorrowHistory();
        }
        cout << Color::DIM << "  Generated by: " << adminName
             << " on " << Date::today().toString() << "\n" << Color::RESET;
        Logger::log("Admin generated customer report.", Logger::ACTION);
    }

    void printCurrentlyIssuedReport() const {
        printHeader("CURRENTLY ISSUED BOOKS");
        bool found = false;
        for (auto& u : users) {
            if (u->getCurrentBorrows() > 0) {
                cout << Color::BCYAN << "  " << u->getFirstName() << " "
                     << u->getLastName() << " (ID:" << u->getUserID() << ")\n"
                     << Color::RESET;
                u->viewCurrentlyBorrowed();
                found = true;
            }
        }
        if (!found) printInfo("No books currently issued.");
        cout << Color::DIM << "  Generated by: " << adminName
             << " on " << Date::today().toString() << "\n" << Color::RESET;
        Logger::log("Admin generated issued-books report.", Logger::ACTION);
    }

    void showProfile() const {
        printHeader("ADMIN PROFILE");
        printField("ID:",           to_string(adminID));
        printField("Name:",         adminName);
        printField("Role:",         role);
        printField("Access Level:", to_string(accessLevel) + "/5");
        printField("Phone:",        phoneNo);
        printField("Joined:",       dateJoined);
        printField("Shift:",        shiftTiming);
    }

    void updateProfile() {
        cin.ignore();
        cout << Color::CYAN << "  New phone number : " << Color::RESET; getline(cin, phoneNo);
        cout << Color::CYAN << "  New shift timing : " << Color::RESET; getline(cin, shiftTiming);
        printSuccess("Admin profile updated.");
        Logger::log("Admin updated their profile.", Logger::ACTION);
    }

    // ── File I/O ──────────────────────────────────────────────

    void saveBooks() const {
        ofstream file("books.txt");
        for (auto& r : resources) file << r->serialize() << "\n";
        file.close();
        printSuccess("Catalog saved to books.txt");
        Logger::log("Books saved to file.", Logger::INFO);
    }

    void loadBooks() {
        ifstream file("books.txt");
        if (!file) return;
        string line;
        int loaded = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> fields;
            while (getline(ss, token, '|')) fields.push_back(token);
            if ((int)fields.size() < 19) continue;             // 19 fields (0–18)
            try {
                int yr     = stoi(fields[4]);
                int maxCop = stoi(fields[5]);                  // maxCopies (ceiling)
                int avCop  = stoi(fields[6]);                  // totalAvailableCopies
                // Validation: publish year, copies, edition, page count, due days
                int ed     = stoi(fields[8]);                  // edition
                
                int pg     = stoi(fields[11]);                 // pageCount
            
                int due    = stoi(fields[14]);                 // dueDays

                if (yr < 1000 || yr > 2026 || maxCop < 0 || avCop < 0 ||
                    avCop > maxCop || ed <= 0 || ed > 50 ||
                    pg <= 0 || pg > 10000 || due <= 0)
                    throw InvalidInputException();

                const string& isbn = fields[7];
                if (isbn.length() != 10 && isbn.length() != 13)
                    throw InvalidInputException();
                bool allDigits = true;
                for (char c : isbn)
                    if (!isdigit(c)) { allDigits = false; break; }
                if (!allDigits) throw InvalidInputException();

                resources.push_back(make_unique<Book>(
                    stoi(fields[0]), fields[1], fields[2], fields[3],
                    yr, avCop, isbn, ed, fields[9], fields[10],
                    pg, fields[12], fields[13], due, stoi(fields[15]),
                    stoi(fields[16]), stoi(fields[17]), stoi(fields[18])));

                resources.back()->setMaxCopies(maxCop);        // restore true ceiling

                loaded++;
            }
            catch (...) {
                Logger::log("Corrupted book record skipped during load.", Logger::WARNING);
                printWarning("Skipped corrupted record in books.txt");
            }
        }
        file.close();
        if (loaded > 0)
            printSuccess(to_string(loaded) + " book(s) loaded from books.txt");
    }

    void saveUsers() const {
        ofstream file("users.txt");
        for (auto& u : users) file << u->serialize() << "\n";
        file.close();

        ofstream bfile("borrows.txt");
        for (auto& u : users) u->saveBorrowRecords(bfile);
        bfile.close();

        printSuccess("Users saved to users.txt");
        printSuccess("Borrow records saved to borrows.txt");
        Logger::log("Users and borrow records saved.", Logger::INFO);
    }

    void loadUsers() {
        ifstream file("users.txt");
        if (!file) return;
        string line;
        int loaded = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> fields;
            while (getline(ss, token, '|')) fields.push_back(token);
            if ((int)fields.size() < 7) continue;
            try {
                double balance = fields.size() > 8  ? stod(fields[8])  : 0.0;
                double fineDue = fields.size() > 9  ? stod(fields[9])  : 0.0;
                auto u = make_unique<User>(
                    stoi(fields[0]), fields[1], fields[2],
                    fields[3], fields[4], fields[5], fields[6],
                    fields.size() > 7 ? fields[7] : "",
                    balance);
                u->getAccount()->setFineDue(fineDue);
                if (fields.size() > 10 && !fields[10].empty())
                    u->getMembership()->loadLevel(fields[10]);
                users.push_back(move(u));
                loaded++;
            }
            catch (...) {
                Logger::log("Corrupted user record skipped.", Logger::WARNING);
                printWarning("Skipped corrupted record in users.txt");
            }
        }
        file.close();
        if (loaded > 0)
            printSuccess(to_string(loaded) + " user(s) loaded from users.txt");
    }

    void loadBorrowRecords() {
        ifstream file("borrows.txt");
        if (!file) return;
        string line;
        int maxID = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> fields;
            while (getline(ss, token, '|')) fields.push_back(token);
            if ((int)fields.size() < 14) continue;
            try {
                int    uid   = stoi(fields[0]);
                int    recID = stoi(fields[1]);
                int    rid   = stoi(fields[2]);
                Date   issue(stoi(fields[3]),  stoi(fields[4]),  stoi(fields[5]));
                Date   due  (stoi(fields[6]),  stoi(fields[7]),  stoi(fields[8]));
                Date   ret  (stoi(fields[9]),  stoi(fields[10]), stoi(fields[11]));
                double fine  = stod(fields[12]);
                string stat  = fields[13];
                if (recID > maxID) maxID = recID;
                User* u = nullptr;
                try { u = findUser(uid); } catch(...) { continue; }
                u->addBorrowRecord(make_unique<BorrowRecord>(
                    recID, uid, rid, issue, due, ret, fine, stat));
            }
            catch (...) {
                Logger::log("Corrupted borrow record skipped.", Logger::WARNING);
            }
        }
        file.close();
        User::setNextRecordID(maxID);
        if (maxID > 0)
            printSuccess("Borrow records loaded. Next ID: " + to_string(maxID + 1));
    }

    void saveHoldRecords() const {
        ofstream file("holds.txt");
        for (auto& u : users) u->saveHoldRecords(file);
        file.close();
        Logger::log("Hold records saved.", Logger::INFO);
    }

    void loadHoldRecords() {
        ifstream file("holds.txt");
        if (!file) return;
        string line;
        int maxHoldID = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> fields;
            while (getline(ss, token, '|')) fields.push_back(token);
            if ((int)fields.size() < 6) continue;
            try {
                int    uid  = stoi(fields[0]);
                int    hid  = stoi(fields[1]);
                int    rid  = stoi(fields[2]);
                int    qpos = stoi(fields[3]);
                int    prio = stoi(fields[4]);
                string stat = fields[5];
                if (hid > maxHoldID) maxHoldID = hid;
                User* u = nullptr;
                try { u = findUser(uid); } catch(...) { continue; }
                u->loadHoldRecord(hid, rid, qpos, prio, stat);
            }
            catch (...) {
                Logger::log("Corrupted hold record skipped.", Logger::WARNING);
            }
        }
        file.close();
        User::setNextHoldID(maxHoldID);
        if (maxHoldID > 0)
            printSuccess("Hold records loaded. Next Hold ID: " + to_string(maxHoldID + 1));
    }

    void saveAll() const { saveBooks(); saveUsers(); saveHoldRecords(); }
    void loadAll()       { loadBooks(); loadUsers(); loadBorrowRecords(); loadHoldRecords(); }

    //  Getters 
    int    getAdminID()     const { return adminID;    }
    string getAdminName()   const { return adminName;  }
    string getRole()        const { return role;       }
    int    getAccessLevel() const { return accessLevel;}
    string getPassword()    const { return password;   }
};

#endif // ADMIN_H