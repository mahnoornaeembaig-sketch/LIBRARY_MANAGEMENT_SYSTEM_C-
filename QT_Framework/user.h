#ifndef USER_H
#define USER_H

#include <vector>
#include <memory>
#include <fstream>
#include "globals.h"
#include "date.h"
#include "logger.h"
#include "resource.h"
#include "records.h"
using namespace std;

// ============================================================
//  MEMBERSHIP
//  Represents the user's membership tier with borrowing limits and discounts.
//
//  Tier limits (active-at-once):
//    Basic  → 2 books
//    Silver → 3 books
//    Gold   → 4 books
//
//  The daily borrow cap is always DAILY_BORROW_LIMIT (2) regardless of tier.
//  Membership only controls how many books can be held simultaneously.
// ============================================================
class Membership {
private:
    int    maxBooks;
    double discountRate;
    int    borrowDuration;
    string level;

public:
    Membership(int maxB = 2, double discount = 0.0,
               int duration = 14, const string& lvl = "Basic")
        : maxBooks(maxB), discountRate(discount),
        borrowDuration(duration), level(lvl) {}

    string getLevelDetails() const {
        string badge;
        if      (level == "Gold")   badge = Color::BYELLOW + "[GOLD]"   + Color::RESET;
        else if (level == "Silver") badge = Color::BWHITE  + "[SILVER]" + Color::RESET;
        else                        badge = Color::DIM     + "[BASIC]"  + Color::RESET;

        return badge + " Max:"  + to_string(maxBooks)         +
               " | Discount:"  + to_string((int)discountRate) + "%" +
               " | Duration:"  + to_string(borrowDuration)    + "d";
    }

    //  Silver and Gold now have meaningfully distinct active-at-once limits.
    // Basic = 2, Silver = 3, Gold = 4.
    // The daily cap (DAILY_BORROW_LIMIT = 2) is enforced separately in borrowBook()
    // and is never overridden by membership tier.
    void upgrade(const string& newLevel) {
        level = newLevel;
        if      (newLevel == "Silver") { maxBooks = 3; discountRate = 10.0; borrowDuration = 21; }
        else if (newLevel == "Gold")   { maxBooks = 4; discountRate = 20.0; borrowDuration = 30; }
        printSuccess("Membership upgraded to " + level + "!");
    }

    void loadLevel(const string& lvl) {
        if (lvl == "Silver" || lvl == "Gold") upgrade(lvl);
    }

    string serialize() const { return level; }

    int    getMaxBooks()       const { return maxBooks;       }
    int    getBorrowDuration() const { return borrowDuration; }
    double getDiscountRate()   const { return discountRate;   }
    string getLevel()          const { return level;          }
};


// ============================================================
//  ACCOUNT
//  Manages the user's balance and fines.
// ============================================================
class Account {
private:
    int    accountID;
    double balance;
    double fineDue;
    int    totalBorrowed;

public:
    Account(int id = 0, double bal = 0.0,
            double fine = 0.0, int borrowed = 0)
        : accountID(id), balance(bal),
        fineDue(fine), totalBorrowed(borrowed) {}

    static Account createAccount(int id, double initialBalance = 0.0) {
        return Account(id, initialBalance, 0.0, 0);
    }

    void addFine(double amount) {
        if (amount < 0) throw InvalidInputException();
        fineDue += amount;
        if (balance >= fineDue) {
            cout << Color::BYELLOW << "  Auto-deducting fine $" << fineDue
                 << " from balance.\n" << Color::RESET;
            balance -= fineDue;
            fineDue  = 0.0;
        } else if (balance > 0) {
            printWarning("Partial fine deduction: $" + to_string((int)balance) + " deducted.");
            fineDue -= balance;
            balance  = 0.0;
            printWarning("Remaining fine due: $" + to_string((int)fineDue));
        } else {
            printWarning("Fine $" + to_string((int)amount) +
                         " added. Outstanding: $" + to_string((int)fineDue));
        }
    }

