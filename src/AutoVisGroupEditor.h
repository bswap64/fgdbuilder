#pragma once
#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include "FGDData.h"

class AutoVisGroupEditor : public QDialog {
    Q_OBJECT
public:
    explicit AutoVisGroupEditor(QWidget *parent = nullptr, const QList<FGDAutoVisGroup> &groups = {});
    QList<FGDAutoVisGroup> getGroups() const;
private slots:
    void addGroup();
    void addChild();
    void addEntity();
    void removeSelected();
    void onSelectionChanged();
private:
    void setupUi();
    void loadGroups(const QList<FGDAutoVisGroup> &groups);
    QList<FGDAutoVisGroup> extractGroups() const;
    FGDAutoVisGroupChild extractChild(QTreeWidgetItem *item) const;

    QTreeWidget *m_tree;
    QLineEdit *m_nameInput;
    QPushButton *m_addGroupBtn;
    QPushButton *m_addChildBtn;
    QPushButton *m_addEntityBtn;
    QPushButton *m_removeBtn;
};
