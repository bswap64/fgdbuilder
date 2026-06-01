#include "MapSizeDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>

MapSizeDialog::MapSizeDialog(QWidget *parent, bool enabled, int mn, int mx)
    : QDialog(parent)
{
    setWindowTitle("@mapsize Settings");
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_enabled = new QCheckBox("Include @mapsize in output");
    m_enabled->setChecked(enabled);
    layout->addWidget(m_enabled);
    QFormLayout *form = new QFormLayout;
    m_min = new QSpinBox;
    m_min->setRange(-1000000, 0);
    m_min->setValue(mn);
    m_max = new QSpinBox;
    m_max->setRange(0, 1000000);
    m_max->setValue(mx);
    form->addRow("Min:", m_min);
    form->addRow("Max:", m_max);
    layout->addLayout(form);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool MapSizeDialog::isEnabled() const { return m_enabled->isChecked(); }
int MapSizeDialog::getMin() const { return m_min->value(); }
int MapSizeDialog::getMax() const { return m_max->value(); }
