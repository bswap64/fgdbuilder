#include "FlagEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QLabel>

FlagEditor::FlagEditor(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Spawnflags Editor");
    setMinimumWidth(420);
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"Bit Value", "Display Name", "Default (0/1)"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_table);
    QHBoxLayout *row = new QHBoxLayout;
    m_bitInput = new QLineEdit; m_bitInput->setPlaceholderText("Bit value (power of 2)");
    m_nameInput = new QLineEdit; m_nameInput->setPlaceholderText("Flag name");
    QPushButton *addBtn = new QPushButton("Add");
    QPushButton *removeBtn = new QPushButton("Remove");
    row->addWidget(m_bitInput); row->addWidget(m_nameInput);
    row->addWidget(addBtn); row->addWidget(removeBtn);
    layout->addLayout(row);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(addBtn, &QPushButton::clicked, this, &FlagEditor::addFlag);
    connect(removeBtn, &QPushButton::clicked, this, &FlagEditor::removeFlag);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void FlagEditor::addFlag() {
    QString bit = m_bitInput->text().trimmed();
    QString name = m_nameInput->text().trimmed();
    if (bit.isEmpty() || name.isEmpty()) return;
    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(bit));
    m_table->setItem(row, 1, new QTableWidgetItem(name));
    m_table->setItem(row, 2, new QTableWidgetItem("0"));
    m_bitInput->clear(); m_nameInput->clear();
}

void FlagEditor::removeFlag() {
    int row = m_table->currentRow();
    if (row >= 0) m_table->removeRow(row);
}

QList<QPair<QString, QPair<QString,int>>> FlagEditor::getFlags() const {
    QList<QPair<QString, QPair<QString,int>>> result;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QString bit = m_table->item(i,0) ? m_table->item(i,0)->text() : "1";
        QString name = m_table->item(i,1) ? m_table->item(i,1)->text() : "";
        int def = m_table->item(i,2) ? m_table->item(i,2)->text().toInt() : 0;
        result.append({bit, {name, def}});
    }
    return result;
}
