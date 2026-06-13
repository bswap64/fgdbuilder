#include "EntityEditor.h"
#include "KeyvalueEditor.h"
#include "IOEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

EntityEditor::EntityEditor(QWidget *parent, const FGDEntity &entity, const QStringList &knownBases)
    : QDialog(parent), m_knownBases(knownBases)
{
    setWindowTitle("Entity Editor");
    setMinimumSize(720, 820);
    setupUi();
    populateFrom(entity);
}

void EntityEditor::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget;

    QWidget *generalTab = new QWidget;
    QVBoxLayout *genLayout = new QVBoxLayout(generalTab);
    QFormLayout *form = new QFormLayout;

    m_classType = new QComboBox;
    m_classType->addItems({"@PointClass","@SolidClass","@NPCClass","@KeyFrameClass","@MoveClass","@FilterClass","@BaseClass"});
    m_classType->setToolTip(
        "@PointClass - a point entity placed at a specific location\n"
        "@SolidClass - a brush entity (tied to a brush volume)\n"
        "@NPCClass - a point entity for NPCs; enables npcclass KV type\n"
        "@KeyFrameClass - for move_rope/keyframe_rope; links NextKey on copy\n"
        "@MoveClass - for path_track and similar; links target on copy\n"
        "@FilterClass - for filter entities; enables filterclass KV type\n"
        "@BaseClass - not shown in entity list; used only as a base for inheritance"
    );

    m_className = new QLineEdit;
    m_className->setToolTip(
        "The classname of the entity as used in game code and the VMF file.\n"
        "Must match the C++ class name in the game.\n"
        "Maximum 63 characters in Source (>80 will crash Hammer on map load)."
    );

    m_description = new QTextEdit;
    m_description->setMaximumHeight(80);
    m_description->setToolTip(
        "Description shown in Hammer's Help box when the mapper clicks 'Help'.\n"
        "Use \\n for line breaks. Do not use quotation marks (use apostrophes instead).\n"
        "Lines longer than 120 characters will be split automatically in the FGD output."
    );

    form->addRow("Class type:", m_classType);
    form->addRow("Class name:", m_className);
    form->addRow("Description:", m_description);

    m_color = new QLineEdit;
    m_color->setPlaceholderText("R G B  e.g. 255 128 0");
    m_color->setToolTip(
        "Color of the entity wireframe box in Hammer's 2D views (R G B, each 0-255).\n"
        "If omitted, defaults to magenta (220 30 220).\n"
        "Note: does not affect color in Hammer 4.x (only text color); fixed in Hammer++."
    );
    m_size = new QLineEdit;
    m_size->setPlaceholderText("e.g. -8 -8 -8, 8 8 8");
    m_size->setToolTip(
        "Defines the bounding box of the entity when no model or sprite is shown.\n"
        "Format: -x -y -z, +x +y +z  (e.g. -8 -8 -8, 8 8 8 for a 16x16x16 cube).\n"
        "Ignored by studioprop() which uses the model's own bounding box."
    );
    m_halfGridSnap = new QCheckBox("halfgridsnap");
    m_halfGridSnap->setToolTip(
        "When moving this entity, it snaps to half the current grid size.\n"
        "Takes no arguments. Supported in Hammer 4.x."
    );

    form->addRow("Color:", m_color);
    form->addRow("Size:", m_size);
    form->addRow("", m_halfGridSnap);

    QGroupBox *modelGroup = new QGroupBox("Model / Sprite helpers");
    modelGroup->setToolTip(
        "Controls how this entity is displayed in the map editor's 3D view.\n"
        "studio(), studioprop(), and iconsprite() can be combined independently.\n\n"
        "studio() - display a model; uses entity's 'model' KV if path is empty\n"
        "studioprop() - like studio(), but uses the model's own bounding box\n"
        "iconsprite() - display a sprite icon; can be used alongside studio()"
    );
    QFormLayout *modelForm = new QFormLayout(modelGroup);

    m_useStudio = new QCheckBox("studio()");
    m_useStudio->setToolTip("Display a model in the 3D view. If path is empty, uses the entity's 'model' keyvalue.");
    m_studioModel = new QLineEdit;
    m_studioModel->setPlaceholderText("e.g. models/combine_soldier.mdl  (empty = use 'model' KV)");
    m_studioModel->setEnabled(false);
    connect(m_useStudio, &QCheckBox::toggled, m_studioModel, &QLineEdit::setEnabled);
    QHBoxLayout *studioRow = new QHBoxLayout;
    studioRow->addWidget(m_useStudio);
    studioRow->addWidget(m_studioModel, 1);
    modelForm->addRow(studioRow);

    m_useStudioprop = new QCheckBox("studioprop()");
    m_useStudioprop->setToolTip("Like studio(), but uses the model's bounding box instead of size().\nRequired for entities with 'angles' that should be rotatable with the mouse in Hammer.");
    m_studiopropModel = new QLineEdit;
    m_studiopropModel->setPlaceholderText("e.g. models/props/barrel.mdl  (empty = use 'model' KV)");
    m_studiopropModel->setEnabled(false);
    connect(m_useStudioprop, &QCheckBox::toggled, m_studiopropModel, &QLineEdit::setEnabled);
    QHBoxLayout *studiopropRow = new QHBoxLayout;
    studiopropRow->addWidget(m_useStudioprop);
    studiopropRow->addWidget(m_studiopropModel, 1);
    modelForm->addRow(studiopropRow);

    m_useIconSprite = new QCheckBox("iconsprite()");
    m_useIconSprite->setToolTip("Display a sprite icon in the 3D view. Can be used alongside studio()/studioprop().\nUses VMT in Hammer 4.x, VMAT in Hammer 5.x, SPR in other editors.");
    m_iconSprite = new QLineEdit;
    m_iconSprite->setPlaceholderText("e.g. editor/env_cubemap.vmt");
    m_iconSprite->setEnabled(false);
    connect(m_useIconSprite, &QCheckBox::toggled, m_iconSprite, &QLineEdit::setEnabled);
    QHBoxLayout *iconSpriteRow = new QHBoxLayout;
    iconSpriteRow->addWidget(m_useIconSprite);
    iconSpriteRow->addWidget(m_iconSprite, 1);
    modelForm->addRow(iconSpriteRow);

    form->addRow("", modelGroup);

    QGroupBox *extraHelpers = new QGroupBox("Additional helpers");
    extraHelpers->setToolTip("Optional helpers that add extra visual aids in the editor viewport.");
    QFormLayout *extraForm = new QFormLayout(extraHelpers);

    m_sphere = new QLineEdit;
    m_sphere->setPlaceholderText("property name, e.g. radius");
    m_sphere->setToolTip("sphere(propertyname) - draws a sphere around the entity. Radius taken from the named KV.");

    m_line = new QLineEdit;
    m_line->setPlaceholderText("e.g. 255 255 255, targetname, target, target, entity");
    m_line->setToolTip("line(color, start_key, start_value, end_key, end_value) - draws a line between two entities.");

    m_vecline = new QLineEdit;
    m_vecline->setPlaceholderText("property name");
    m_vecline->setToolTip("vecline(property) - positions a vector property and draws a line from the entity to it.");

    m_axis = new QLineEdit;
    m_axis->setPlaceholderText("property name");
    m_axis->setToolTip("axis(property) - positions two points joined by a line in the map.");

    m_wirebox = new QLineEdit;
    m_wirebox->setPlaceholderText("e.g. mins, maxs");
    m_wirebox->setToolTip("wirebox(min, max) - draws a bounding box between two properties.");

    m_origin = new QLineEdit;
    m_origin->setPlaceholderText("property name, e.g. origin");
    m_origin->setToolTip("origin(property) - allows positioning a vector property in the map. Updates when entity is moved.");

    m_lightprop = new QLineEdit;
    m_lightprop->setPlaceholderText("e.g. models/props/light.mdl");
    m_lightprop->setToolTip("lightprop(\"path\") - like studioprop(), but pitch of the model is inverted. Hammer 4.x.");

    extraForm->addRow("sphere():", m_sphere);
    extraForm->addRow("line():", m_line);
    extraForm->addRow("vecline():", m_vecline);
    extraForm->addRow("axis():", m_axis);
    extraForm->addRow("wirebox():", m_wirebox);
    extraForm->addRow("origin():", m_origin);
    extraForm->addRow("lightprop():", m_lightprop);

    QHBoxLayout *flagHelpers = new QHBoxLayout;
    m_decal           = new QCheckBox("decal()");
    m_overlay         = new QCheckBox("overlay()");
    m_sprite          = new QCheckBox("sprite()");
    m_sweptplayerhull = new QCheckBox("sweptplayerhull()");
    m_instance_       = new QCheckBox("instance()");
    m_animator        = new QCheckBox("animator()");
    m_keyframe_       = new QCheckBox("keyframe()");
    m_worldtext       = new QCheckBox("worldtext()");

    m_decal->setToolTip("Renders decals on nearby surfaces. Uses the 'texture' KV for the material.");
    m_overlay->setToolTip("Renders overlays on a surface. Used for info_overlay. Auto-sets UV corner KVs.");
    m_sprite->setToolTip("Renders the sprite material from the 'model' KV (env_sprite and variants).");
    m_sweptplayerhull->setToolTip("Draws 32x32x72 rectangular prisms at point0 and point1 to show movement space.");
    m_instance_->setToolTip("Renders the instance VMF in the map. Generates additional KVs for instance parameters.");
    m_animator->setToolTip("Used with @MoveClass to mark this entity as the first in a chain.");
    m_keyframe_->setToolTip("Automatically renames this entity when cloned.");
    m_worldtext->setToolTip("Displays the 'message' KV contents in the 3D viewport. Hammer 4.x / Strata.");

    flagHelpers->addWidget(m_decal);
    flagHelpers->addWidget(m_overlay);
    flagHelpers->addWidget(m_sprite);
    flagHelpers->addWidget(m_sweptplayerhull);
    flagHelpers->addWidget(m_instance_);
    flagHelpers->addWidget(m_animator);
    flagHelpers->addWidget(m_keyframe_);
    flagHelpers->addWidget(m_worldtext);
    flagHelpers->addStretch();
    extraForm->addRow("Flag helpers:", flagHelpers);

    genLayout->addLayout(form);
    genLayout->addWidget(extraHelpers);

    QGroupBox *basesGroup = new QGroupBox("Base classes");
    basesGroup->setToolTip(
        "Attach previously defined @BaseClass entities to inherit their keyvalues and helpers.\n"
        "Multiple bases can be specified."
    );
    QVBoxLayout *basesLayout = new QVBoxLayout(basesGroup);
    m_baseList = new QListWidget;
    m_baseList->setMaximumHeight(90);
    QHBoxLayout *baseInputRow = new QHBoxLayout;
    m_baseCombo = new QComboBox;
    m_baseCombo->setEditable(true);
    m_baseCombo->setInsertPolicy(QComboBox::NoInsert);
    m_baseCombo->setPlaceholderText("Base class name");
    m_baseCombo->setToolTip("Select or type the name of a @BaseClass defined in this project or an included FGD.");
    for (auto &b : m_knownBases) m_baseCombo->addItem(b);
    QPushButton *addBaseBtn = new QPushButton("Add");
    QPushButton *removeBaseBtn = new QPushButton("Remove");
    baseInputRow->addWidget(m_baseCombo, 1);
    baseInputRow->addWidget(addBaseBtn);
    baseInputRow->addWidget(removeBaseBtn);
    basesLayout->addWidget(m_baseList);
    basesLayout->addLayout(baseInputRow);
    genLayout->addWidget(basesGroup);
    genLayout->addStretch();
    tabs->addTab(generalTab, "General");

    QWidget *kvTab = new QWidget;
    QVBoxLayout *kvLayout = new QVBoxLayout(kvTab);
    QLabel *kvHint = new QLabel(
        "Keyvalues (KVs) define the entity's configurable properties. "
        "Each KV has an internal name, a type, an optional display name, default value, and description."
    );
    kvHint->setWordWrap(true);
    kvLayout->addWidget(kvHint);
    m_kvTable = new QTableWidget(0, 4);
    m_kvTable->setHorizontalHeaderLabels({"Name","Type","Default","Description"});
    m_kvTable->horizontalHeader()->setStretchLastSection(true);
    m_kvTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_kvTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHBoxLayout *kvBtns = new QHBoxLayout;
    QPushButton *addKV = new QPushButton("Add Keyvalue");
    QPushButton *editKV = new QPushButton("Edit");
    QPushButton *removeKV = new QPushButton("Remove");
    kvBtns->addWidget(addKV);
    kvBtns->addWidget(editKV);
    kvBtns->addWidget(removeKV);
    kvBtns->addStretch();
    kvLayout->addWidget(m_kvTable);
    kvLayout->addLayout(kvBtns);
    tabs->addTab(kvTab, "Keyvalues");

    QWidget *ioTab = new QWidget;
    QVBoxLayout *ioLayout = new QVBoxLayout(ioTab);
    QLabel *ioHint = new QLabel(
        "Inputs are actions that can be triggered on this entity via I/O connections. "
        "Outputs are events this entity fires that can trigger other entities."
    );
    ioHint->setWordWrap(true);
    ioLayout->addWidget(ioHint);
    m_ioTable = new QTableWidget(0, 3);
    m_ioTable->setHorizontalHeaderLabels({"Direction","Name","Type"});
    m_ioTable->horizontalHeader()->setStretchLastSection(true);
    m_ioTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ioTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHBoxLayout *ioBtns = new QHBoxLayout;
    QPushButton *addIO = new QPushButton("Add Input/Output");
    QPushButton *editIO = new QPushButton("Edit");
    QPushButton *removeIO = new QPushButton("Remove");
    ioBtns->addWidget(addIO);
    ioBtns->addWidget(editIO);
    ioBtns->addWidget(removeIO);
    ioBtns->addStretch();
    ioLayout->addWidget(m_ioTable);
    ioLayout->addLayout(ioBtns);
    tabs->addTab(ioTab, "Inputs / Outputs");

    mainLayout->addWidget(tabs);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    connect(addBaseBtn, &QPushButton::clicked, this, &EntityEditor::addBase);
    connect(removeBaseBtn, &QPushButton::clicked, this, &EntityEditor::removeBase);
    connect(addKV, &QPushButton::clicked, this, &EntityEditor::addKeyvalue);
    connect(editKV, &QPushButton::clicked, this, &EntityEditor::editKeyvalue);
    connect(removeKV, &QPushButton::clicked, this, &EntityEditor::removeKeyvalue);
    connect(addIO, &QPushButton::clicked, this, &EntityEditor::addIO);
    connect(editIO, &QPushButton::clicked, this, &EntityEditor::editIO);
    connect(removeIO, &QPushButton::clicked, this, &EntityEditor::removeIO);
    connect(m_kvTable, &QTableWidget::cellDoubleClicked, this, &EntityEditor::editKeyvalue);
    connect(m_ioTable, &QTableWidget::cellDoubleClicked, this, &EntityEditor::editIO);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void EntityEditor::populateFrom(const FGDEntity &entity) {
    int idx = m_classType->findText(classTypeToString(entity.classType));
    if (idx >= 0) m_classType->setCurrentIndex(idx);
    m_className->setText(entity.className);
    m_description->setPlainText(entity.description);
    m_color->setText(entity.color);
    m_size->setText(entity.size);
    m_halfGridSnap->setChecked(entity.halfGridSnap);
    m_sphere->setText(entity.sphere);
    m_line->setText(entity.line);
    m_vecline->setText(entity.vecline);
    m_axis->setText(entity.axis);
    m_wirebox->setText(entity.wirebox);
    m_origin->setText(entity.origin);
    m_lightprop->setText(entity.lightprop);
    m_decal->setChecked(entity.decal);
    m_overlay->setChecked(entity.overlay);
    m_sprite->setChecked(entity.sprite);
    m_sweptplayerhull->setChecked(entity.sweptplayerhull);
    m_instance_->setChecked(entity.instance_);
    m_animator->setChecked(entity.animator);
    m_keyframe_->setChecked(entity.keyframe_);
    m_worldtext->setChecked(entity.worldtext);

    m_useStudio->setChecked(!entity.studioModel.isEmpty());
    m_studioModel->setText(entity.studioModel);
    m_studioModel->setEnabled(!entity.studioModel.isEmpty());

    m_useStudioprop->setChecked(!entity.studioprop.isEmpty());
    m_studiopropModel->setText(entity.studioprop);
    m_studiopropModel->setEnabled(!entity.studioprop.isEmpty());

    m_useIconSprite->setChecked(!entity.iconSprite.isEmpty());
    m_iconSprite->setText(entity.iconSprite);
    m_iconSprite->setEnabled(!entity.iconSprite.isEmpty());

    for (auto &b : entity.bases) m_baseList->addItem(b);
    m_keyvalues = entity.keyvalues;
    m_ios = entity.ios;
    for (auto &kv : m_keyvalues) {
        int row = m_kvTable->rowCount();
        m_kvTable->insertRow(row);
        m_kvTable->setItem(row, 0, new QTableWidgetItem(kv.name));
        m_kvTable->setItem(row, 1, new QTableWidgetItem(kv.type));
        m_kvTable->setItem(row, 2, new QTableWidgetItem(kv.defaultValue));
        m_kvTable->setItem(row, 3, new QTableWidgetItem(kv.description));
    }
    for (auto &io : m_ios) {
        int row = m_ioTable->rowCount();
        m_ioTable->insertRow(row);
        m_ioTable->setItem(row, 0, new QTableWidgetItem(io.isInput ? "Input" : "Output"));
        m_ioTable->setItem(row, 1, new QTableWidgetItem(io.name));
        m_ioTable->setItem(row, 2, new QTableWidgetItem(io.type));
    }
}

