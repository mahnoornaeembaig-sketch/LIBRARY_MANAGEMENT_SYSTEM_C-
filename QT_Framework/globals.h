#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cctype>
using namespace std;

// ============================================================
//  CUSTOM EXCEPTIONS
// ============================================================
class InvalidInputException : public exception {
public:
    const char* what() const throw() { return "Invalid input provided."; }
};

class InvalidIDException : public exception {
public:
    const char* what() const throw() { return "ID must be a positive integer."; }
};

class NotFoundException : public exception {
public:
    const char* what() const throw() { return "Record not found."; }
};

class BorrowLimitException : public exception {
public:
    const char* what() const throw() {
        return "Borrow limit reached! Maximum 2 books allowed at a time.";
    }
};
class DailyLimitException : public exception {
public:
    const char* what() const noexcept override {
        return "Daily borrow limit reached (2/day). Try again tomorrow.";
    }
};
class OutstandingFineException : public exception {
public:
    const char* what() const noexcept override {
        return "Outstanding fine must be cleared before borrowing.";
    }
};

class NotAvailableException : public exception {
public:
    const char* what() const throw() { return "Resource not available for borrowing."; }
};

// ============================================================
//  FINE CONSTANT
// ============================================================
const double FINE_PER_DAY = 10.0;   // $10 per overdue day
const int DAILY_BORROW_LIMIT = 2;

// ============================================================
//  ANSI COLOR CODES
// ============================================================
namespace Color {
const string RESET    = "\033[0m";
const string BOLD     = "\033[1m";
const string DIM      = "\033[2m";
const string RED      = "\033[31m";
const string GREEN    = "\033[32m";
const string YELLOW   = "\033[33m";
const string BLUE     = "\033[34m";
const string MAGENTA  = "\033[35m";
const string CYAN     = "\033[36m";
const string WHITE    = "\033[37m";
const string BRED     = "\033[91m";
const string BGREEN   = "\033[92m";
const string BYELLOW  = "\033[93m";
const string BBLUE    = "\033[94m";
const string BMAGENTA = "\033[95m";
const string BCYAN    = "\033[96m";
const string BWHITE   = "\033[97m";
const string BG_BLUE  = "\033[44m";
const string BG_CYAN  = "\033[46m";
const string BG_GREEN = "\033[42m";
const string BG_RED   = "\033[41m";
const string BG_MAG   = "\033[45m";
}

// ============================================================
//  UI HELPERS
// ============================================================
inline void printLine(char c = '-', int len = 52) {
    cout << Color::DIM;
    for (int i = 0; i < len; i++) cout << c;
    cout << Color::RESET << "\n";
}

inline void printDoubleLine(int len = 52) {
    cout << Color::CYAN << Color::BOLD;
    for (int i = 0; i < len; i++) cout << "=";
    cout << Color::RESET << "\n";
}

inline void printHeader(const string& title) {
    printDoubleLine();
    int pad = (int)(52 - title.size()) / 2;
    cout << Color::BG_BLUE << Color::BWHITE << Color::BOLD;
    cout << string(pad, ' ') << title
         << string(52 - pad - (int)title.size(), ' ');
    cout << Color::RESET << "\n";
    printDoubleLine();
}

inline void printSuccess(const string& msg) {
    cout << Color::BGREEN << Color::BOLD << "  [OK] " << Color::RESET
         << Color::GREEN  << msg << Color::RESET << "\n";
}

inline void printError(const string& msg) {
    cout << Color::BRED  << Color::BOLD << "  [!!] " << Color::RESET
         << Color::RED   << msg << Color::RESET << "\n";
}

inline void printInfo(const string& msg) {
    cout << Color::BCYAN << "  [*] " << Color::RESET
         << Color::CYAN  << msg << Color::RESET << "\n";
}

inline void printWarning(const string& msg) {
    cout << Color::BYELLOW << "  [!] " << Color::RESET
         << Color::YELLOW  << msg << Color::RESET << "\n";
}

