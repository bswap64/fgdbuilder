#include "BaseClassManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

BaseClassManager::BaseClassManager(QWidget *parent, const QStringList &exclusions)
    : QDialog(parent)
{
    setWindowTitle("@MaterialExclusion Paths");
    setMinimumWidth(380);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Material subdirectory paths to exclude from browser:"));
    m_list = new QListWidget;
    for (auto &e : exclusions) m_list->addItem(e);
    layout->addWidget(m_list);
    m_input = new QLineEdit;
    m_input->setPlaceholderText("e.g. debug");
    QHBoxLayout *row = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Add");
    QPushButton *removeBtn = new QPushButton("Remove");
    row->addWidget(m_input);
    row->addWidget(addBtn);
    row->addWidget(removeBtn);
    layout->addLayout(row);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(addBtn, &QPushButton::clicked, this, &BaseClassManager::addItem);
    connect(removeBtn, &QPushButton::clicked, this, &BaseClassManager::removeItem);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BaseClassManager::addItem() {
    QString s = m_input->text().trimmed();
    if (!s.isEmpty()) { m_list->addItem(s); m_input->clear(); }
}

void BaseClassManager::removeItem() {
    int row = m_list->currentRow();
    if (row >= 0) delete m_list->takeItem(row);
}

QStringList BaseClassManager::getExclusions() const {
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i) result << m_list->item(i)->text();
    return result;
}