void EntityEditor::addBase() {
    QString name = m_baseCombo->currentText().trimmed();
    if (name.isEmpty()) return;
    for (int i = 0; i < m_baseList->count(); ++i)
        if (m_baseList->item(i)->text() == name) return;
    m_baseList->addItem(name);
    m_baseCombo->setCurrentIndex(-1);
    m_baseCombo->clearEditText();
}

void EntityEditor::removeBase() {
    int row = m_baseList->currentRow();
    if (row >= 0) delete m_baseList->takeItem(row);
}

void EntityEditor::addKeyvalue() {
    KeyvalueEditor dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        FGDKeyvalue kv = dlg.getKeyvalue();
        if (kv.name.isEmpty()) return;
        m_keyvalues.append(kv);
        int row = m_kvTable->rowCount();
        m_kvTable->insertRow(row);
        m_kvTable->setItem(row, 0, new QTableWidgetItem(kv.name));
        m_kvTable->setItem(row, 1, new QTableWidgetItem(kv.type));
        m_kvTable->setItem(row, 2, new QTableWidgetItem(kv.defaultValue));
        m_kvTable->setItem(row, 3, new QTableWidgetItem(kv.description));
    }
}

void EntityEditor::editKeyvalue() {
    int row = m_kvTable->currentRow();
    if (row < 0 || row >= m_keyvalues.size()) return;
    KeyvalueEditor dlg(this, m_keyvalues[row]);
    if (dlg.exec() == QDialog::Accepted) {
        FGDKeyvalue kv = dlg.getKeyvalue();
        m_keyvalues[row] = kv;
        m_kvTable->setItem(row, 0, new QTableWidgetItem(kv.name));
        m_kvTable->setItem(row, 1, new QTableWidgetItem(kv.type));
        m_kvTable->setItem(row, 2, new QTableWidgetItem(kv.defaultValue));
        m_kvTable->setItem(row, 3, new QTableWidgetItem(kv.description));
    }
}

