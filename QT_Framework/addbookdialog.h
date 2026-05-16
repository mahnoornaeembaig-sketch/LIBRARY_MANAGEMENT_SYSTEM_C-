#ifndef ADDBOOKDIALOG_H
#define ADDBOOKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <memory>
#include "resource.h"

class AddBookDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddBookDialog(QWidget *parent = nullptr);
    std::unique_ptr<Book> getBook();

private slots:
    void onAccept();

private:
    QSpinBox  *idSpin, *yearSpin, *copiesSpin;
    QSpinBox  *editionSpin, *pagesSpin, *dueDaysSpin;
    QLineEdit *titleEdit, *authorEdit, *categoryEdit;
    QLineEdit *isbnEdit, *publisherEdit, *languageEdit;
    QLineEdit *shelfEdit, *genreEdit;
    QCheckBox *bestsellerCheck;
};

#endif // ADDBOOKDIALOG_H
