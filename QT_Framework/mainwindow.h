#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "admin.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdminLogin();
    void onUserLogin();
    void onRegister();      // ← add this

private:
    Administrator admin;
    QLineEdit   *usernameEdit;
    QLineEdit   *passwordEdit;
    QPushButton *adminBtn;
    QPushButton *userBtn;
    QPushButton *registerBtn;  // ← add this
};

#endif // MAINWINDOW_H