    void payFine(double amount) {
        if (amount <= 0 || amount > fineDue)
            throw InvalidInputException();
        if (amount > balance) {
            printError("Insufficient balance! You have $" + to_string((int)balance) +
                       " but need $" + to_string((int)amount) + ".");
            return;
        }
        balance -= amount;   // ← deduct from balance
        fineDue -= amount;
        printSuccess("Fine $" + to_string((int)amount) +
                     " paid. Remaining: $" + to_string((int)fineDue));
    }

    void deposit(double amount) {
        if (amount <= 0) throw InvalidInputException();
        balance += amount;
        printSuccess("Deposited $" + to_string((int)amount) +
                     ". Balance: $" + to_string((int)balance));
    }

    void deductFine(double amount) {
        if (amount < 0 || amount > fineDue) throw InvalidInputException();
        fineDue -= amount;
    }

    void resetFines()        { fineDue = 0.0;  }
    void incrementBorrowed() { totalBorrowed++; }
    void decrementBorrowed() { if (totalBorrowed > 0) totalBorrowed--; }

    void getStatement() const {
        printLine('-', 40);
        cout << Color::BCYAN << Color::BOLD << "  ACCOUNT STATEMENT\n" << Color::RESET;
        printLine('-', 40);
        printField("Account ID:",     to_string(accountID));
        printFieldColored("Balance:",  Color::BGREEN + "$" + to_string((int)balance) + Color::RESET, "");
        string fineColor = fineDue > 0 ? Color::BRED : Color::BGREEN;
        printFieldColored("Fine Due:", fineColor + "$" + to_string((int)fineDue) + Color::RESET, "");
        printField("Total Borrowed:", to_string(totalBorrowed));
        printLine('-', 40);
    }

    int    getAccountID()     const { return accountID;     }
    double getBalance()       const { return balance;       }
    double getFinesDue()      const { return fineDue;       }
    int    getTotalBorrowed() const { return totalBorrowed; }
    void   setBalance(double b)     { balance = b; }
    void   setFineDue(double f)     { fineDue = f; }
};


// ============================================================
//  USER
//  Owns an Account, a Membership, a borrow history, and a hold list.
// ============================================================
class User {
private:
    int    userID;
    string firstName;
    string lastName;
    string username;
    string password;
    string address;
    string email;
    string registrationDate;
    int    currentBorrows;

    unique_ptr<Account>              account;
    unique_ptr<Membership>           membership;
    vector<unique_ptr<BorrowRecord>> borrowHistory;
    vector<unique_ptr<HoldRecord>>   holdList;

    static int nextRecordID;   // for borrow records only
    static int nextHoldID;     // for hold records only

public:
    User(int id, const string& fName, const string& lName,
         const string& uname, const string& pass,
         const string& addr,  const string& mail,
         const string& regDate = "",
         double initialBalance = 0.0)
        : userID(id), firstName(fName), lastName(lName),
        username(uname), password(pass), address(addr),
        email(mail), currentBorrows(0)
    {
        if (userID <= 0) throw InvalidIDException();
        if (!isValidName(fName) || !isValidName(lName))
            throw InvalidInputException();
        if (!isValidUsername(username))
            throw InvalidInputException();
        if (!isValidEmail(email))
            throw InvalidInputException();
        registrationDate = regDate.empty() ? Date::today().toString() : regDate;
        account    = make_unique<Account>(id, initialBalance);
        membership = make_unique<Membership>();
    }

    ~User() = default;

    bool verifyPassword(const string& pass) const { return password == pass; }

    bool login(const string& uname, const string& pass) const {
        if (username == uname && password == pass) {
            printSuccess("Welcome back, " + firstName + "!");
            return true;
        }
        printError("Invalid credentials.");
        return false;
    }

    void logout() const {
        printInfo(firstName + " has been logged out. Goodbye!");
        Logger::log("User " + username + " logged out.", Logger::ACTION);
    }

