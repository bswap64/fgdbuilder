#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>

class MapSizeDialog : public QDialog {
    Q_OBJECT
public:
    explicit MapSizeDialog(QWidget *parent = nullptr, bool enabled = false, int mn = -16384, int mx = 16384);
    bool isEnabled() const;
    int getMin() const;
    int getMax() const;
private:
    QCheckBox *m_enabled;
    QSpinBox *m_min;
    QSpinBox *m_max;
};