inline void printField(const string& label, const string& value,
                       int labelWidth = 16) {
    cout << Color::DIM << "  " << left << setw(labelWidth) << label
         << Color::RESET << Color::BWHITE << value << Color::RESET << "\n";
}

inline void printFieldColored(const string& label, const string& value,
                              const string& /*valueColor*/, int labelWidth = 16) {
    cout << Color::DIM << "  " << left << setw(labelWidth) << label
         << Color::RESET << value << Color::RESET << "\n";
}

// ============================================================
//  VALIDATION HELPERS
// ============================================================
inline bool isValidName(const string& name) {
    if (name.empty()) return false;
    for (char c : name)
        if (!isalpha(c) && c != ' ') return false;
    return true;
}

// Username: 3-15 chars, no spaces, must have at least one letter
inline bool isValidUsername(const string& u) {
    if (u.empty()) return false;
    if (u.length() < 3 || u.length() > 15) return false;
    for (char c : u)
        if (isspace(c)) return false;
    bool hasLetter = false;
    for (char c : u)
        if (isalpha(c)) { hasLetter = true; break; }
    return hasLetter;
}

inline bool isValidPassword(const string& p) {
    if (p.length() < 6) return false;
    bool hasLetter = false, hasDigit = false;
    for (char c : p) {
        if (isalpha(c)) hasLetter = true;
        if (isdigit(c)) hasDigit  = true;
    }
    return hasLetter && hasDigit;
}

// Email: no spaces, no consecutive dots, no dot right after @,
// must have @ and a dot after it with chars on both sides
inline bool isValidEmail(const string& e) {
    if (e.empty()) return false;
    for (char c : e)
        if (isspace(c)) return false;
    if (e.find("..") != string::npos) return false;
    size_t at  = e.find('@');
    size_t dot = e.rfind('.');
    if (at == string::npos || dot == string::npos) return false;
    if (at < 1 || dot < at + 2 || dot == e.length() - 1) return false;
    if (e[at + 1] == '.') return false;
    return true;
}

inline string toLower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

// Publisher: 2–100 chars, letters/digits/spaces and . , & - ( ) only
inline bool isValidPublisher(const string& pub) {
    if (pub.length() < 2 || pub.length() > 100) return false;
    bool allSpaces = true;
    for (char c : pub) {
        if (!isspace(c)) allSpaces = false;
        if (!isalnum(c) && !isspace(c) &&
            c != '.' && c != ',' && c != '&' &&
            c != '-' && c != '(' && c != ')')
            return false;
    }
    return !allSpaces;
}

// Language: 2–30 chars, letters and spaces only
inline bool isValidLanguage(const string& lang) {
    if (lang.length() < 2 || lang.length() > 30) return false;
    bool allSpaces = true;
    for (char c : lang) {
        if (!isspace(c)) allSpaces = false;
        if (!isalpha(c) && !isspace(c)) return false;
    }
    return !allSpaces;
}

// Genre: 2-50 chars, letters/spaces/hyphens/& only (e.g. "Science Fiction", "Self-Help")
inline bool isValidGenre(const string& genre) {
    if (genre.length() < 2 || genre.length() > 50) return false;
    bool allSpaces = true;
    for (char c : genre) {
        if (!isspace(c)) allSpaces = false;
        if (!isalpha(c) && !isspace(c) && c != '-' && c != '&' && c != '\'')
            return false;
    }
    return !allSpaces;
}

inline bool isValidShelfCode(const string& code) {
    if (code.length() < 2 || code.length() > 10) return false;
    for (char c : code)
        if (!isalnum(c)) return false;
    return true;
}

inline bool isValidResourceID(const string& id) {
    if (id.empty()) return false;
    for (char c : id)
        if (!isdigit(c)) return false;
    return true;
}
inline bool isValidUserID(const string& id) {
    if (id.empty()) return false;
    for (char c : id)
        if (!isdigit(c)) return false;
    return true;
}
inline bool isValidBookID(const string& id) {
    if (id.empty()) return false;
    for (char c : id)
        if (!isdigit(c)) return false;
    return true;
}

#endif // GLOBALS_H
