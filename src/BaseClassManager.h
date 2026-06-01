#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>

class BaseClassManager : public QDialog {
    Q_OBJECT
public:
    explicit BaseClassManager(QWidget *parent = nullptr, const QStringList &exclusions = {});
    QStringList getExclusions() const;
private slots:
    void addItem();
    void removeItem();
private:
    QListWidget *m_list;
    QLineEdit *m_input;
};
