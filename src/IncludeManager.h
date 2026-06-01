#pragma once
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>

class IncludeManager : public QDialog {
    Q_OBJECT
public:
    explicit IncludeManager(QWidget *parent = nullptr, const QStringList &includes = {});
    QStringList getIncludes() const;
private slots:
    void addInclude();
    void removeInclude();
private:
    QListWidget *m_list;
    QLineEdit *m_input;
};
