#include "IncludeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>

IncludeManager::IncludeManager(QWidget *parent, const QStringList &includes)
    : QDialog(parent)
{
    setWindowTitle("Manage @include Files");
    setMinimumWidth(380);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("FGD files to include (e.g. base.fgd):"));
    m_list = new QListWidget;
    for (auto &inc : includes) m_list->addItem(inc);
    layout->addWidget(m_list);
    m_input = new QLineEdit;
    m_input->setPlaceholderText("filename.fgd");
    QHBoxLayout *row = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Add");
    QPushButton *removeBtn = new QPushButton("Remove");
    row->addWidget(m_input);
    row->addWidget(addBtn);
    row->addWidget(removeBtn);
    layout->addLayout(row);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(addBtn, &QPushButton::clicked, this, &IncludeManager::addInclude);
    connect(removeBtn, &QPushButton::clicked, this, &IncludeManager::removeInclude);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void IncludeManager::addInclude() {
    QString s = m_input->text().trimmed();
    if (!s.isEmpty()) { m_list->addItem(s); m_input->clear(); }
}

void IncludeManager::removeInclude() {
    int row = m_list->currentRow();
    if (row >= 0) delete m_list->takeItem(row);
}

QStringList IncludeManager::getIncludes() const {
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i) result << m_list->item(i)->text();
    return result;
}
