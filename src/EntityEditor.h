#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QRadioButton>
#include <QStackedWidget>
#include "FGDData.h"

class EntityEditor : public QDialog {
    Q_OBJECT
public:
    explicit EntityEditor(QWidget *parent = nullptr, const FGDEntity &entity = FGDEntity(), const QStringList &knownBases = {});
    FGDEntity getEntity() const;
private slots:
    void addKeyvalue();
    void editKeyvalue();
    void removeKeyvalue();
    void addIO();
    void editIO();
    void removeIO();
    void addBase();
    void removeBase();
    void onModelModeChanged();
private:
    void setupUi();
    void populateFrom(const FGDEntity &entity);
    QComboBox *m_classType;
    QLineEdit *m_className;
    QTextEdit *m_description;
    QRadioButton *m_useStudio;
    QRadioButton *m_useIconSprite;
    QRadioButton *m_useStudioprop;
    QRadioButton *m_noModel;
    QStackedWidget *m_modelStack;
    QLineEdit *m_studioModel;
    QLineEdit *m_iconSprite;
    QLineEdit *m_studiopropModel;
    QLineEdit *m_color;
    QLineEdit *m_size;
    QCheckBox *m_halfGridSnap;
    QLineEdit *m_sphere;
    QLineEdit *m_line;
    QLineEdit *m_vecline;
    QLineEdit *m_axis;
    QLineEdit *m_wirebox;
    QCheckBox *m_decal;
    QCheckBox *m_overlay;
    QCheckBox *m_sprite;
    QCheckBox *m_sweptplayerhull;
    QCheckBox *m_instance_;
    QCheckBox *m_animator;
    QCheckBox *m_keyframe_;
    QCheckBox *m_worldtext;
    QComboBox *m_baseCombo;
    QListWidget *m_baseList;
    QTableWidget *m_kvTable;
    QTableWidget *m_ioTable;
    QList<FGDKeyvalue> m_keyvalues;
    QList<FGDIO> m_ios;
    QStringList m_knownBases;
};
