#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "admin.h"

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(Administrator &admin, QWidget *parent = nullptr);

private slots:
    void onAccept();

private:
    Administrator &admin;
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPassEdit;
    QLineEdit *addressEdit;
    QLineEdit *emailEdit;
};

#endif // REGISTERDIALOG_H