    // ── Library Operations ────────────────────────────────────

    bool canBorrow() const {
        return currentBorrows    <  membership->getMaxBooks()
        && getBorrowsToday() <  DAILY_BORROW_LIMIT
            && account->getFinesDue() == 0.0;
    }

    void borrowBook(Resource* r) {
        if (!r) throw InvalidInputException();

        // Active-at-once limit — throws so the caller gets a named exception.
        if (currentBorrows >= membership->getMaxBooks())
            throw BorrowLimitException();

        if (getBorrowsToday() >= DAILY_BORROW_LIMIT)
                    throw DailyLimitException();


        if (account->getFinesDue() > 0.0)
            throw OutstandingFineException();

        r->borrowCopy();

        Date today = Date::today();

        // Use the book's own dueDays; fall back to membership duration.
        Book* b    = dynamic_cast<Book*>(r);
        int   days = b ? b->getDueDays() : membership->getBorrowDuration();

        Date dueDay = today.addDays(days);

        borrowHistory.push_back(make_unique<BorrowRecord>(
            ++nextRecordID, userID, r->getResourceID(), today, dueDay));

        account->incrementBorrowed();
        currentBorrows++;

        printSuccess("'" + r->getTitle() + "' borrowed successfully!");
        borrowHistory.back()->printReceipt();
        Logger::logUser(userID, "Borrowed: " + r->getTitle());
    }

    int getBorrowsToday() const {
        Date today = Date::today();
        int count = 0;
        for (auto& rec : borrowHistory)
            if (rec->getIssueDate().toString() == today.toString())
                count++;
        return count;
    }

    void returnBook(Resource* r) {
        for (auto& rec : borrowHistory) {
            if (rec->getResourceID() == r->getResourceID() && rec->isActive()) {
                rec->closeRecord();
                r->returnCopy();
                if (rec->getFineAmount() > 0.0) {
                    account->addFine(rec->getFineAmount());
                    // if addFine auto-cleared it, mark the record paid too
                    if (account->getFinesDue() == 0.0)
                        rec->markFinePaid();        // ← add this
                }
                if (currentBorrows > 0) currentBorrows--;
                printSuccess("'" + r->getTitle() + "' returned successfully.");
                Logger::logUser(userID, "Returned: " + r->getTitle());
                return;
            }
        }
        throw NotFoundException();
    }

    void placeHold(Resource* r) {
        if (r->checkAvailability()) {
            printInfo("Book is available — borrow it directly.");
            return;
        }
        for (auto& h : holdList) {
            if (h->getResourceID() == r->getResourceID() &&
                h->getStatus() == "pending") {
                printError("You already have an active hold on this book.");
                return;
            }
        }
        // count only active pending holds for correct position
        int pos = 0;
        for (auto& h : holdList)
            if (h->getStatus() == "pending") pos++;
        pos += 1;

        holdList.push_back(make_unique<HoldRecord>(
            ++nextHoldID, userID, r->getResourceID(), pos));
        holdList.back()->placeHold();
        Logger::logUser(userID, "Placed hold on: " + r->getTitle());
    }

    HoldRecord* getActiveHoldForResource(int rid) {
        for (auto& h : holdList)
            if (h->getResourceID() == rid && h->getStatus() == "pending")
                return h.get();
        return nullptr;
    }

    bool hasActiveBorrowForResource(int rid) const {
        for (auto& rec : borrowHistory)
            if (rec->getResourceID() == rid && rec->isActive()) return true;
        return false;
    }

    void printOverdueRecords() const {
        for (auto& rec : borrowHistory) {
            if (rec->isOverdue()) {
                cout << Color::BRED << "  >> Overdue: " << firstName << " " << lastName
                     << " (ID:" << userID << ")" << Color::RESET << "\n";
                rec->printReceipt();
            }
        }
    }

    bool hasOverdueRecords() const {
        for (auto& rec : borrowHistory)
            if (rec->isOverdue()) return true;
        return false;
    }

