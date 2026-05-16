#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <memory>
#include <ctime>
#include "globals.h"
#include "date.h"

/* ============================================================
  RESOURCE  —  Abstract Base Class
 ============================================================*/
class Resource {
protected:
    int         resourceID;
    std::string title;
    std::string author;
    std::string category;
    int         publishYear;
    int         totalAvailableCopies;
    int         maxCopies;
    Date        addedDate;

public:
    Resource()
        : resourceID(0),
          publishYear(0),
          totalAvailableCopies(0),
          maxCopies(0),
          addedDate(Date::today()) {}

    virtual ~Resource() {}

    // Prevent accidental copying of polymorphic base
    Resource(const Resource&)            = delete;
    Resource& operator=(const Resource&) = delete;

    // --- Pure virtual methods ---
    virtual void        input()           = 0;
    virtual void        display()   const = 0;
    virtual std::string serialize() const = 0;

    /*
     Returns genre if the resource has one (e.g. Book), empty string otherwise. Kept virtual on the base so callers with a Resource* can safely call it.
     */
    virtual std::string getGenre()  const { return ""; }

    /*
      Copy management  (virtual so derived classes can override
      if their borrowing rules differ, e.g. hourly journals)
     */
    virtual void borrowCopy() {
        if (totalAvailableCopies <= 0)
            throw NotAvailableException();

        totalAvailableCopies--;
    }

    virtual void returnCopy() {
        if (totalAvailableCopies >= maxCopies)
            throw InvalidInputException(); // double-return / phantom borrow — hard error

        totalAvailableCopies++;
    }

    bool checkAvailability() const {
        return totalAvailableCopies > 0;
    }

    /* ========================================================
     Getters
     ========================================================*/

    int         getResourceID()           const { return resourceID; }
    std::string getTitle()                const { return title; }
    std::string getAuthor()               const { return author; }
    std::string getCategory()             const { return category; }
    int         getPublishYear()          const { return publishYear; }
    int         getTotalAvailableCopies() const { return totalAvailableCopies; }
    int         getMaxCopies()            const { return maxCopies; }
    Date        getAddedDate()            const { return addedDate; }

    /* ========================================================
     Setters
     ========================================================*/

    void setTitle(const std::string& t)  { title  = t; }
    void setAuthor(const std::string& a) { author = a; }

    void setMaxCopies(int max) {
        int borrowedCopies = maxCopies - totalAvailableCopies;
        if (max < 0 || max < borrowedCopies)
            throw InvalidInputException();

        maxCopies = max;
    }

private:
    // Helper: returns the current year at runtime so we never hardcode it
    static int currentYear() {
        std::time_t now = std::time(nullptr);
        return 1900 + std::localtime(&now)->tm_year;
    }

protected:
    // Exposed to derived classes that need it for input validation
    static int getValidCurrentYear() { return currentYear(); }
};

/* ============================================================
  BOOK  —  Derived from Resource
 ============================================================ */
class Book : public Resource {

    /* --------------------------------------------------------
       Named constants  (for validation and input prompts)
      --------------------------------------------------------
     */
    static constexpr int MAX_TITLE_LEN    = 100;
    static constexpr int MAX_AUTHOR_LEN   =  50;
    static constexpr int MAX_CATEGORY_MIN =   2;
    static constexpr int MAX_CATEGORY_MAX =  50;
    static constexpr int DEFAULT_DUE_DAYS =  14;
    static constexpr int MAX_EDITION      =  50;
    static constexpr int MAX_PAGE_COUNT   = 10000;
    static constexpr int MIN_PUBLISH_YEAR = 1000;
    static constexpr int CIN_IGNORE_LIMIT = 1000;

    std::string ISBN;
    int         edition;
    std::string publisher;
    std::string language;
    int         pageCount;
    std::string shelfCode;
    std::string genre;
    int         dueDays;
    bool        isBestseller;

public:
    Book()
        : edition(0),
          pageCount(0),
          dueDays(DEFAULT_DUE_DAYS),
          isBestseller(false) {}

    /* ========================================================
     Parameterized constructor (used while loading from file)
    ======================================================== */

    Book(int id,
         const std::string& t,
         const std::string& auth,
         const std::string& cat,
         int year,
         int availableCopies,
         int maxCop,
         const std::string& isbn,
         int ed,
         const std::string& pub,
         const std::string& lang,
         int pages,
         const std::string& shelf,
         const std::string& gen,
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
        if (maxCop < availableCopies || maxCop < 0 || availableCopies < 0)
            throw InvalidInputException();

        resourceID           = id;
        title                = t;
        author               = auth;
        category             = cat;
        publishYear          = year;
        totalAvailableCopies = availableCopies;
        maxCopies            = maxCop;
        addedDate            = Date(addDay, addMonth, addYear);
    }