void EntityEditor::removeKeyvalue() {
    int row = m_kvTable->currentRow();
    if (row < 0 || row >= m_keyvalues.size()) return;
    m_kvTable->removeRow(row);
    m_keyvalues.removeAt(row);
}

void EntityEditor::addIO() {
    IOEditor dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        FGDIO io = dlg.getIO();
        if (io.name.isEmpty()) return;
        m_ios.append(io);
        int row = m_ioTable->rowCount();
        m_ioTable->insertRow(row);
        m_ioTable->setItem(row, 0, new QTableWidgetItem(io.isInput ? "Input" : "Output"));
        m_ioTable->setItem(row, 1, new QTableWidgetItem(io.name));
        m_ioTable->setItem(row, 2, new QTableWidgetItem(io.type));
    }
}

void EntityEditor::editIO() {
    int row = m_ioTable->currentRow();
    if (row < 0 || row >= m_ios.size()) return;
    IOEditor dlg(this, m_ios[row]);
    if (dlg.exec() == QDialog::Accepted) {
        FGDIO io = dlg.getIO();
        m_ios[row] = io;
        m_ioTable->setItem(row, 0, new QTableWidgetItem(io.isInput ? "Input" : "Output"));
        m_ioTable->setItem(row, 1, new QTableWidgetItem(io.name));
        m_ioTable->setItem(row, 2, new QTableWidgetItem(io.type));
    }
}

