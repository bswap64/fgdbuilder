#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include "FGDData.h"

class IOEditor : public QDialog {
    Q_OBJECT
public:
    explicit IOEditor(QWidget *parent = nullptr, const FGDIO &io = FGDIO());
    FGDIO getIO() const;
private:
    void setupUi();
    QRadioButton *m_input;
    QRadioButton *m_output;
    QLineEdit *m_name;
    QComboBox *m_type;
    QLineEdit *m_description;
};
