#ifndef RECORDS_H
#define RECORDS_H

#include <string>
#include <stdexcept>
#include "globals.h"
#include "date.h"
using namespace std;

/* ============================================================
  BORROW RECORD
 ============================================================*/
class BorrowRecord {
private:
    int    recordID;
    Date   issueDate;
    Date   dueDate;
    Date   returnDate;
    double fineAmount;
    bool   hasReturnDate;
    string status;
    bool   finePaid;
    int    userID;
    int    resourceID;

    static void validateID(int id, const string& name) {
        if (id < 0)
            throw invalid_argument(name + " cannot be negative: " + to_string(id));
    }

public:
    BorrowRecord()
        : recordID(0), fineAmount(0.0), hasReturnDate(false),
          status("active"), finePaid(false), userID(0), resourceID(0) {}

    // Constructor for new borrows
    BorrowRecord(int id, int uid, int rid,
                 const Date& issue, const Date& due)
        : recordID(id), issueDate(issue), dueDate(due),
          returnDate(Date()), fineAmount(0.0), hasReturnDate(false),
          status("active"), finePaid(false), userID(uid), resourceID(rid)
    {
        validateID(id,  "Record ID");
        validateID(uid, "User ID");
        validateID(rid, "Resource ID");
    }

    // Constructor for loading saved records from file
    BorrowRecord(int id, int uid, int rid,
                 const Date& issue, const Date& due,
                 const Date& ret, double fine, const string& stat,
                 bool paid = false)
        : recordID(id), issueDate(issue), dueDate(due),
          returnDate(ret), fineAmount(fine), status(stat),
          finePaid(paid), userID(uid), resourceID(rid)
    {
        validateID(id,  "Record ID");
        validateID(uid, "User ID");
        validateID(rid, "Resource ID");
        hasReturnDate = (stat == "returned" || stat == "overdue");
    }

    double calculateFine(const Date& currentDate) const {
        long overdue = currentDate.toDays() - dueDate.toDays();
        return (overdue > 0) ? overdue * FINE_PER_DAY : 0.0;
    }

    // guard against calling closeRecord() more than once
    void closeRecord() {
        if (!isActive()) {
            printError("Record #" + to_string(recordID) + " is already closed.");
            return;
        }
        returnDate    = Date::today();
        hasReturnDate = true;
        fineAmount    = calculateFine(returnDate);
        status        = (fineAmount > 0.0) ? "overdue" : "returned";

        if (fineAmount > 0.0)
            printWarning("Fine of $" + to_string((int)fineAmount) + " charged (overdue by "
                + to_string((int)(returnDate.toDays() - dueDate.toDays())) + " days).");
    }

    bool isActive() const { return status == "active"; }

    // isOverdue() — true only while the record is still open and past due
    bool isOverdue() const {
        return isActive() && Date::today().toDays() > dueDate.toDays();
    }

    // Use this to check if a CLOSED record was overdue at return time
    bool wasOverdue() const {
        return status == "overdue";
    }

    int getDaysRemaining() const {
        return (int)(dueDate.toDays() - Date::today().toDays());
    }

