#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <memory>
#include "globals.h"
#include "date.h"

using namespace std;

// ============================================================
//  RESOURCE  —  Abstract Base Class
// ============================================================
class Resource {
protected:
    int    resourceID;
    string title;
    string author;
    string category;
    int    publishYear;
    int    totalAvailableCopies;
    int    maxCopies;
    Date   addedDate;

public:
    Resource()
        : resourceID(0),
        publishYear(0),
        totalAvailableCopies(0),
        maxCopies(0),
        addedDate(Date::today()) {}

    virtual ~Resource() {}

    // --- Pure virtual methods ---
    virtual void   input()           = 0;
    virtual void   display()   const = 0;
    virtual string serialize() const = 0;
    virtual string getGenre()  const { return ""; }

    // ========================================================
    // Copy management
    // ========================================================

    void borrowCopy() {
        if (totalAvailableCopies <= 0)
            throw NotAvailableException();

        totalAvailableCopies--;
    }

    void returnCopy() {
        if (totalAvailableCopies < maxCopies)
            totalAvailableCopies++;
        else
            printWarning("Return rejected: copies already at maximum.");
    }

    bool checkAvailability() const {
        return totalAvailableCopies > 0;
    }

    // ========================================================
    // Getters
    // ========================================================

    int    getResourceID()           const { return resourceID; }
    string getTitle()                const { return title; }
    string getAuthor()               const { return author; }
    string getCategory()             const { return category; }
    int    getPublishYear()          const { return publishYear; }
    int    getTotalAvailableCopies() const { return totalAvailableCopies; }
    int    getMaxCopies()            const { return maxCopies; }
    Date   getAddedDate()            const { return addedDate; }

    // ========================================================
    // Setters
    // ========================================================

    void setTitle(const string& t) {
        title = t;
    }

    void setAuthor(const string& a) {
        author = a;
    }

    // Used after loading from file
    void setMaxCopies(int max) {
        if (max < totalAvailableCopies)
            throw InvalidInputException();

        maxCopies = max;
    }
};

// ============================================================
//  BOOK  —  Derived from Resource
// ============================================================
class Book : public Resource {
private:
    string ISBN;
    int    edition;
    string publisher;
    string language;
    int    pageCount;
    string shelfCode;
    string genre;
    int    dueDays;
    bool   isBestseller;

public:
    Book()
        : edition(0),
        pageCount(0),
        dueDays(14),
        isBestseller(false) {}

    // ========================================================
    // Parameterized constructor (used while loading from file)
    // copies = CURRENT AVAILABLE COPIES
    // maxCopies restored later using setMaxCopies()
    // ========================================================

    Book(int id,
         const string& t,
         const string& auth,
         const string& cat,
         int year,
         int copies,
         const string& isbn,
         int ed,
         const string& pub,
         const string& lang,
         int pages,
         const string& shelf,
         const string& gen,
         int due,
         bool bestseller,
         int addDay,
         int addMonth,
         int addYear)
        : ISBN(isbn),
        edition(ed),
        publisher(pub),
        language(lang),
        pageCount(pages),
        shelfCode(shelf),
        genre(gen),
        dueDays(due),
        isBestseller(bestseller)
    {
        resourceID           = id;
        title                = t;
        author               = auth;
        category             = cat;
        publishYear          = year;

        // CURRENT AVAILABLE COPIES
        totalAvailableCopies = copies;

        // Temporary assignment
        // Real max restored via setMaxCopies()
        maxCopies = copies;

        addedDate = Date(addDay, addMonth, addYear);
    }

    // ========================================================
    // INPUT
    // ========================================================

