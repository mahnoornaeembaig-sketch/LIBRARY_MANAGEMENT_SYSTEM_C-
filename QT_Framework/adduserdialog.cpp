#include "adduserdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

AddUserDialog::AddUserDialog(Administrator &adm, QWidget *parent)
    : QDialog(parent), admin(adm)
{
    setWindowTitle("Register New User");
    setMinimumWidth(400);

    QFormLayout *form = new QFormLayout();

    idSpin        = new QSpinBox(this);
    idSpin->setRange(1, 99999);

    firstNameEdit = new QLineEdit(this);
    firstNameEdit->setPlaceholderText("Letters only, max 20 chars");

    lastNameEdit  = new QLineEdit(this);
    lastNameEdit->setPlaceholderText("Letters only, max 20 chars");

    usernameEdit  = new QLineEdit(this);
    usernameEdit->setPlaceholderText("3-15 chars, at least one letter, no spaces");

    passwordEdit  = new QLineEdit(this);
    passwordEdit->setPlaceholderText("6-20 chars, letters + numbers");
    passwordEdit->setEchoMode(QLineEdit::Password);

    addressEdit   = new QLineEdit(this);
    addressEdit->setPlaceholderText("Min 5 characters");

    emailEdit     = new QLineEdit(this);
    emailEdit->setPlaceholderText("example@domain.com");

    balanceSpin   = new QDoubleSpinBox(this);
    balanceSpin->setRange(0.0, 10000.0);
    balanceSpin->setPrefix("$ ");
    balanceSpin->setDecimals(2);

    form->addRow("User ID:",          idSpin);
    form->addRow("First Name:",       firstNameEdit);
    form->addRow("Last Name:",        lastNameEdit);
    form->addRow("Username:",         usernameEdit);
    form->addRow("Password:",         passwordEdit);
    form->addRow("Address:",          addressEdit);
    form->addRow("Email:",            emailEdit);
    form->addRow("Initial Balance:",  balanceSpin);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &AddUserDialog::onAccept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(btns);
}

void AddUserDialog::onAccept() {
    int    id       = idSpin->value();
    QString fname   = firstNameEdit->text().trimmed();
    QString lname   = lastNameEdit->text().trimmed();
    QString uname   = usernameEdit->text().trimmed();
    QString pass    = passwordEdit->text();
    QString addr    = addressEdit->text().trimmed();
    QString email   = emailEdit->text().trimmed();
    double  balance = balanceSpin->value();

    // ── Validation ───────────────────────────────────────────

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
    if (addr.length() < 5) {
        QMessageBox::warning(this, "Validation", "Address too short! Min 5 characters."); return;
    }
    if (!isValidEmail(email.toStdString())) {
        QMessageBox::warning(this, "Validation",
                             "Invalid email format!\nExample: username@domain.com"); return;
    }

    // ── Check duplicate ID and username via admin ─────────────
    try {
        // Check duplicate ID
        for (auto &u : admin.getUsers()) {
            if (u->getUserID() == id) {
                QMessageBox::warning(this, "Validation",
                                     "User ID " + QString::number(id) + " already exists!"); return;
            }
            if (u->getUsername() == uname.toStdString()) {
                QMessageBox::warning(this, "Validation",
                                     "Username '" + uname + "' is already taken!"); return;
            }
        }

        // ── Create user directly ──────────────────────────────
        auto newUser = std::make_unique<User>(
            id,
            fname.toStdString(),
            lname.toStdString(),
            uname.toStdString(),
            pass.toStdString(),
            addr.toStdString(),
            email.toStdString(),
            "",       // registration date — auto set to today inside User constructor
            balance
            );

        admin.addUserDirect(std::move(newUser));

        QMessageBox::information(this, "Success",
                                 "User '" + uname + "' registered successfully!");
        accept();

    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

