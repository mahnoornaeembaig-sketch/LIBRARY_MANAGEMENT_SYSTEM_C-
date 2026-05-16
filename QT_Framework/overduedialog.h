#ifndef OVERDUEDIALOG_H
#define OVERDUEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include "admin.h"

class OverdueDialog : public QDialog {
    Q_OBJECT

public:
    explicit OverdueDialog(Administrator &admin, QWidget *parent = nullptr);

private:
    void populate(Administrator &admin);

    QTableWidget *table;
    QLabel       *summaryLabel;
};

#endif // OVERDUEDIALOG_H
