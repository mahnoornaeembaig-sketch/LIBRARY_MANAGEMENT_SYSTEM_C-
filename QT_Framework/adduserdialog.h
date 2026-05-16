#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "admin.h"

class AddUserDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddUserDialog(Administrator &admin, QWidget *parent = nullptr);

private slots:
    void onAccept();

private:
    Administrator &admin;

    QSpinBox       *idSpin;
    QLineEdit      *firstNameEdit;
    QLineEdit      *lastNameEdit;
    QLineEdit      *usernameEdit;
    QLineEdit      *passwordEdit;
    QLineEdit      *addressEdit;
    QLineEdit      *emailEdit;
    QDoubleSpinBox *balanceSpin;
};

#endif // ADDUSERDIALOG_H