    // 18-arg constructor
    Book(int id,
         const std::string& t,
         const std::string& auth,
         const std::string& cat,
         int year,
         int availableCopies,
         const std::string& isbn,
         int ed,
         const std::string& pub,
         const std::string& lang,
         int pages,
         const std::string& shelf,
         const std::string& gen,
         int due,
         bool bestseller,
         int addDay,
         int addMonth,
         int addYear)
        : Book(id, t, auth, cat, year,
               availableCopies, availableCopies, // maxCop = availableCopies temporarily
               isbn, ed, pub, lang, pages,
               shelf, gen, due, bestseller,
               addDay, addMonth, addYear)
    {}

    /* ========================================================
     INPUT
     ======================================================== */

    void input() override {

        printHeader("ADD NEW BOOK");

        // --- Resource ID ---
        while (true) {
            std::cout << Color::CYAN << "  Resource ID     : " << Color::RESET;
            if (std::cin >> resourceID && resourceID > 0) {
                std::cin.ignore();
                break;
            }
            printError("Invalid ID! Must be positive.");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        // --- Title ---
        while (true) {
            std::cout << Color::CYAN << "  Title           : " << Color::RESET;
            std::getline(std::cin, title);

            if (title.empty()) {
                printError("Title cannot be empty!");
                continue;
            }

            bool allSpaces = true;
            for (char c : title)
                if (!std::isspace(static_cast<unsigned char>(c))) { allSpaces = false; break; }

            if (allSpaces) {
                printError("Title cannot be all spaces!");
                continue;
            }
            if (title.length() > MAX_TITLE_LEN) {
                printError("Title too long! Maximum " + std::to_string(MAX_TITLE_LEN) + " characters.");
                continue;
            }
            break;
        }

        //  Author 
        while (true) {
            std::cout << Color::CYAN << "  Author          : " << Color::RESET;
            std::getline(std::cin, author);

            if (!isValidName(author)) {
                printError("Invalid author name!");
                continue;
            }
            if (author.length() > MAX_AUTHOR_LEN) {
                printError("Author name too long!");
                continue;
            }
            break;
        }

        //  Category 
        while (true) {
            std::cout << Color::CYAN << "  Category        : " << Color::RESET;
            std::getline(std::cin, category);

            if (category.empty()) {
                printError("Category cannot be empty!");
                continue;
            }
            if (category.length() < MAX_CATEGORY_MIN ||
                category.length() > MAX_CATEGORY_MAX)
            {
                printError("Category must be " +
                           std::to_string(MAX_CATEGORY_MIN) + "-" +
                           std::to_string(MAX_CATEGORY_MAX) + " chars.");
                continue;
            }
            break;
        }

        //  Publish Year 
       
        while (true) {
            std::cout << Color::CYAN << "  Publish Year    : " << Color::RESET;
            if (std::cin >> publishYear &&
                publishYear >= MIN_PUBLISH_YEAR &&
                publishYear <= getValidCurrentYear())
            {
                std::cin.ignore();
                break;
            }
            printError("Invalid publish year! Must be between " +
                       std::to_string(MIN_PUBLISH_YEAR) + " and " +
                       std::to_string(getValidCurrentYear()) + ".");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        //  Copies 
        while (true) {
            std::cout << Color::CYAN << "  Total Copies    : " << Color::RESET;
            if (std::cin >> totalAvailableCopies && totalAvailableCopies >= 1) {
                maxCopies = totalAvailableCopies;
                std::cin.ignore();
                break;
            }
            printError("Invalid copies!");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        //  ISBN 
        while (true) {
            std::cout << Color::CYAN << "  ISBN (10 or 13) : " << Color::RESET;
            std::cin >> ISBN;

            if (ISBN.length() != 10 && ISBN.length() != 13) {
                printError("ISBN must be 10 or 13 digits!");
                continue;
            }

            bool allDigits = true;
            for (char c : ISBN)
                if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }

            if (!allDigits) {
                printError("ISBN must contain digits only!");
                continue;
            }
            std::cin.ignore();
            break;
        }

        //  Edition 
        while (true) {
            std::cout << Color::CYAN << "  Edition         : " << Color::RESET;
            if (std::cin >> edition && edition > 0 && edition <= MAX_EDITION) {
                std::cin.ignore();
                break;
            }
            printError("Invalid edition! Must be 1-" + std::to_string(MAX_EDITION) + ".");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        //  Publisher 
        while (true) {
            std::cout << Color::CYAN << "  Publisher       : " << Color::RESET;
            std::getline(std::cin, publisher);
            if (!isValidPublisher(publisher)) {
                printError("Invalid publisher!");
                continue;
            }
            break;
        }

        //  Language ---
        while (true) {
            std::cout << Color::CYAN << "  Language        : " << Color::RESET;
            std::getline(std::cin, language);
            if (!isValidLanguage(language)) {
                printError("Invalid language!");
                continue;
            }
            break;
        }

        //  Page Count 
        while (true) {
            std::cout << Color::CYAN << "  Page Count      : " << Color::RESET;
            if (std::cin >> pageCount && pageCount > 0 && pageCount <= MAX_PAGE_COUNT) {
                std::cin.ignore();
                break;
            }
            printError("Invalid page count! Must be 1-" + std::to_string(MAX_PAGE_COUNT) + ".");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        //  Shelf Code 
        while (true) {
            std::cout << Color::CYAN << "  Shelf Code      : " << Color::RESET;
            std::getline(std::cin, shelfCode);
            if (!isValidShelfCode(shelfCode)) {
                printError("Invalid shelf code!");
                continue;
            }
            break;
        }

        //  Genre 
        while (true) {
            std::cout << Color::CYAN << "  Genre           : " << Color::RESET;
            std::getline(std::cin, genre);
            if (!isValidGenre(genre)) {
                printError("Invalid genre!");
                continue;
            }
            break;
        }

        //  Due Days 
        while (true) {
            std::cout << Color::CYAN << "  Due Days        : " << Color::RESET;
            if (std::cin >> dueDays && dueDays > 0) {
                std::cin.ignore();
                break;
            }
            printError("Invalid due days!");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        // Bestseller
        while (true) {
            int temp;
            std::cout << Color::CYAN << "  Bestseller(1/0) : " << Color::RESET;
            if (std::cin >> temp && (temp == 0 || temp == 1)) {
                isBestseller = static_cast<bool>(temp);
                std::cin.ignore();
                break;
            }
            printError("Enter 1 or 0 only!");
            std::cin.clear();
            std::cin.ignore(CIN_IGNORE_LIMIT, '\n');
        }

        addedDate = Date::today();
    }

    /* ========================================================
     DISPLAY
     ======================================================== */

    void display() const override {
        printDoubleLine(50);
        std::cout << Color::BG_BLUE << Color::BWHITE << Color::BOLD
                  << "  BOOK #" << resourceID << " - " << title
                  << Color::RESET << "\n";
        printLine('-', 50);

        printField("Author:",    author);
        printField("Category:",  category);
        printField("Year:",      std::to_string(publishYear));
        printField("ISBN:",      ISBN);
        printField("Edition:",   std::to_string(edition));
        printField("Publisher:", publisher);
        printField("Language:",  language);
        printField("Pages:",     std::to_string(pageCount));
        printField("Genre:",     genre);
        printField("Shelf:",     shelfCode);
        printField("Due Days:",  std::to_string(dueDays));
        printField("Max Copies:",std::to_string(maxCopies));   // helpful for staff

        std::string avail =
            checkAvailability()
            ? (Color::BGREEN +
               std::string("Available (") +
               std::to_string(totalAvailableCopies) +
               " / " + std::to_string(maxCopies) +
               " copies)" + Color::RESET)
            : (Color::BRED + std::string("Not Available") + Color::RESET);

        printFieldColored("Status:", avail, "");

        if (isBestseller)
            std::cout << Color::BYELLOW << "  * BESTSELLER" << Color::RESET << "\n";

        printLine('-', 50);
    }

    /*========================================================
     SERIALIZE
     ======================================================== */

    std::string serialize() const override {
        return
            std::to_string(resourceID)           + "|" +
            title                                + "|" +
            author                               + "|" +
            category                             + "|" +
            std::to_string(publishYear)          + "|" +
            std::to_string(maxCopies)            + "|" +
            std::to_string(totalAvailableCopies) + "|" +
            ISBN                                 + "|" +
            std::to_string(edition)              + "|" +
            publisher                            + "|" +
            language                             + "|" +
            std::to_string(pageCount)            + "|" +
            shelfCode                            + "|" +
            genre                                + "|" +
            std::to_string(dueDays)              + "|" +
            std::to_string(isBestseller)         + "|" +
            std::to_string(addedDate.day)        + "|" +
            std::to_string(addedDate.month)      + "|" +
            std::to_string(addedDate.year);
    }

    // ========================================================
    // Comparison
    // Two Books are considered identical if their ISBN matches.
    // Note: different editions share an ISBN prefix but differ
    // in the last digit — treat as same logical work.
    // ========================================================

    bool operator==(const Book& other) const { return ISBN == other.ISBN; }
    bool operator!=(const Book& other) const { return ISBN != other.ISBN; }

    /* ========================================================
     Getters
     ======================================================== */

    std::string getISBN()      const { return ISBN;      }
    int         getEdition()   const { return edition;   }
    std::string getPublisher() const { return publisher; }
    int         getDueDays()   const { return dueDays;   }
    std::string getGenre()     const override { return genre; }

    /*========================================================
     Setters
     ======================================================== */

    void setISBN(const std::string& n)  { ISBN      = n;  }
    void setEdition(int ed)             { edition   = ed; }
    void setPublisher(const std::string& p) { publisher = p; }
};

#endif // RESOURCE_H