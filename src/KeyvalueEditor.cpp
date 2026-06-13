#include "KeyvalueEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

static const QStringList KV_TYPES = {
    "string", "integer", "float", "boolean", "choices", "flags",
    "angle", "angle_negative_pitch", "color255", "color1",
    "target_destination", "target_source", "target_name_or_class",
    "studio", "sound", "sprite", "material", "decal",
    "node_id", "particlesystem", "sidelist", "origin", "vector",
    "script", "scriptlist", "scene", "npcclass", "pointentityclass",
    "filterclass", "instance_file", "instance_parm", "instance_variable",
    "sky", "soundscape", "axis", "vecline"
};

static const QMap<QString,QString> KV_TYPE_TOOLTIPS = {
    {"string",              "A text string value."},
    {"integer",             "A whole number."},
    {"float",               "A decimal number."},
    {"boolean",             "True (1) or false (0). Shown as Yes/No dropdown. Default must be 0 or 1."},
    {"choices",             "A dropdown with a predefined set of options."},
    {"flags",               "Bitfield flags shown as checkboxes. Use spawnflags as the key name for Hammer/JACK compatibility."},
    {"angle",               "An angle widget for setting pitch/yaw/roll."},
    {"angle_negative_pitch","Like angle, but pitch is inverted."},
    {"color255",            "RGB color picker (0–255 per channel)."},
    {"color1",              "RGB color picker (0.0–1.0 per channel)."},
    {"target_destination",  "Dropdown of other entities' targetnames (what this entity points to)."},
    {"target_source",       "Marks this property as the entity's own targetname."},
    {"target_name_or_class","Another entity's targetname or classname."},
    {"studio",              "Opens the model browser (MDL files)."},
    {"sound",               "Opens the sound browser."},
    {"sprite",              "Opens the sprite browser."},
    {"material",            "Opens the material/texture browser."},
    {"decal",               "Opens the material browser filtered to decals."},
    {"node_id",             "Auto-incrementing node ID for AI nodes."},
    {"particlesystem",      "Opens the particle system browser."},
    {"sidelist",            "Side selection eyedropper for brush faces."},
    {"origin",              "The entity's origin; updates when moved in the viewport."},
    {"vector",              "A 3D vector (three space-delimited numbers)."},
    {"script",              "Opens the file browser for VScript files."},
    {"scriptlist",          "A list of VScript files."},
    {"scene",               "Opens the browser for scene/choreography files."},
    {"npcclass",            "Dropdown of all NPCClass entities."},
    {"pointentityclass",    "Dropdown of all PointClass entities."},
    {"filterclass",         "Marks this as the filter entity to use."},
    {"instance_file",       "File browser for func_instance VMF files."},
    {"instance_parm",       "Used in func_instance_parms to define fixup variables."},
    {"instance_variable",   "Used in func_instance to set fixup variables."},
    {"sky",                 "Dropdown of available skyboxes (JACK)."},
    {"soundscape",          "Dropdown of soundscapes from the manifest (Strata/JBE3)."},
    {"axis",                "Adds a 2-point axis helper in the viewport."},
    {"vecline",             "Adds a 1-point axis helper and draws a line from the entity."},
};

KeyvalueEditor::KeyvalueEditor(QWidget *parent, const FGDKeyvalue &kv)
    : QDialog(parent)
{
    setWindowTitle("Keyvalue Editor");
    setMinimumWidth(560);
    setupUi();
    m_name->setText(kv.name);
    int idx = m_type->findText(kv.type);
    if (idx >= 0) m_type->setCurrentIndex(idx);
    m_displayName->setText(kv.displayName);
    m_defaultValue->setText(kv.defaultValue);
    m_description->setText(kv.description);
    m_readOnly->setChecked(kv.readOnly);
    m_report->setChecked(kv.report);

    for (int i = 0; i < kv.choiceItems.size(); ++i) {
        auto &c = kv.choiceItems[i];
        QString desc = (i < kv.choiceDescriptions.size()) ? kv.choiceDescriptions[i].second : QString();
        int row = m_choicesTable->rowCount();
        m_choicesTable->insertRow(row);
        m_choicesTable->setItem(row, 0, new QTableWidgetItem(c.first));
        m_choicesTable->setItem(row, 1, new QTableWidgetItem(c.second));
        m_choicesTable->setItem(row, 2, new QTableWidgetItem(desc));
    }

    for (int i = 0; i < kv.flagItems.size(); ++i) {
        auto &f = kv.flagItems[i];
        QString desc = (i < kv.flagDescriptions.size()) ? kv.flagDescriptions[i] : QString();
        int row = m_flagsTable->rowCount();
        m_flagsTable->insertRow(row);
        m_flagsTable->setItem(row, 0, new QTableWidgetItem(f.first));
        m_flagsTable->setItem(row, 1, new QTableWidgetItem(f.second.first));
        m_flagsTable->setItem(row, 2, new QTableWidgetItem(QString::number(f.second.second)));
        m_flagsTable->setItem(row, 3, new QTableWidgetItem(desc));
    }

    onTypeChanged(m_type->currentText());
}

