#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <ctime>
#include <string>
using namespace std;

// ============================================================
//  LOGGER
//  Appends timestamped, severity-tagged entries to log.txt.
//  All methods are static — no instance needed.
// ============================================================
class Logger {
public:
    enum Level { INFO, WARNING, ERROR_LVL, ACTION };

    static void log(const string& message, Level level = INFO) {
        ofstream file("log.txt", ios::app);
        if (!file.is_open()) return;

        // Format: "2026-05-03 14:22:01 [ACTION ] UserID=5 | Borrowed: Dune"
        time_t now = time(0);
        char buf[32];
        struct tm* t = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);

        string tag;
        switch (level) {
            case INFO:      tag = "[INFO   ]"; break;
            case WARNING:   tag = "[WARN   ]"; break;
            case ERROR_LVL: tag = "[ERROR  ]"; break;
            case ACTION:    tag = "[ACTION ]"; break;
        }

        file << buf << " " << tag << " " << message << "\n";
        file.flush(); // flush immediately so a crash doesn't lose entries
        file.close();
    }

    // Convenience wrapper that prepends the user ID context
    static void logUser(int uid, const string& action) {
        log("UserID=" + to_string(uid) + " | " + action, ACTION);
    }
};

#endif // LOGGER_H
