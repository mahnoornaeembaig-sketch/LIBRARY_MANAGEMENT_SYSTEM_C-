# 📚 Library Management System

---

## 📌 Project Overview

This project contains two fully functional versions of the Library Management System:

### 1. 🖥️ Console-Based Version (`G2-7CONSOLEBASED/`)
A terminal application with ANSI colour-coded menus, compiled and run using g++. No external dependencies required.

### 2. 🪟 GUI Version (`G2-7GUI/`)
A graphical desktop application built using the Qt 6.10.2 framework with a full windowed interface, dialogs, and a custom theme.

Both versions share the same backend logic and implement identical OOP concepts and features.

---

## 🚀 Steps to Run

### Console Version

1. Make sure **all source/header files are in the same folder**
2. Open terminal/command prompt in that folder
3. Compile using g++:

```bash
# Windows (MinGW / g++)
g++ main.cpp menus.cpp -o library -std=c++14

# Linux / macOS
g++ main.cpp menus.cpp -o library -std=c++14
```

4. Run the executable:

```bash
# Windows
library.exe

# Linux / macOS
./library
```

> **Note:** ANSI colour codes require a colour-capable terminal.
> Recommended: Windows Terminal, VS Code Terminal, or Linux/macOS default terminal.

---

### GUI Version

#### Method 1 — Qt Creator (Recommended)

1. Open **Qt Creator**
2. Go to **File → Open File or Project**
3. Navigate to `G2-7GUI/` and select **`CMakeLists.txt`**
4. When prompted, select the kit: `Desktop Qt 6.10.2 MinGW 64-bit` → click **Configure Project**
5. Press **Ctrl+B** to build and wait for it to finish
6. Go to `G2-7GUI/build/Desktop_Qt_6_10_2_MinGW_64_bit-Debug/` in File Explorer and locate the `.exe`
7. Copy the following files into the **same folder as the `.exe`**:
   - `theme.qss`
   - `books.txt`
   - `users.txt`
   - `borrows.txt`
   - `holds.txt`
8. Press **Ctrl+R** in Qt Creator to run, or double-click the `.exe` directly

#### Method 2 — Command Line

```bash
cd path\to\G2-7GUI
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.10.2\mingw_64
mingw32-make
```
Then copy `theme.qss` and `.txt` data files into the build output folder and run:
```bash
.\G2-7LibraryManagementSystem.exe
```

> **Notes:**
> - Make sure Qt 6.10.2 MinGW kit is properly configured in Qt Creator before building
> - All source and header files must remain in the same project folder
> - After building, copy `theme.qss` next to the `.exe` — the UI theme will not load otherwise
> - The program auto-generates data files on first run, but sample data files are included for convenience so the system can be tested immediately without manual data entry

---

## 🔑 Default Admin Login *(both versions)*

| Field | Value |
|-------|-------|
| Username | `Admin` |
| Password | `admin123` |

---

## 🗂️ Project File Structure

### Console Version (`G2-7CONSOLEBASED/`)

| File | Description |
|------|-------------|
| `globals.h` | Exceptions, color codes, validation, helper functions |
| `date.h` | Date structure and date arithmetic |
| `logger.h` | Logger class for timestamped action logging |
| `resource.h` | Abstract Resource class and Book derived class |
| `records.h` | BorrowRecord and HoldRecord classes |
| `user.h` | Account, Membership, and User classes |
| `admin.h` | Administrator class with full save/load cycle |
| `menus.h` | Menu function declarations |
| `menus.cpp` | Menu function implementations |
| `main.cpp` | Main driver program |

### GUI Version (`G2-7GUI/`)

**Build File**

| File | Description |
|------|-------------|
| `CMakeLists.txt` | CMake project configuration file |

**Backend (Logic Layer)** — *all implemented as header-only*

| File | Description |
|------|-------------|
| `globals.h` | Exceptions, validation helpers, shared constants |
| `date.h` | Date struct with arithmetic (today, addDays) |
| `logger.h` | Static Logger for timestamped file logging |
| `resource.h` | Abstract Resource base class + Book subclass |
| `records.h` | BorrowRecord and HoldRecord classes |
| `user.h` | Account, Membership, and User classes |
| `admin.h` | Administrator class (catalog + user management, save/load cycle) |

**Frontend (GUI Layer)**

| File | Description |
|------|-------------|
| `theme.qss` | Qt stylesheet defining global colour scheme and UI theme |
| `main.cpp` | Qt application entry point |
| `mainwindow.h/.cpp` | Login screen for Admin and User |
| `adminwindow.h/.cpp` | Full admin dashboard |
| `userwindow.h/.cpp` | User dashboard |
| `addbookdialog.h/.cpp` | Modal dialog for adding a new book |
| `adduserdialog.h/.cpp` | Modal dialog for registering a new user |
| `registerdialog.h/.cpp` | Dialog for new user self-registration |
| `overduedialog.h/.cpp` | Dialog displaying overdue books with fines |
| `userstabledialog.h/.cpp` | Sortable table of all registered users |

---

## 🖼️ GUI Overview

**Login Screen** — Role selection (Admin/User), credential validation

**Admin Window**
- Book Management: Add, update, remove, search books
- User Management: Register users, view user table
- Circulation: Issue and return books
- Holds: Approve or reject hold requests
- Reports: Overdue report, issued books, customer statements

**User Window**
- Browse and search the book catalog
- Borrow and return books
- Place and view hold requests
- View borrow history and account balance
- Deposit funds and pay outstanding fines
- Membership upgrade option

---

## 💾 Data Files *(both versions)*

