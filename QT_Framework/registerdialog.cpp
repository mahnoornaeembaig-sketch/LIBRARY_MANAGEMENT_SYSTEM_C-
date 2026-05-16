#include "registerdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

RegisterDialog::RegisterDialog(Administrator &adm, QWidget *parent)
    : QDialog(parent), admin(adm)
{
    setWindowTitle("Create Account");
    setMinimumWidth(420);

    QFormLayout *form = new QFormLayout();

    firstNameEdit = new QLineEdit(this);
    firstNameEdit->setPlaceholderText("Letters only, max 20 chars");

    lastNameEdit = new QLineEdit(this);
    lastNameEdit->setPlaceholderText("Letters only, max 20 chars");

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("3-15 chars, at least one letter, no spaces");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("6-20 chars, letters + numbers");
    passwordEdit->setEchoMode(QLineEdit::Password);

    confirmPassEdit = new QLineEdit(this);
    confirmPassEdit->setPlaceholderText("Re-enter password");
    confirmPassEdit->setEchoMode(QLineEdit::Password);

    addressEdit = new QLineEdit(this);
    addressEdit->setPlaceholderText("Min 5 characters");

    emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("example@domain.com");

    form->addRow("First Name:",       firstNameEdit);
    form->addRow("Last Name:",        lastNameEdit);
    form->addRow("Username:",         usernameEdit);
    form->addRow("Password:",         passwordEdit);
    form->addRow("Confirm Password:", confirmPassEdit);
    form->addRow("Address:",          addressEdit);
    form->addRow("Email:",            emailEdit);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &RegisterDialog::onAccept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addWidget(btns);
}

void RegisterDialog::onAccept()
{
    QString fname   = firstNameEdit->text().trimmed();
    QString lname   = lastNameEdit->text().trimmed();
    QString uname   = usernameEdit->text().trimmed();
    QString pass    = passwordEdit->text();
    QString confirm = confirmPassEdit->text();
    QString addr    = addressEdit->text().trimmed();
    QString email   = emailEdit->text().trimmed();

    // ── Validation ────────────────────────────────────────────

    if (fname.isEmpty() || !isValidName(fname.toStdString())) {
        QMessageBox::warning(this, "Validation", "Invalid first name! Letters only."); return;
    }
    if (fname.length() > 20) {
        QMessageBox::warning(this, "Validation", "First name too long! Max 20 characters."); return;
    }
    if (lname.isEmpty() || !isValidName(lname.toStdString())) {
        QMessageBox::warning(this, "Validation", "Invalid last name! Letters only."); return;
    }
    if (lname.length() > 20) {
        QMessageBox::warning(this, "Validation", "Last name too long! Max 20 characters."); return;
    }
    if (!isValidUsername(uname.toStdString())) {
        QMessageBox::warning(this, "Validation",
                             "Invalid username!\nMust be 3-15 characters, at least one letter, no spaces."); return;
    }
    if (!isValidPassword(pass.toStdString())) {
        QMessageBox::warning(this, "Validation",
                             "Weak password!\nMust be 6-20 characters with letters and numbers."); return;
    }
    if (pass != confirm) {
        QMessageBox::warning(this, "Validation", "Passwords do not match!"); return;
    }
    if (addr.length() < 5) {
        QMessageBox::warning(this, "Validation", "Address too short! Min 5 characters."); return;
    }
    if (!isValidEmail(email.toStdString())) {
        QMessageBox::warning(this, "Validation",
                             "Invalid email format!\nExample: username@domain.com"); return;
    }

    // ── Duplicate username check ───────────────────────────────
    for (auto &u : admin.getUsers()) {
        if (u->getUsername() == uname.toStdString()) {
            QMessageBox::warning(this, "Validation",
                                 "Username '" + uname + "' is already taken!"); return;
        }
    }

    // ── Auto-generate ID (max existing + 1) ───────────────────
    int newID = 1;
    for (auto &u : admin.getUsers())
        if (u->getUserID() >= newID)
            newID = u->getUserID() + 1;

    // ── Create user ───────────────────────────────────────────
    try {
        auto newUser = std::make_unique<User>(
            newID,
            fname.toStdString(),
            lname.toStdString(),
            uname.toStdString(),
            pass.toStdString(),
            addr.toStdString(),
            email.toStdString(),
            "",    // registration date — auto set to today inside User
            0.0    // initial balance = $0
            );

        admin.addUserDirect(std::move(newUser));

        QMessageBox::information(this, "Account Created",
                                 "Welcome, " + fname + "!\n"
                                                       "Your account has been created. You can now log in.");
        accept();

    } catch (std::exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}
