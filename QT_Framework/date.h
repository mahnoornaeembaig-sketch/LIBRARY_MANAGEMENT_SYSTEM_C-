#ifndef DATE_H
#define DATE_H

#include <ctime>
#include <string>
#include "globals.h"
using namespace std;

// ============================================================
//  DATE STRUCT
//  Wraps a calendar date with helpers for arithmetic and
//  conversion to a day-count for overdue calculations.
// ============================================================
struct Date {
    int day, month, year;

    Date(int d = 1, int m = 1, int y = 2026)
        : day(d), month(m), year(y)
    {
        if (day   < 1 || day   > 31) throw InvalidInputException();
        if (month < 1 || month > 12) throw InvalidInputException();
        if (year  < 0)               throw InvalidInputException();
    }

    // Returns today's date from the system clock
    static Date today() {
        time_t now = time(0);
        tm* t = localtime(&now);
        return Date(t->tm_mday, t->tm_mon + 1, 1900 + t->tm_year);
    }

    // Converts date to total days since Unix epoch (for overdue math)
    long toDays() const {
        struct tm t = {};
        t.tm_mday  = day;
        t.tm_mon   = month - 1;
        t.tm_year  = year - 1900;
        t.tm_isdst = -1;
        time_t val = mktime(&t);
        if (val == (time_t)-1)
            return year * 365L + month * 30L + day; // fallback estimate
        return (long)(val / 86400L);
    }

    // Returns "DD/MM/YYYY" formatted string
    string toString() const {
        char buf[12];
        snprintf(buf, sizeof(buf), "%02d/%02d/%04d", day, month, year);
        return string(buf);
    }

    // Returns a new Date n calendar days in the future
    Date addDays(int n) const {
        struct tm t = {};
        t.tm_mday  = day + n;
        t.tm_mon   = month - 1;
        t.tm_year  = year - 1900;
        t.tm_isdst = -1;
        mktime(&t); // normalises overflow (e.g. Jan 32 → Feb 1)
        return Date(t.tm_mday, t.tm_mon + 1, 1900 + t.tm_year);
    }
};

#endif // DATE_H