    // ── Profile / Account ─────────────────────────────────────

    void updateProfile() {
        string newAddr, newMail;

        while (true) {
            cout << Color::CYAN << "  New address (min 5 chars): " << Color::RESET;
            getline(cin, newAddr);
            if (newAddr.empty() || newAddr.length() < 5) {
                printError("Address too short! Must be at least 5 characters.");
                continue;
            }
            bool allSpaces = true;
            for (char c : newAddr)
                if (!isspace(c)) { allSpaces = false; break; }
            if (allSpaces) {
                printError("Address cannot be all spaces!");
                continue;
            }
            break;
        }

        while (true) {
            cout << Color::CYAN << "  New email       : " << Color::RESET;
            getline(cin, newMail);
            if (!isValidEmail(newMail)) {
                printError("Invalid email format!");
                printInfo("Example: username@domain.com — no spaces or consecutive dots.");
                continue;
            }
            break;
        }

        address = newAddr;
        email   = newMail;
        printSuccess("Profile updated.");
        Logger::logUser(userID, "Updated profile.");
    }

    void payFine(double amount) {
        if (amount <= 0) {
            printError("Amount must be greater than $0.");
            return;
        }
        if (account->getFinesDue() == 0) {
            printInfo("You have no outstanding fines.");
            return;
        }
        if (amount > account->getFinesDue()) {
            printError("Amount exceeds fine due of $" +
                       to_string((int)account->getFinesDue()) + ".");
            return;
        }
        account->payFine(amount);

        // Mark overdue records as paid if no fine remains.
        if (account->getFinesDue() == 0) {
            for (auto& rec : borrowHistory)
                if (rec->getStatus() == "overdue" && !rec->isFinePaid())
                    rec->markFinePaid();
        }
    }

    void depositBalance() {
        double amt;
        cout << Color::CYAN << "  Deposit amount ($): " << Color::RESET;
        if (!(cin >> amt)) {
            cin.clear(); cin.ignore(1000, '\n');
            throw InvalidInputException();
        }
        if (amt <= 0) {
            printError("Amount must be greater than $0.");
            return;
        }
        if (amt > 10000) {
            printError("Maximum deposit is $10000 at a time.");
            return;
        }
        account->deposit(amt);
        Logger::logUser(userID, "Deposited $" + to_string((int)amt));
    }

    // ── View Methods ──────────────────────────────────────────

    void viewAccountStatement() const { account->getStatement(); }

    void viewBorrowHistory() const {
        if (borrowHistory.empty()) { printInfo("No borrow records found."); return; }
        for (auto& rec : borrowHistory) rec->printReceipt();
    }

    vector<BorrowRecord*> getBorrowHistory() const {
        vector<BorrowRecord*> result;
        for (auto& rec : borrowHistory)
            result.push_back(rec.get());
        return result;
    }

    void viewCurrentlyBorrowed() const {
        bool found = false;
        printHeader("CURRENTLY BORROWED");
        for (auto& rec : borrowHistory) {
            if (rec->isActive()) { rec->printReceipt(); found = true; }
        }
        if (!found) printInfo("No books currently borrowed.");
    }

    void viewHolds() const {
        if (holdList.empty()) { printInfo("No holds placed."); return; }
        for (auto& h : holdList) h->display();
    }

    void show() const {
        printHeader("USER PROFILE");
        printField("ID:",         to_string(userID));
        printField("Name:",       firstName + " " + lastName);
        printField("Username:",   username);
        printField("Email:",      email);
        printField("Address:",    address);
        printField("Registered:", registrationDate);
        printField("Membership:", membership->getLevelDetails());

        string borrowColor = (currentBorrows >= membership->getMaxBooks())
                                 ? Color::BRED : Color::BGREEN;
        printFieldColored("Borrows:",
                          borrowColor + to_string(currentBorrows) + "/" +
                              to_string(membership->getMaxBooks()) + Color::RESET, "");
        account->getStatement();
    }

