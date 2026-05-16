#include "mainwindow.h"
#include "adminwindow.h"
#include "userwindow.h"
#include "registerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Library Management System");
    setFixedSize(400, 300);

    admin.loadAll();

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setSpacing(16);
    layout->setContentsMargins(48, 48, 48, 48);

    QLabel *title = new QLabel("Library Management System", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout->addWidget(title);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Username");
    layout->addWidget(usernameEdit);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QHBoxLayout *btnRow = new QHBoxLayout();
    adminBtn = new QPushButton("Admin Login", this);
    userBtn  = new QPushButton("User Login",  this);
    // wherever you create adminBtn and userBtn, add:
    registerBtn = new QPushButton("New User? Register Here", this);
    registerBtn->setStyleSheet("color: #2980b9; border: none; text-decoration: underline;");

    // add it to your layout after the login buttons
    btnRow->addWidget(adminBtn);
    btnRow->addWidget(userBtn);
    layout->addLayout(btnRow);

    connect(adminBtn, &QPushButton::clicked, this, &MainWindow::onAdminLogin);
    connect(userBtn,  &QPushButton::clicked, this, &MainWindow::onUserLogin);
    connect(registerBtn, &QPushButton::clicked, this, &MainWindow::onRegister);
}

MainWindow::~MainWindow() {}

void MainWindow::onAdminLogin() {
    QString uname = usernameEdit->text().trimmed();
    QString pass  = passwordEdit->text();

    if (uname.toStdString() == admin.getAdminName() &&
        admin.verifyCredentials(pass.toStdString())) {
        Logger::log("Admin logged in.", Logger::ACTION);
        AdminWindow *w = new AdminWindow(admin, this);
        w->show();
        hide();
    } else {
        QMessageBox::warning(this, "Error", "Incorrect username or password.");
        Logger::log("Failed admin login attempt.", Logger::WARNING);
    }
}

void MainWindow::onUserLogin() {
    QString uname = usernameEdit->text().trimmed();
    QString pass  = passwordEdit->text();

    if (uname.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password.");
        return;
    }

    try {
        User *u = admin.findUserByUsername(uname.toStdString());

        if (u->verifyPassword(pass.toStdString())) {
            Logger::logUser(u->getUserID(), "Logged in.");

            // UserWindow takes (User*, Administrator&, QWidget*)
            // It handles borrow/return/search internally — no signals needed for those
            UserWindow *w = new UserWindow(u, admin, this);

            // Only wire logout so MainWindow can reappear
            connect(w, &UserWindow::sig_logout, this, [this, w]() {
                w->close();
                this->show();
            });

            w->show();
            hide();

        } else {
            QMessageBox::warning(this, "Error", "Incorrect password.");
            Logger::log("Failed login for user: " + uname.toStdString(), Logger::WARNING);
        }

    } catch (exception &e) {
        QMessageBox::critical(this, "Error", e.what());
        Logger::log("Login attempt for unknown user: " + uname.toStdString(), Logger::WARNING);
    }
}
void MainWindow::onRegister() {
    RegisterDialog dlg(admin, this);
    dlg.exec();
}
