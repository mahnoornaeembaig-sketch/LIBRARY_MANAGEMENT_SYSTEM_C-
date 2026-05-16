#ifndef USERSTABLEDIALOG_H
#define USERSTABLEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include "admin.h"

class UsersTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit UsersTableDialog(Administrator& admin, QWidget* parent = nullptr);

private:
    QTableWidget* userTable;
    void populate(Administrator& admin);
};

#endif
