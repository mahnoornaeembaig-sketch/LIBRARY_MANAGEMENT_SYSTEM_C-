#include "addbookdialog.h"
#include "date.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

AddBookDialog::AddBookDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Add New Book");
    setMinimumWidth(420);

    QFormLayout *form = new QFormLayout();

    idSpin      = new QSpinBox(this); idSpin->setRange(1, 99999);
    yearSpin    = new QSpinBox(this); yearSpin->setRange(1000, 2026); yearSpin->setValue(2024);
    copiesSpin  = new QSpinBox(this); copiesSpin->setRange(1, 999);
    editionSpin = new QSpinBox(this); editionSpin->setRange(1, 50);
    pagesSpin   = new QSpinBox(this); pagesSpin->setRange(1, 10000);
    dueDaysSpin = new QSpinBox(this); dueDaysSpin->setRange(1, 365); dueDaysSpin->setValue(14);

    titleEdit     = new QLineEdit(this);
    authorEdit    = new QLineEdit(this);
    categoryEdit  = new QLineEdit(this);
    isbnEdit      = new QLineEdit(this);
    publisherEdit = new QLineEdit(this);
    languageEdit  = new QLineEdit(this);
    shelfEdit     = new QLineEdit(this);
    genreEdit     = new QLineEdit(this);
    bestsellerCheck = new QCheckBox(this);

    form->addRow("Resource ID:",  idSpin);
    form->addRow("Title:",        titleEdit);
    form->addRow("Author:",       authorEdit);
    form->addRow("Category:",     categoryEdit);
    form->addRow("Publish Year:", yearSpin);
    form->addRow("Copies:",       copiesSpin);
    form->addRow("ISBN:",         isbnEdit);
    form->addRow("Edition:",      editionSpin);
    form->addRow("Publisher:",    publisherEdit);
    form->addRow("Language:",     languageEdit);
    form->addRow("Pages:",        pagesSpin);
    form->addRow("Shelf Code:",   shelfEdit);
    form->addRow("Genre:",        genreEdit);
    form->addRow("Due Days:",     dueDaysSpin);
    form->addRow("Bestseller:",   bestsellerCheck);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &AddBookDialog::onAccept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(btns);
}

void AddBookDialog::onAccept() {
    // Title check
    if (titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Title cannot be empty.");
        return;
    }
    // Author check
    if (authorEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Author cannot be empty.");
        return;
    }
    // ISBN check
    QString isbn = isbnEdit->text().trimmed();
    if (isbn.length() != 10 && isbn.length() != 13) {
        QMessageBox::warning(this, "Validation", "ISBN must be exactly 10 or 13 digits.");
        return;
    }
    for (QChar c : isbn) {
        if (!c.isDigit()) {
            QMessageBox::warning(this, "Validation", "ISBN must contain digits only.");
            return;
        }
    }
    // Publisher check
    if (publisherEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Publisher cannot be empty.");
        return;
    }
    // Shelf code check
    if (shelfEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Shelf code cannot be empty.");
        return;
    }
    // Genre check
    if (genreEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Genre cannot be empty.");
        return;
    }

    accept();
}

std::unique_ptr<Book> AddBookDialog::getBook() {
    Date today = Date::today();
    return std::make_unique<Book>(
        idSpin->value(),
        titleEdit->text().trimmed().toStdString(),
        authorEdit->text().trimmed().toStdString(),
        categoryEdit->text().trimmed().toStdString(),
        yearSpin->value(),
        copiesSpin->value(),
        isbnEdit->text().trimmed().toStdString(),
        editionSpin->value(),
        publisherEdit->text().trimmed().toStdString(),
        languageEdit->text().trimmed().toStdString(),
        pagesSpin->value(),
        shelfEdit->text().trimmed().toStdString(),
        genreEdit->text().trimmed().toStdString(),
        dueDaysSpin->value(),
        bestsellerCheck->isChecked(),
        today.day,
        today.month,
        today.year
        );
}