    void printReceipt() const {
        printLine('-', 46);
        cout << Color::BCYAN << Color::BOLD
             << "  BORROW RECEIPT  #" << recordID << Color::RESET << "\n";
        printLine('-', 46);
        printField("User ID:",     to_string(userID));
        printField("Resource ID:", to_string(resourceID));
        printField("Issue Date:",  issueDate.toString());

        int daysLeft = getDaysRemaining();
        string dueStr = dueDate.toString();
        if (isActive()) {
            if (daysLeft < 0)
                dueStr += Color::BRED    + " (OVERDUE " + to_string(-daysLeft) + " days)" + Color::RESET;
            else if (daysLeft <= 3)
                dueStr += Color::BYELLOW + " (" + to_string(daysLeft) + " days left)" + Color::RESET;
            else
                dueStr += Color::BGREEN  + " (" + to_string(daysLeft) + " days left)" + Color::RESET;
        }
        printFieldColored("Due Date:", dueStr, "");

        if (hasReturnDate)
            printField("Return Date:", returnDate.toString());
        else
            printFieldColored("Return Date:", Color::DIM + string("Not yet returned") + Color::RESET, "");

        double displayFine = (isActive() && isOverdue())
                           ? calculateFine(Date::today())
                           : fineAmount;
        string fineStr = "$" + to_string((int)displayFine);

        string statusStr;
        if (status == "active")
            statusStr = Color::BGREEN + "Active" + Color::RESET;
        else if (status == "overdue")
            statusStr = isFinePaid()
                ? Color::BGREEN + "Fine Paid" + Color::RESET
                : Color::BRED   + "Overdue"   + Color::RESET;
        else
            statusStr = Color::DIM + "Returned" + Color::RESET;

        printFieldColored("Fine:",   displayFine > 0
            ? Color::BRED   + fineStr + Color::RESET
            : Color::BGREEN + fineStr + Color::RESET, "");
        printFieldColored("Status:", statusStr, "");
        printLine('-', 46);
    }

    //finePaid is serialized so it survives restarts
    string serialize() const {
        return to_string(userID)               + "|" + to_string(recordID)          + "|" +
               to_string(resourceID)           + "|" +
               to_string(issueDate.day)        + "|" + to_string(issueDate.month)   + "|" + to_string(issueDate.year)   + "|" +
               to_string(dueDate.day)          + "|" + to_string(dueDate.month)     + "|" + to_string(dueDate.year)     + "|" +
               to_string(returnDate.day)       + "|" + to_string(returnDate.month)  + "|" + to_string(returnDate.year)  + "|" +
               to_string(fineAmount)           + "|" + status                       + "|" +
               to_string(finePaid);
    }

    // Getters / Setters
    int    getRecordID()   const { return recordID;   }
    int    getUserID()     const { return userID;     }
    int    getResourceID() const { return resourceID; }
    double getFineAmount() const { return fineAmount; }
    string getStatus()     const { return status;     }
    Date   getIssueDate()  const { return issueDate;  }
    Date   getDueDate()    const { return dueDate;    }
    void   setStatus    (const string& s) { status     = s; }
    void   setFineAmount(double f)        { fineAmount = f; }
    void   markFinePaid()       { finePaid = true;  }
    bool   isFinePaid()   const { return finePaid;   }
};


/* ============================================================
  HOLD RECORD
============================================================*/
class HoldRecord {
private:
    int    holdID;
    Date   holdDate;
    Date   expiryDate;
    string status;
    int    queuePosition;
    int    priority;
    bool   adminApproved;
    int    userID;
    int    resourceID;

public:
    HoldRecord()
        : holdID(0), status("pending"), queuePosition(1),
          priority(1), adminApproved(false), userID(0), resourceID(0) {}

    HoldRecord(int id, int uid, int rid, int qPos = 1, int prio = 1)
        : holdID(id), holdDate(Date::today()),
          expiryDate(Date::today().addDays(7)),
          status("pending"), queuePosition(qPos),
          priority(prio), adminApproved(false),
          userID(uid), resourceID(rid) {}

    // Constructor for loading saved holds from file
    HoldRecord(int id, int uid, int rid,
               const Date& hDate, const Date& eDate,
               const string& stat, int qPos, int prio, bool approved)
        : holdID(id), holdDate(hDate), expiryDate(eDate),
          status(stat), queuePosition(qPos), priority(prio),
          adminApproved(approved), userID(uid), resourceID(rid) {}

