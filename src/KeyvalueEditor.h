#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QStackedWidget>
#include "FGDData.h"

class KeyvalueEditor : public QDialog {
    Q_OBJECT
public:
    explicit KeyvalueEditor(QWidget *parent = nullptr, const FGDKeyvalue &kv = FGDKeyvalue());
    FGDKeyvalue getKeyvalue() const;
private slots:
    void onTypeChanged(const QString &type);
    void addChoiceItem();
    void removeChoiceItem();
    void addFlagItem();
    void removeFlagItem();
private:
    void setupUi();
    QLineEdit *m_name;
    QComboBox *m_type;
    QLineEdit *m_displayName;
    QLineEdit *m_defaultValue;
    QLineEdit *m_description;
    QCheckBox *m_readOnly;
    QCheckBox *m_report;
    QTableWidget *m_choicesTable;
    QTableWidget *m_flagsTable;
    QStackedWidget *m_stack;
};