void EntityEditor::removeIO() {
    int row = m_ioTable->currentRow();
    if (row < 0 || row >= m_ios.size()) return;
    m_ioTable->removeRow(row);
    m_ios.removeAt(row);
}

FGDEntity EntityEditor::getEntity() const {
    FGDEntity e;
    QString ct = m_classType->currentText();
    if      (ct == "@PointClass")    e.classType = EntityClassType::PointClass;
    else if (ct == "@SolidClass")    e.classType = EntityClassType::SolidClass;
    else if (ct == "@NPCClass")      e.classType = EntityClassType::NPCClass;
    else if (ct == "@KeyFrameClass") e.classType = EntityClassType::KeyFrameClass;
    else if (ct == "@MoveClass")     e.classType = EntityClassType::MoveClass;
    else if (ct == "@FilterClass")   e.classType = EntityClassType::FilterClass;
    else if (ct == "@BaseClass")     e.classType = EntityClassType::BaseClass;
    e.className   = m_className->text().trimmed();
    e.description = m_description->toPlainText().trimmed();
    e.color       = m_color->text().trimmed();
    e.size        = m_size->text().trimmed();
    e.halfGridSnap = m_halfGridSnap->isChecked();
    e.sphere      = m_sphere->text().trimmed();
    e.line        = m_line->text().trimmed();
    e.vecline     = m_vecline->text().trimmed();
    e.axis        = m_axis->text().trimmed();
    e.wirebox     = m_wirebox->text().trimmed();
    e.origin      = m_origin->text().trimmed();
    e.lightprop   = m_lightprop->text().trimmed();
    e.decal       = m_decal->isChecked();
    e.overlay     = m_overlay->isChecked();
    e.sprite      = m_sprite->isChecked();
    e.sweptplayerhull = m_sweptplayerhull->isChecked();
    e.instance_   = m_instance_->isChecked();
    e.animator    = m_animator->isChecked();
    e.keyframe_   = m_keyframe_->isChecked();
    e.worldtext   = m_worldtext->isChecked();
    if (m_useStudio->isChecked())
        e.studioModel = m_studioModel->text().trimmed();
    if (m_useStudioprop->isChecked())
        e.studioprop  = m_studiopropModel->text().trimmed();
    if (m_useIconSprite->isChecked())
        e.iconSprite  = m_iconSprite->text().trimmed();
    for (int i = 0; i < m_baseList->count(); ++i)
        e.bases << m_baseList->item(i)->text();
    e.keyvalues = m_keyvalues;
    e.ios       = m_ios;
    return e;
}
