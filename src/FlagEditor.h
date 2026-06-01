#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QTableWidget>
#include "FGDData.h"

class FlagEditor : public QDialog {
    Q_OBJECT
public:
    explicit FlagEditor(QWidget *parent = nullptr);
    QList<QPair<QString, QPair<QString,int>>> getFlags() const;
private slots:
    void addFlag();
    void removeFlag();
private:
    QTableWidget *m_table;
    QLineEdit *m_bitInput;
    QLineEdit *m_nameInput;
};