The program automatically generates all data files on first run if they do not already exist. Sample data files are included in the project folder so the system can be tested immediately without manual data entry.

| File | Description |
|------|-------------|
| `books.txt` | Book catalog *(sample books included)* |
| `users.txt` | Registered users *(sample users included)* |
| `borrows.txt` | Borrowing transaction history |
| `holds.txt` | Hold/reservation records |
| `log.txt` | Timestamped system activity log *(auto-generated)* |

---

## 🧩 OOP Concepts Implemented

### 1. Inheritance
Book inherits all protected attributes and public methods from the abstract `Resource` class without redeclaring them.
```
resource.h — class Book : public Resource
```

### 2. Abstract Class & Pure Virtual Functions
`Resource` declares three pure virtual methods (`input()`, `display()`, `serialize()`) that every subclass must override.
```
resource.h — Resource::input(), display(), serialize() = 0
```

### 3. Method Overriding & Runtime Polymorphism
`Book` overrides all virtual methods using the `override` keyword. `Administrator` stores books as `vector<unique_ptr<Resource>>` and dispatches calls at runtime via the vtable.
```
resource.h — Book::input(), display(), serialize(), getGenre()
admin.h    — showAllBooks(), saveBooks(), searchByGenre()
```

### 4. Encapsulation
All attributes are private/protected. Critical state transitions locked behind dedicated methods: `closeRecord()`, `markApproved()`, `payFine()`.

### 5. Abstraction
Complex logic hidden behind simple method names — `addFine()`, `isExpired()`, `isOverdue()`, `calculateFine()` expose clean interfaces with no internal arithmetic visible to callers.

### 6. Composition
`User` owns `Account`, `Membership`, `vector<BorrowRecord>`, and `vector<HoldRecord>` via `unique_ptr`. `Administrator` owns the full catalog and user list. All owned objects destroyed automatically with the parent.
```
user.h, admin.h
```

### 7. Operator Overloading
`Book` overloads `operator==` and `operator!=` to compare by ISBN. Used in `Administrator::addBook()` for duplicate detection.
```
resource.h — Book::operator==, Book::operator!=
```

### 8. Exception Handling
Seven custom exception classes in `globals.h`, each inheriting `std::exception`:
- `InvalidInputException` / `InvalidIDException` / `NotFoundException`
- `BorrowLimitException` / `DailyLimitException`
- `OutstandingFineException` / `NotAvailableException`

Every action wrapped in try-catch — failures display formatted errors instead of crashing.

### 9. File Handling
All data persists via pipe-delimited text files. `loadAll()` enforces strict load order. Corrupted lines are caught and skipped. `Logger` writes timestamped entries at four severity levels.

### 10. Static Members
`User::nextRecordID` and `User::nextHoldID` are static counters guaranteeing globally unique IDs. Resynced on startup via `setNextRecordID()` and `setNextHoldID()`.

### 11. Object Lifetime & RAII
All records stored as `unique_ptr` inside `User` — automatic destruction with no manual `delete` anywhere in the system.

---

## ✅ What Works

**Console Version**
- Administrator login and full admin panel (21 options)
- User self-registration and login
- ANSI colour-coded menus, receipts, and error banners

**GUI Version**
- Login screen with role-based routing (Admin / User)
- Full admin panel via dedicated Qt windows and dialogs
- User dashboard with full member workflow

**Both Versions**
- Add, update, remove, and search books (keyword, author, genre, year)
- Issue and return books with automatic due-date calculation
- Per-day fine calculation ($10/day), fine payment, wallet deposit
- Hold/reserve system with priority queue, 7-day expiry, admin approval, and automatic notification on return
- Membership upgrade (Basic → Silver → Gold) with limit enforcement
- Daily borrow cap (2 books/day) and concurrent borrow limit
- Outstanding fine block on borrowing
- Overdue report, currently issued report, customer report
- Full borrow history and account statement per user
- Data persistence across sessions (all four files)
- Timestamped logging to `log.txt` at four severity levels

---

## ⚠️ Known Limitations

- No password masking — input visible on screen
- No multi-admin support — single administrator account only
- Only `Book` resource type implemented — `Journal`, `DVD` etc. architecturally supported via abstract `Resource` but not yet developed
- No database integration — plain-text file storage only
- No password encryption
- ANSI colours do not display on older Windows CMD (console version) — use Windows Terminal or VS Code Terminal

---

## 📖 References

**Books**
- *Object-Oriented Programming in C++* — Robert Lafore
- *Programming Principles and Practice Using C++* — Bjarne Stroustrup

**Websites**
- [cppreference.com](https://cppreference.com) — STL containers, unique_ptr, virtual functions
- [cplusplus.com](https://cplusplus.com) — Standard library reference
- [learncpp.com](https://learncpp.com) — Inheritance, polymorphism, RAII
- [geeksforgeeks.org](https://geeksforgeeks.org) — Concept examples and syntax reference
- [w3schools.com/cpp](https://w3schools.com/cpp) — Quick C++ reference
- [doc.qt.io/qt-6](https://doc.qt.io/qt-6) — Qt 6 official documentation
- [wiki.qt.io](https://wiki.qt.io) — Qt tutorials and examples

**Tools**
- Qt Creator Community 18.0.2 — IDE and GUI designer
- Qt 6.10.2 (MinGW 13.1.0 64-bit) — Framework and compiler
- Visual Studio Code — Code editor (console version)
- g++ (MinGW) — Compilation (console version)
- StarUML — UML class diagram design
- Claude AI — Debugging assistance, concept clarification, and documentation help