    void input() override {

        printHeader("ADD NEW BOOK");

        // ----------------------------------------------------
        // Resource ID
        // ----------------------------------------------------

        while (true) {
            cout << Color::CYAN
                 << "  Resource ID     : "
                 << Color::RESET;

            if (cin >> resourceID && resourceID > 0) {
                cin.ignore();
                break;
            }

            printError("Invalid ID! Must be positive.");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // Title
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Title           : "
                 << Color::RESET;

            getline(cin, title);

            if (title.empty()) {
                printError("Title cannot be empty!");
                continue;
            }

            bool allSpaces = true;

            for (char c : title) {
                if (!isspace(c)) {
                    allSpaces = false;
                    break;
                }
            }

            if (allSpaces) {
                printError("Title cannot be all spaces!");
                continue;
            }

            if (title.length() > 100) {
                printError("Title too long! Maximum 100 characters.");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Author
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Author          : "
                 << Color::RESET;

            getline(cin, author);

            if (!isValidName(author)) {
                printError("Invalid author name!");
                continue;
            }

            if (author.length() > 50) {
                printError("Author name too long!");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Category
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Category        : "
                 << Color::RESET;

            getline(cin, category);

            if (category.empty()) {
                printError("Category cannot be empty!");
                continue;
            }

            if (category.length() < 2 || category.length() > 50) {
                printError("Category must be 2-50 chars.");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Publish Year
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Publish Year    : "
                 << Color::RESET;

            if (cin >> publishYear &&
                publishYear >= 1000 &&
                publishYear <= 2026)
            {
                cin.ignore();
                break;
            }

            printError("Invalid publish year!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // Copies
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Total Copies    : "
                 << Color::RESET;

            if (cin >> totalAvailableCopies &&
                totalAvailableCopies >= 1)
            {
                maxCopies = totalAvailableCopies;
                cin.ignore();
                break;
            }

            printError("Invalid copies!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // ISBN
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  ISBN (10 or 13) : "
                 << Color::RESET;

            cin >> ISBN;

            if (ISBN.length() != 10 &&
                ISBN.length() != 13)
            {
                printError("ISBN must be 10 or 13 digits!");
                continue;
            }

            bool allDigits = true;

            for (char c : ISBN) {
                if (!isdigit(c)) {
                    allDigits = false;
                    break;
                }
            }

            if (!allDigits) {
                printError("ISBN must contain digits only!");
                continue;
            }

            cin.ignore();
            break;
        }

        // ----------------------------------------------------
        // Edition
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Edition         : "
                 << Color::RESET;

            if (cin >> edition &&
                edition > 0 &&
                edition <= 50)
            {
                cin.ignore();
                break;
            }

            printError("Invalid edition!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // Publisher
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Publisher       : "
                 << Color::RESET;

            getline(cin, publisher);

            if (!isValidPublisher(publisher)) {
                printError("Invalid publisher!");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Language
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Language        : "
                 << Color::RESET;

            getline(cin, language);

            if (!isValidLanguage(language)) {
                printError("Invalid language!");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Page Count
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Page Count      : "
                 << Color::RESET;

            if (cin >> pageCount &&
                pageCount > 0 &&
                pageCount <= 10000)
            {
                cin.ignore();
                break;
            }

            printError("Invalid page count!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // Shelf Code
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Shelf Code      : "
                 << Color::RESET;

            getline(cin, shelfCode);

            if (!isValidShelfCode(shelfCode)) {
                printError("Invalid shelf code!");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Genre
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Genre           : "
                 << Color::RESET;

            getline(cin, genre);

            if (!isValidGenre(genre)) {
                printError("Invalid genre!");
                continue;
            }

            break;
        }

        // ----------------------------------------------------
        // Due Days
        // ----------------------------------------------------

        while (true) {

            cout << Color::CYAN
                 << "  Due Days        : "
                 << Color::RESET;

            if (cin >> dueDays && dueDays > 0) {
                cin.ignore();
                break;
            }

            printError("Invalid due days!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        // ----------------------------------------------------
        // Bestseller
        // ----------------------------------------------------

        while (true) {

            int temp;

            cout << Color::CYAN
                 << "  Bestseller(1/0) : "
                 << Color::RESET;

            if (cin >> temp &&
                (temp == 0 || temp == 1))
            {
                isBestseller = temp;
                cin.ignore();
                break;
            }

            printError("Enter 1 or 0 only!");
            cin.clear();
            cin.ignore(1000, '\n');
        }

        addedDate = Date::today();
    }

    // ========================================================
    // DISPLAY
    // ========================================================

    void display() const override {

        printDoubleLine(50);

        cout << Color::BG_BLUE
             << Color::BWHITE
             << Color::BOLD
             << "  BOOK #" << resourceID
             << " - "
             << title
             << Color::RESET
             << "\n";

        printLine('-', 50);

        printField("Author:", author);
        printField("Category:", category);
        printField("Year:", to_string(publishYear));
        printField("ISBN:", ISBN);
        printField("Edition:", to_string(edition));
        printField("Publisher:", publisher);
        printField("Language:", language);
        printField("Pages:", to_string(pageCount));
        printField("Genre:", genre);
        printField("Shelf:", shelfCode);
        printField("Due Days:", to_string(dueDays));

        string avail =
            checkAvailability()
                ? (Color::BGREEN +
                   string("Available (") +
                   to_string(totalAvailableCopies) +
                   " copies)" +
                   Color::RESET)
                : (Color::BRED +
                   string("Not Available") +
                   Color::RESET);

        printFieldColored("Status:", avail, "");

        if (isBestseller) {
            cout << Color::BYELLOW
                 << "  * BESTSELLER"
                 << Color::RESET
                 << "\n";
        }

        printLine('-', 50);
    }

    // ========================================================
    // SERIALIZE
    // ========================================================

    string serialize() const override {

        return
            to_string(resourceID) + "|" +
            title + "|" +
            author + "|" +
            category + "|" +
            to_string(publishYear) + "|" +
            to_string(maxCopies) + "|" +
            to_string(totalAvailableCopies) + "|" +
            ISBN + "|" +
            to_string(edition) + "|" +
            publisher + "|" +
            language + "|" +
            to_string(pageCount) + "|" +
            shelfCode + "|" +
            genre + "|" +
            to_string(dueDays) + "|" +
            to_string(isBestseller) + "|" +
            to_string(addedDate.day) + "|" +
            to_string(addedDate.month) + "|" +
            to_string(addedDate.year);
    }

    // ========================================================
    // Other methods
    // ========================================================

    string getGenre() const override {
        return genre;
    }

    bool operator==(const Book& other) const {
        return ISBN == other.ISBN;
    }

    bool operator!=(const Book& other) const {
        return ISBN != other.ISBN;
    }

    // ========================================================
    // Getters
    // ========================================================

    string getISBN() const {
        return ISBN;
    }

    int getEdition() const {
        return edition;
    }

    string getPublisher() const {
        return publisher;
    }

    int getDueDays() const {
        return dueDays;
    }

    // ========================================================
    // Setters
    // ========================================================

    void setISBN(const string& n) {
        ISBN = n;
    }

    void setEdition(int ed) {
        edition = ed;
    }

    void setPublisher(const string& p) {
        publisher = p;
    }
    void setCategory  (const string& c) { category   = c; }
    void setPublishYear(int y)           { publishYear = y; }

    // Adjusts both maxCopies and totalAvailableCopies together.
    // Throws InvalidInputException if newMax < currently-borrowed count.
    void updateTotalCopies(int newMax) {
        int borrowed = maxCopies - totalAvailableCopies;
        if (newMax < borrowed) throw InvalidInputException();
        maxCopies            = newMax;
        totalAvailableCopies = newMax - borrowed;
    }


    // ── 2. Inside class Book — add after the existing getISBN/getEdition/getPublisher/getDueDays getters ──

    string getLanguage()     const { return language;    }
    int    getPageCount()    const { return pageCount;   }
    string getShelfCode()    const { return shelfCode;   }
    bool   getIsBestseller() const { return isBestseller;}


    // ── 3. Inside class Book — add after the existing setISBN/setEdition/setPublisher setters ──

    void setLanguage   (const string& l) { language     = l;  }
    void setPageCount  (int p)           { pageCount    = p;  }
    void setShelfCode  (const string& s) { shelfCode    = s;  }
    void setGenre      (const string& g) { genre        = g;  }
    void setDueDays    (int d)           { dueDays      = d;  }
    void setBestseller (bool bs)         { isBestseller = bs; }
};

#endif // RESOURCE_H