    // ── File I/O ──────────────────────────────────────────────

    // currentBorrows is excluded — rebuilt automatically from borrow records on load.
    string serialize() const {
        return to_string(userID) + "|" + firstName        + "|" + lastName + "|" +
               username          + "|" + password         + "|" + address  + "|" +
               email             + "|" + registrationDate + "|" +
               to_string(account->getBalance())  + "|" +
               to_string(account->getFinesDue()) + "|" +
               membership->serialize();
    }

    void saveBorrowRecords(ofstream& file) const {
        for (auto& rec : borrowHistory)
            file << rec->serialize() << "\n";
    }

    void saveHoldRecords(ofstream& file) const {
        for (auto& h : holdList) {
            file << to_string(userID)               + "|"
                        + to_string(h->getHoldID())        + "|"
                        + to_string(h->getResourceID())    + "|"
                        + to_string(h->getQueuePosition()) + "|"
                        + to_string(h->getPriority())      + "|"
                        + h->getStatus()                   + "\n";
        }
    }

    // Only loads pending holds — cancelled/approved/expired are discarded.
    void loadHoldRecord(int holdID, int resourceID,
                        int qPos, int prio, const string& status) {
        if (status == "cancelled" || status == "approved" || status == "expired")
            return;
        holdList.push_back(make_unique<HoldRecord>(
            holdID, userID, resourceID, qPos, prio));
    }

    // FIX 5 — account->incrementBorrowed() is now called here so that
    // totalBorrowed is correctly restored after a file reload.

    void addBorrowRecord(unique_ptr<BorrowRecord> rec) {
        if (rec->isActive()) currentBorrows++;
        account->incrementBorrowed();   // restore totalBorrowed on load
        borrowHistory.push_back(move(rec));
    }

    void upgradeMembership(const string& level) {
        string normalised = toLower(level);

        if (normalised != "silver" && normalised != "gold") {
            printError("Invalid membership level. Must be Silver or Gold.");
            return;
        }

        string currentNorm = toLower(membership->getLevel());

        if (currentNorm == normalised) {
            printInfo("Already at " + membership->getLevel() + " membership level.");
            return;
        }

        if (currentNorm == "gold" && normalised == "silver") {
            printError("Downgrades are not allowed.");
            return;
        }

        string canonical = (normalised == "gold") ? "Gold" : "Silver";
        membership->upgrade(canonical);
        Logger::logUser(userID, "Membership upgraded to " + canonical + " by admin.");
    }

    // ── Static Helpers ────────────────────────────────────────
    static void setNextRecordID(int id) { nextRecordID = id; }
    static int  getNextRecordID()       { return nextRecordID; }
    static void setNextHoldID(int id)   { nextHoldID = id; }
    static int  getNextHoldID()         { return nextHoldID; }

    // ── Getters / Setters ─────────────────────────────────────
    int         getUserID()           const { return userID;           }
    string      getUsername()         const { return username;         }
    string      getFirstName()        const { return firstName;        }
    string      getLastName()         const { return lastName;         }
    string      getEmail()            const { return email;            }
    string      getAddress()          const { return address;          }
    string      getPassword()         const { return password;         }
    string      getRegistrationDate() const { return registrationDate; }
    int         getCurrentBorrows()   const { return currentBorrows;   }
    Account*    getAccount()          const { return account.get();    }
    Membership* getMembership()       const { return membership.get(); }

    void setAddress(const string& addr) { address = addr; }
    void setEmail  (const string& mail) { email   = mail; }

    // Returns pointers to all active overdue borrow records
    vector<BorrowRecord*> getOverdueRecords() const {
        vector<BorrowRecord*> result;
        for (auto &rec : borrowHistory)
            if (rec->isOverdue()) result.push_back(rec.get());
        return result;
    }

    vector<HoldRecord*> getHoldList() const {
        vector<HoldRecord*> result;
        for (auto& h : holdList)
            result.push_back(h.get());
        return result;
    }
};

#endif // USER_H