    //  placeHold() checks for expired or cancelled status before allowing a new hold to be placed.
    bool placeHold() {
        if (isExpired()) {
            status = "expired";
            printError("Hold #" + to_string(holdID) + " cannot be placed (expired).");
            return false;
        }
        if (status == "cancelled" || status == "expired") {
            printError("Hold #" + to_string(holdID) + " cannot be placed (" + status + ").");
            return false;
        }
        status = "pending";
        printInfo("Hold #" + to_string(holdID) + " placed. Queue position: "
                  + to_string(queuePosition));
        return true;
    }

    void cancelHold() {
        status = "cancelled";
        printWarning("Hold #" + to_string(holdID) + " cancelled.");
    }

    // markApproved()  refuses cancelled or expired holds
    void markApproved() {
        if (status == "cancelled" || status == "expired") {
            printError("Cannot approve hold #" + to_string(holdID) + " (" + status + ").");
            return;
        }
        adminApproved = true;
        status        = "approved";
        printSuccess("Hold #" + to_string(holdID) + " approved.");
    }

    bool isExpired() const {
        return Date::today().toDays() > expiryDate.toDays();
    }

    void notifyUser() const {
        printLine('*', 50);
        cout << Color::BMAGENTA << Color::BOLD
             << "  [NOTIFICATION] Hold #" << holdID << " READY\n" << Color::RESET;
        cout << Color::MAGENTA
             << "  Resource ID " << resourceID
             << " is now available for User " << userID << ".\n"
             << "  Please collect before " << expiryDate.toString() << ".\n"
             << Color::RESET;
        printLine('*', 50);
    }

    void extendExpiry(int extraDays = 7) {
        if (status == "cancelled" || status == "expired") {
            printError("Cannot extend hold #" + to_string(holdID) + " (" + status + ").");
            return;
        }
        expiryDate = expiryDate.addDays(extraDays);
        printInfo("Hold #" + to_string(holdID) + " expiry extended to "
                  + expiryDate.toString());
    }

    void display() const {
        printLine('-', 46);
        cout << Color::BMAGENTA << Color::BOLD
             << "  HOLD RECORD #" << holdID << Color::RESET << "\n";
        printLine('-', 46);
        printField("User ID:",     to_string(userID));
        printField("Resource ID:", to_string(resourceID));
        printField("Hold Date:",   holdDate.toString());
        printField("Expiry Date:", expiryDate.toString());
        printField("Queue Pos:",   to_string(queuePosition));
        printField("Priority:",    to_string(priority));

        string sColor = (status == "approved") ? Color::BGREEN
                      : (status == "pending")  ? Color::BYELLOW : Color::BRED;
        printFieldColored("Status:",   sColor + status + Color::RESET, "");
        printFieldColored("Admin OK:", adminApproved
            ? Color::BGREEN + string("Yes") + Color::RESET
            : Color::DIM    + string("No")  + Color::RESET, "");
        printLine('-', 46);
    }

    // Serialize for file storage
    string serialize() const {
        return to_string(userID)          + "|" + to_string(holdID)         + "|" +
               to_string(resourceID)      + "|" +
               to_string(holdDate.day)    + "|" + to_string(holdDate.month)   + "|" + to_string(holdDate.year)   + "|" +
               to_string(expiryDate.day)  + "|" + to_string(expiryDate.month) + "|" + to_string(expiryDate.year) + "|" +
               status                     + "|" +
               to_string(queuePosition)   + "|" + to_string(priority)        + "|" +
               to_string(adminApproved);
    }

    // Getters / Setters
    int    getHoldID()        const { return holdID;        }
    int    getUserID()        const { return userID;        }
    int    getResourceID()    const { return resourceID;    }
    int    getPriority()      const { return priority;      }
    int    getQueuePosition() const { return queuePosition; }
    string getStatus()        const { return status;        }
    Date   getHoldDate()      const { return holdDate;      }
    Date   getExpiryDate()    const { return expiryDate;    }
    bool   isAdminApproved()  const { return adminApproved; }
    void   setPriority     (int p)  { priority      = p;   }
    void   setQueuePosition(int q)  { queuePosition = q;   }
};

#endif // RECORDS_H