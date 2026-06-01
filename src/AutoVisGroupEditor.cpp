#include "AutoVisGroupEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

enum ItemRole {
    TypeRole = Qt::UserRole
};

static const int TYPE_GROUP  = 0;
static const int TYPE_CHILD  = 1;
static const int TYPE_ENTITY = 2;

AutoVisGroupEditor::AutoVisGroupEditor(QWidget *parent, const QList<FGDAutoVisGroup> &groups)
    : QDialog(parent)
{
    setWindowTitle("@AutoVisGroup Editor");
    setMinimumSize(540, 480);
    setupUi();
    loadGroups(groups);
}

void AutoVisGroupEditor::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *hint = new QLabel(
        "Tree structure: Groups contain child groups, child groups contain entity classnames.\n"
        "Select a node to add items inside it or below it.\n"
        "Groups = top-level @AutoVisGroup parents. Children can be nested."
    );
    hint->setWordWrap(true);
    mainLayout->addWidget(hint);

    m_tree = new QTreeWidget;
    m_tree->setHeaderLabels({"Name", "Type"});
    m_tree->setColumnWidth(0, 280);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_tree);

    QHBoxLayout *inputRow = new QHBoxLayout;
    m_nameInput = new QLineEdit;
    m_nameInput->setPlaceholderText("Name");
    m_nameInput->setToolTip("Name for the new group, child group, or entity classname to add");
    inputRow->addWidget(m_nameInput);
    mainLayout->addLayout(inputRow);

    QHBoxLayout *btnRow = new QHBoxLayout;
    m_addGroupBtn  = new QPushButton("Add Group");
    m_addChildBtn  = new QPushButton("Add Child Group");
    m_addEntityBtn = new QPushButton("Add Entity");
    m_removeBtn    = new QPushButton("Remove");

    m_addGroupBtn->setToolTip(
        "Add a top-level @AutoVisGroup. These appear as parent categories in Hammer's filter toolbar."
    );
    m_addChildBtn->setToolTip(
        "Add a child group inside the selected group or child group. "
        "Children can be nested inside other children."
    );
    m_addEntityBtn->setToolTip(
        "Add an entity classname to the selected child group. "
        "Entities listed here will appear under that visgroup in Hammer."
    );
    m_removeBtn->setToolTip("Remove the selected item and all its children.");

    btnRow->addWidget(m_addGroupBtn);
    btnRow->addWidget(m_addChildBtn);
    btnRow->addWidget(m_addEntityBtn);
    btnRow->addWidget(m_removeBtn);
    btnRow->addStretch();
    mainLayout->addLayout(btnRow);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    connect(m_addGroupBtn,  &QPushButton::clicked, this, &AutoVisGroupEditor::addGroup);
    connect(m_addChildBtn,  &QPushButton::clicked, this, &AutoVisGroupEditor::addChild);
    connect(m_addEntityBtn, &QPushButton::clicked, this, &AutoVisGroupEditor::addEntity);
    connect(m_removeBtn,    &QPushButton::clicked, this, &AutoVisGroupEditor::removeSelected);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &AutoVisGroupEditor::onSelectionChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    onSelectionChanged();
}

static QTreeWidgetItem *makeItem(const QString &name, int type) {
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setData(0, TypeRole, type);
    switch (type) {
        case TYPE_GROUP:  item->setText(1, "Group");  break;
        case TYPE_CHILD:  item->setText(1, "Child");  break;
        case TYPE_ENTITY: item->setText(1, "Entity"); break;
    }
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    return item;
}

static void addChildToItem(QTreeWidgetItem *parentItem, const FGDAutoVisGroupChild &child);

static void addChildToItem(QTreeWidgetItem *parentItem, const FGDAutoVisGroupChild &child) {
    QTreeWidgetItem *childItem = makeItem(child.name, TYPE_CHILD);
    parentItem->addChild(childItem);
    for (auto &ent : child.entities) {
        childItem->addChild(makeItem(ent, TYPE_ENTITY));
    }
    for (auto &subchild : child.children) {
        addChildToItem(childItem, subchild);
    }
    childItem->setExpanded(true);
}