void KeyvalueEditor::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout;
    m_name = new QLineEdit;
    m_name->setToolTip("Internal keyvalue name used in the game code and VMF file.\nMax 30 characters in Source. Avoid periods and hashes.");
    m_type = new QComboBox;
    m_type->addItems(KV_TYPES);
    m_type->setToolTip("Value type of this keyvalue. Determines the widget shown in Hammer's Object Properties.");

    m_displayName = new QLineEdit;
    m_displayName->setToolTip("Human-readable name shown in Hammer's SmartEdit mode.\nIf blank, the internal name is shown.");
    m_defaultValue = new QLineEdit;
    m_defaultValue->setToolTip("Default value auto-filled when the entity is placed.\nLeave blank if the entity should not have this key by default.\nStrings and floats must be quoted in FGD output; this tool handles that automatically.");
    m_description = new QLineEdit;
    m_description->setToolTip("Description shown in Hammer's Help box for this keyvalue.\nUse \\n for line breaks. Quotation marks will be replaced by apostrophes.");
    m_readOnly = new QCheckBox("Read only");
    m_readOnly->setToolTip("Prevents the user from editing this keyvalue without disabling SmartEdit.\nSupported in Hammer 4.x and Hammer 5.x.");
    m_report = new QCheckBox("Report");
    m_report->setToolTip("Causes this keyvalue's value to appear in Hammer's Entity Report.\ntargetname is always shown regardless of this modifier.");

    form->addRow("Internal name:", m_name);
    form->addRow("Type:", m_type);
    form->addRow("Display name:", m_displayName);
    form->addRow("Default value:", m_defaultValue);
    form->addRow("Description:", m_description);
    QHBoxLayout *modRow = new QHBoxLayout;
    modRow->addWidget(m_readOnly);
    modRow->addWidget(m_report);
    modRow->addStretch();
    form->addRow("Modifiers:", modRow);
    mainLayout->addLayout(form);

    m_stack = new QStackedWidget;

    QWidget *emptyPage = new QWidget;
    m_stack->addWidget(emptyPage);

    QWidget *choicesPage = new QWidget;
    QVBoxLayout *choicesLayout = new QVBoxLayout(choicesPage);
    QLabel *choicesLabel = new QLabel("Choice items - Value : Display Name : Description (optional, shown in JACK):");
    choicesLabel->setToolTip(
        "Each choice item has:\n"
        "  Value - the actual value written to the VMF (number or string)\n"
        "  Display Name - shown in Hammer's dropdown\n"
        "  Description - optional extra help text shown in J.A.C.K."
    );
    m_choicesTable = new QTableWidget(0, 3);
    m_choicesTable->setHorizontalHeaderLabels({"Value", "Display Name", "Description (JACK)"});
    m_choicesTable->horizontalHeader()->setStretchLastSection(true);
    m_choicesTable->setMinimumHeight(130);
    QHBoxLayout *choicesBtns = new QHBoxLayout;
    QPushButton *addChoice = new QPushButton("Add");
    QPushButton *removeChoice = new QPushButton("Remove");
    choicesBtns->addWidget(addChoice);
    choicesBtns->addWidget(removeChoice);
    choicesBtns->addStretch();
    choicesLayout->addWidget(choicesLabel);
    choicesLayout->addWidget(m_choicesTable);
    choicesLayout->addLayout(choicesBtns);
    m_stack->addWidget(choicesPage);

    QWidget *flagsPage = new QWidget;
    QVBoxLayout *flagsLayout = new QVBoxLayout(flagsPage);
    QLabel *flagsLabel = new QLabel("Flag items - Bit Value : Name : Default (0/1) : Description (Strata/JACK/TrenchBroom):");
    flagsLabel->setToolTip(
        "Each flag item has:\n"
        "  Bit Value - power of 2 (1, 2, 4, 8, 16, ...)\n"
        "  Display Name - checkbox label shown in Hammer\n"
        "  Default - 0 = unchecked by default, 1 = checked by default\n"
        "  Description - optional, supported in Strata Hammer, J.A.C.K., and TrenchBroom"
    );
    m_flagsTable = new QTableWidget(0, 4);
    m_flagsTable->setHorizontalHeaderLabels({"Bit Value", "Display Name", "Default (0/1)", "Description"});
    m_flagsTable->horizontalHeader()->setStretchLastSection(true);
    m_flagsTable->setMinimumHeight(130);
    QHBoxLayout *flagsBtns = new QHBoxLayout;
    QPushButton *addFlag = new QPushButton("Add");
    QPushButton *removeFlag = new QPushButton("Remove");
    flagsBtns->addWidget(addFlag);
    flagsBtns->addWidget(removeFlag);
    flagsBtns->addStretch();
    flagsLayout->addWidget(flagsLabel);
    flagsLayout->addWidget(m_flagsTable);
    flagsLayout->addLayout(flagsBtns);
    m_stack->addWidget(flagsPage);

    mainLayout->addWidget(m_stack);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);

    connect(m_type, &QComboBox::currentTextChanged, this, &KeyvalueEditor::onTypeChanged);
    connect(m_type, &QComboBox::currentTextChanged, this, [this](const QString &t){
        m_type->setToolTip(KV_TYPE_TOOLTIPS.value(t, "Value type for this keyvalue."));
    });
    connect(addChoice, &QPushButton::clicked, this, &KeyvalueEditor::addChoiceItem);
    connect(removeChoice, &QPushButton::clicked, this, &KeyvalueEditor::removeChoiceItem);
    connect(addFlag, &QPushButton::clicked, this, &KeyvalueEditor::addFlagItem);
    connect(removeFlag, &QPushButton::clicked, this, &KeyvalueEditor::removeFlagItem);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]{
        QString name = m_name->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Validation", "Keyvalue name cannot be empty.");
            return;
        }
        if (name.length() > 30) {
            QMessageBox::warning(this, "Validation",
                QString("Keyvalue name '%1' is %2 characters long. Source engine supports max 30 characters.").arg(name).arg(name.length()));
            return;
        }
        if (name.contains('.') || name.contains('#')) {
            QMessageBox::warning(this, "Validation",
                QString("Keyvalue name '%1' contains '.' or '#'. These characters do not work correctly in Source FGD files.").arg(name));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void KeyvalueEditor::onTypeChanged(const QString &type) {
    if (type == "choices") {
        m_stack->setCurrentIndex(1);
    } else if (type == "flags") {
        m_stack->setCurrentIndex(2);
    } else {
        m_stack->setCurrentIndex(0);
    }
}

void KeyvalueEditor::addChoiceItem() {
    int row = m_choicesTable->rowCount();
    m_choicesTable->insertRow(row);
    m_choicesTable->setItem(row, 0, new QTableWidgetItem("0"));
    m_choicesTable->setItem(row, 1, new QTableWidgetItem("Option"));
    m_choicesTable->setItem(row, 2, new QTableWidgetItem(""));
}

void KeyvalueEditor::removeChoiceItem() {
    int row = m_choicesTable->currentRow();
    if (row >= 0) m_choicesTable->removeRow(row);
}

void KeyvalueEditor::addFlagItem() {
    int row = m_flagsTable->rowCount();
    quint64 bitVal = (row == 0) ? 1ULL : (1ULL << row);
    m_flagsTable->insertRow(row);
    m_flagsTable->setItem(row, 0, new QTableWidgetItem(QString::number(bitVal)));
    m_flagsTable->setItem(row, 1, new QTableWidgetItem("Flag " + QString::number(row + 1)));
    m_flagsTable->setItem(row, 2, new QTableWidgetItem("0"));
    m_flagsTable->setItem(row, 3, new QTableWidgetItem(""));
}

void KeyvalueEditor::removeFlagItem() {
    int row = m_flagsTable->currentRow();
    if (row >= 0) m_flagsTable->removeRow(row);
}

FGDKeyvalue KeyvalueEditor::getKeyvalue() const {
    FGDKeyvalue kv;
    kv.name = m_name->text().trimmed();
    kv.type = m_type->currentText();
    kv.displayName = m_displayName->text();
    kv.defaultValue = m_defaultValue->text();
    kv.description = m_description->text();
    kv.readOnly = m_readOnly->isChecked();
    kv.report = m_report->isChecked();
    for (int i = 0; i < m_choicesTable->rowCount(); ++i) {
        QString val  = m_choicesTable->item(i, 0) ? m_choicesTable->item(i, 0)->text() : "";
        QString name = m_choicesTable->item(i, 1) ? m_choicesTable->item(i, 1)->text() : "";
        QString desc = m_choicesTable->item(i, 2) ? m_choicesTable->item(i, 2)->text() : "";
        kv.choiceItems.append({val, name});
        kv.choiceDescriptions.append({val, desc});
    }
    for (int i = 0; i < m_flagsTable->rowCount(); ++i) {
        QString bv   = m_flagsTable->item(i, 0) ? m_flagsTable->item(i, 0)->text() : "1";
        QString fn   = m_flagsTable->item(i, 1) ? m_flagsTable->item(i, 1)->text() : "";
        int def      = m_flagsTable->item(i, 2) ? m_flagsTable->item(i, 2)->text().toInt() : 0;
        QString desc = m_flagsTable->item(i, 3) ? m_flagsTable->item(i, 3)->text() : "";
        kv.flagItems.append({bv, {fn, def}});
        kv.flagDescriptions.append(desc);
    }
    return kv;
}