void AutoVisGroupEditor::loadGroups(const QList<FGDAutoVisGroup> &groups) {
    m_tree->clear();
    for (auto &g : groups) {
        QTreeWidgetItem *groupItem = makeItem(g.parent, TYPE_GROUP);
        m_tree->addTopLevelItem(groupItem);
        for (auto &child : g.children) {
            addChildToItem(groupItem, child);
        }
        groupItem->setExpanded(true);
    }
}

void AutoVisGroupEditor::addGroup() {
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) return;
    m_tree->addTopLevelItem(makeItem(name, TYPE_GROUP));
    m_nameInput->clear();
}

void AutoVisGroupEditor::addChild() {
    QTreeWidgetItem *sel = m_tree->currentItem();
    if (!sel) {
        QMessageBox::information(this, "Add Child Group", "Select a group or child group first.");
        return;
    }
    int selType = sel->data(0, TypeRole).toInt();
    if (selType == TYPE_ENTITY) {
        QMessageBox::information(this, "Add Child Group", "Cannot add a child group inside an entity. Select a group or child group.");
        return;
    }
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) return;
    sel->addChild(makeItem(name, TYPE_CHILD));
    sel->setExpanded(true);
    m_nameInput->clear();
}

void AutoVisGroupEditor::addEntity() {
    QTreeWidgetItem *sel = m_tree->currentItem();
    if (!sel) {
        QMessageBox::information(this, "Add Entity", "Select a child group first.");
        return;
    }
    int selType = sel->data(0, TypeRole).toInt();
    if (selType == TYPE_GROUP) {
        QMessageBox::information(this, "Add Entity", "Entities must be inside a child group, not directly in a group. Add a child group first.");
        return;
    }
    if (selType == TYPE_ENTITY) {
        QMessageBox::information(this, "Add Entity", "Select a child group to add entities to it.");
        return;
    }
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) return;
    sel->addChild(makeItem(name, TYPE_ENTITY));
    sel->setExpanded(true);
    m_nameInput->clear();
}

void AutoVisGroupEditor::removeSelected() {
    QTreeWidgetItem *sel = m_tree->currentItem();
    if (!sel) return;
    if (!sel->parent()) {
        int idx = m_tree->indexOfTopLevelItem(sel);
        delete m_tree->takeTopLevelItem(idx);
    } else {
        sel->parent()->removeChild(sel);
        delete sel;
    }
}

void AutoVisGroupEditor::onSelectionChanged() {
    QTreeWidgetItem *sel = m_tree->currentItem();
    bool hasGroup = sel && sel->data(0, TypeRole).toInt() == TYPE_GROUP;
    bool hasChild = sel && sel->data(0, TypeRole).toInt() == TYPE_CHILD;
    m_addChildBtn->setEnabled(hasGroup || hasChild);
    m_addEntityBtn->setEnabled(hasChild);
    m_removeBtn->setEnabled(sel != nullptr);
}

FGDAutoVisGroupChild AutoVisGroupEditor::extractChild(QTreeWidgetItem *item) const {
    FGDAutoVisGroupChild child;
    child.name = item->text(0);
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *ci = item->child(i);
        int t = ci->data(0, TypeRole).toInt();
        if (t == TYPE_ENTITY) {
            child.entities << ci->text(0);
        } else if (t == TYPE_CHILD) {
            child.children << extractChild(ci);
        }
    }
    return child;
}

QList<FGDAutoVisGroup> AutoVisGroupEditor::extractGroups() const {
    QList<FGDAutoVisGroup> result;
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *groupItem = m_tree->topLevelItem(i);
        FGDAutoVisGroup g;
        g.parent = groupItem->text(0);
        for (int j = 0; j < groupItem->childCount(); ++j) {
            QTreeWidgetItem *ci = groupItem->child(j);
            int t = ci->data(0, TypeRole).toInt();
            if (t == TYPE_CHILD) {
                g.children << extractChild(ci);
            }
        }
        result << g;
    }
    return result;
}

QList<FGDAutoVisGroup> AutoVisGroupEditor::getGroups() const {
    return extractGroups();
}
