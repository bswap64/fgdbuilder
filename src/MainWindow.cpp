#include "DiscordRPC.h"
#include "MainWindow.h"
#include "EntityEditor.h"
#include "IncludeManager.h"
#include "BaseClassManager.h"
#include "AutoVisGroupEditor.h"
#include "MapSizeDialog.h"
#include "FGDGenerator.h"
#include "FGDImporter.h"
#include "FGDHighlighter.h"
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QFont>
#include <QIcon>
#include <QTextEdit>
#include <QLabel>
#include <QFileInfo>
#include <QSettings>
#include <QCloseEvent>
#include <QUndoStack>
#include <QUndoCommand>
#include <functional>

static QJsonObject kvToJson(const FGDKeyvalue &kv) {
    QJsonObject o;
    o["name"] = kv.name;
    o["type"] = kv.type;
    o["displayName"] = kv.displayName;
    o["defaultValue"] = kv.defaultValue;
    o["description"] = kv.description;
    o["readOnly"] = kv.readOnly;
    o["report"] = kv.report;
    QJsonArray choices;
    for (int i = 0; i < kv.choiceItems.size(); ++i) {
        auto &c = kv.choiceItems[i];
        QString desc = (i < kv.choiceDescriptions.size()) ? kv.choiceDescriptions[i].second : QString();
        QJsonObject ci; ci["value"] = c.first; ci["label"] = c.second; ci["desc"] = desc;
        choices.append(ci);
    }
    o["choices"] = choices;
    QJsonArray flags;
    for (int i = 0; i < kv.flagItems.size(); ++i) {
        auto &f = kv.flagItems[i];
        QString desc = (i < kv.flagDescriptions.size()) ? kv.flagDescriptions[i] : QString();
        QJsonObject fi; fi["bit"] = f.first; fi["label"] = f.second.first; fi["def"] = f.second.second; fi["desc"] = desc;
        flags.append(fi);
    }
    o["flags"] = flags;
    return o;
}

static FGDKeyvalue kvFromJson(const QJsonObject &o) {
    FGDKeyvalue kv;
    kv.name = o["name"].toString();
    kv.type = o["type"].toString();
    kv.displayName = o["displayName"].toString();
    kv.defaultValue = o["defaultValue"].toString();
    kv.description = o["description"].toString();
    kv.readOnly = o["readOnly"].toBool();
    kv.report = o["report"].toBool();
    for (auto v : o["choices"].toArray()) {
        auto ci = v.toObject();
        kv.choiceItems.append({ci["value"].toString(), ci["label"].toString()});
        kv.choiceDescriptions.append({ci["value"].toString(), ci["desc"].toString()});
    }
    for (auto v : o["flags"].toArray()) {
        auto fi = v.toObject();
        kv.flagItems.append({fi["bit"].toString(), {fi["label"].toString(), fi["def"].toInt()}});
        kv.flagDescriptions.append(fi["desc"].toString());
    }
    return kv;
}

static QJsonObject ioToJson(const FGDIO &io) {
    QJsonObject o;
    o["isInput"] = io.isInput;
    o["name"] = io.name;
    o["type"] = io.type;
    o["description"] = io.description;
    return o;
}

static FGDIO ioFromJson(const QJsonObject &o) {
    FGDIO io;
    io.isInput = o["isInput"].toBool(true);
    io.name = o["name"].toString();
    io.type = o["type"].toString();
    io.description = o["description"].toString();
    return io;
}

static QJsonObject childToJson(const FGDAutoVisGroupChild &c);

static QJsonObject childToJson(const FGDAutoVisGroupChild &c) {
    QJsonObject co;
    co["name"] = c.name;
    QJsonArray entArr;
    for (auto &e : c.entities) entArr.append(e);
    co["entities"] = entArr;
    QJsonArray childArr;
    for (auto &sc : c.children) childArr.append(childToJson(sc));
    co["children"] = childArr;
    return co;
}

static FGDAutoVisGroupChild childFromJson(const QJsonObject &co);

static FGDAutoVisGroupChild childFromJson(const QJsonObject &co) {
    FGDAutoVisGroupChild c;
    c.name = co["name"].toString();
    for (auto ev : co["entities"].toArray()) c.entities << ev.toString();
    for (auto cv : co["children"].toArray()) c.children << childFromJson(cv.toObject());
    return c;
}

static QJsonObject entityToJson(const FGDEntity &e) {
    QJsonObject o;
    o["classType"]  = (int)e.classType;
    o["className"]  = e.className;
    o["description"] = e.description;
    o["bases"]      = QJsonArray::fromStringList(e.bases);
    o["studioModel"] = e.studioModel;
    o["studioprop"] = e.studioprop;
    o["iconSprite"] = e.iconSprite;
    o["color"]      = e.color;
    o["size"]       = e.size;
    o["halfGridSnap"] = e.halfGridSnap;
    o["sphere"]     = e.sphere;
    o["line"]       = e.line;
    o["vecline"]    = e.vecline;
    o["axis"]       = e.axis;
    o["wirebox"]    = e.wirebox;
    o["origin"]     = e.origin;
    o["lightprop"]  = e.lightprop;
    o["decal"]      = e.decal;
    o["overlay"]    = e.overlay;
    o["sprite"]     = e.sprite;
    o["sweptplayerhull"] = e.sweptplayerhull;
    o["instance_"]  = e.instance_;
    o["animator"]   = e.animator;
    o["keyframe_"]  = e.keyframe_;
    o["worldtext"]  = e.worldtext;
    QJsonArray kvs;
    for (auto &kv : e.keyvalues) kvs.append(kvToJson(kv));
    o["keyvalues"] = kvs;
    QJsonArray ios;
    for (auto &io : e.ios) ios.append(ioToJson(io));
    o["ios"] = ios;
    return o;
}

static FGDEntity entityFromJson(const QJsonObject &o) {
    FGDEntity e;
    e.classType   = (EntityClassType)o["classType"].toInt();
    e.className   = o["className"].toString();
    e.description = o["description"].toString();
    for (auto v : o["bases"].toArray()) e.bases << v.toString();
    e.studioModel = o["studioModel"].toString();
    e.studioprop  = o["studioprop"].toString();
    e.iconSprite  = o["iconSprite"].toString();
    e.color       = o["color"].toString();
    e.size        = o["size"].toString();
    e.halfGridSnap = o["halfGridSnap"].toBool();
    e.sphere      = o["sphere"].toString();
    e.line        = o["line"].toString();
    e.vecline     = o["vecline"].toString();
    e.axis        = o["axis"].toString();
    e.wirebox     = o["wirebox"].toString();
    e.origin      = o["origin"].toString();
    e.lightprop   = o["lightprop"].toString();
    e.decal       = o["decal"].toBool();
    e.overlay     = o["overlay"].toBool();
    e.sprite      = o["sprite"].toBool();
    e.sweptplayerhull = o["sweptplayerhull"].toBool();
    e.instance_   = o["instance_"].toBool();
    e.animator    = o["animator"].toBool();
    e.keyframe_   = o["keyframe_"].toBool();
    e.worldtext   = o["worldtext"].toBool();
    for (auto v : o["keyvalues"].toArray()) e.keyvalues.append(kvFromJson(v.toObject()));
    for (auto v : o["ios"].toArray())       e.ios.append(ioFromJson(v.toObject()));
    return e;
}

class EntityListCommand : public QUndoCommand {
public:
    EntityListCommand(QList<FGDEntity> *list, const QList<FGDEntity> &before, const QList<FGDEntity> &after,
                      const QString &text, std::function<void()> refresh)
        : QUndoCommand(text), m_list(list), m_before(before), m_after(after), m_refresh(refresh) {}
    void undo() override { *m_list = m_before; m_refresh(); }
    void redo() override { *m_list = m_after; m_refresh(); }
private:
    QList<FGDEntity> *m_list;
    QList<FGDEntity> m_before, m_after;
    std::function<void()> m_refresh;
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("FGDBuilder");
    setWindowIcon(QIcon(":/icons/app.png"));
    setMinimumSize(900, 600);
    m_undoStack = new QUndoStack(this);
    setupUi();
    setupMenuBar();
    setupToolBar();
    statusBar()->addWidget(m_statusLabel = new QLabel("Ready"));

    QSettings settings("FGDBuilder", "FGDBuilder");
    m_recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFilesMenu();

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
        QString title = "FGDBuilder";
        if (!m_filePath.isEmpty()) title += " - " + QFileInfo(m_filePath).fileName();
        if (!clean) title += " *";
        setWindowTitle(title);
    });
    DiscordRPC::instance().setActivity("No file open", "FGDBuilder v1.1.0");
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!confirmDiscard()) {
        event->ignore();
        return;
    }
    QSettings settings("FGDBuilder", "FGDBuilder");
    settings.setValue("recentFiles", m_recentFiles);
    event->accept();
}

void MainWindow::setupUi() {
    QWidget *central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    QWidget *leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->addWidget(new QLabel("Entities:"));

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search entities...");
    leftLayout->addWidget(m_searchEdit);

    m_entityList = new QListWidget;
    m_entityList->setMinimumWidth(220);
    leftLayout->addWidget(m_entityList);

    QHBoxLayout *entityBtns = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Add");
    QPushButton *editBtn = new QPushButton("Edit");
    QPushButton *removeBtn = new QPushButton("Remove");
    QPushButton *dupBtn = new QPushButton("Duplicate");
    entityBtns->addWidget(addBtn);
    entityBtns->addWidget(editBtn);
    entityBtns->addWidget(removeBtn);
    entityBtns->addWidget(dupBtn);
    leftLayout->addLayout(entityBtns);

    QHBoxLayout *moveRow = new QHBoxLayout;
    QPushButton *upBtn = new QPushButton("Move Up");
    QPushButton *downBtn = new QPushButton("Move Down");
    moveRow->addWidget(upBtn);
    moveRow->addWidget(downBtn);
    moveRow->addStretch();
    leftLayout->addLayout(moveRow);

    splitter->addWidget(leftPanel);

    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0,0,0,0);
    rightLayout->addWidget(new QLabel("FGD Preview:"));
    m_preview = new QTextEdit;
    m_preview->setReadOnly(true);
    QFont monoFont("Courier New", 9);
    monoFont.setStyleHint(QFont::Monospace);
    m_preview->setFont(monoFont);
    m_preview->setStyleSheet("QTextEdit { background: #1e1e1e; color: #d4d4d4; }");
    new FGDHighlighter(m_preview->document());
    rightLayout->addWidget(m_preview);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({240, 600});

    mainLayout->addWidget(splitter);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addEntity);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::editEntity);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::removeEntity);
    connect(dupBtn, &QPushButton::clicked, this, &MainWindow::duplicateEntity);
    connect(upBtn, &QPushButton::clicked, this, &MainWindow::moveEntityUp);
    connect(downBtn, &QPushButton::clicked, this, &MainWindow::moveEntityDown);
    connect(m_entityList, &QListWidget::currentRowChanged, this, &MainWindow::onEntitySelectionChanged);
    connect(m_entityList, &QListWidget::itemDoubleClicked, this, &MainWindow::editEntity);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
}

static QAction* makeAction(const QString &text, QObject *receiver, std::function<void()> slot, const QKeySequence &key = {}) {
    QAction *a = new QAction(text);
    if (!key.isEmpty()) a->setShortcut(key);
    QObject::connect(a, &QAction::triggered, receiver, slot);
    return a;
}

void MainWindow::setupMenuBar() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(makeAction("&New", this, [this]{ newProject(); }, QKeySequence::New));
    fileMenu->addAction(makeAction("&Open...", this, [this]{ openProject(); }, QKeySequence::Open));
    fileMenu->addAction(makeAction("&Import FGD...", this, [this]{ importFGD(); }, QKeySequence("Ctrl+I")));
    fileMenu->addSeparator();
    fileMenu->addAction(makeAction("&Save", this, [this]{ saveProject(); }, QKeySequence::Save));
    fileMenu->addAction(makeAction("Save &As...", this, [this]{ saveProjectAs(); }, QKeySequence::SaveAs));
    fileMenu->addSeparator();
    fileMenu->addAction(makeAction("&Export FGD...", this, [this]{ exportFGD(); }, QKeySequence("Ctrl+E")));
    fileMenu->addSeparator();
    m_recentMenu = fileMenu->addMenu("Recent Files");
    fileMenu->addSeparator();
    fileMenu->addAction(makeAction("E&xit", qApp, []{ qApp->quit(); }, QKeySequence::Quit));

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_undoStack->createUndoAction(this, "&Undo"));
    editMenu->actions().last()->setShortcut(QKeySequence::Undo);
    editMenu->addAction(m_undoStack->createRedoAction(this, "&Redo"));
    editMenu->actions().last()->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction(makeAction("&Add Entity", this, [this]{ addEntity(); }, QKeySequence("Ctrl+N")));
    editMenu->addAction(makeAction("&Edit Entity", this, [this]{ editEntity(); }, QKeySequence("F2")));
    editMenu->addAction(makeAction("&Remove Entity", this, [this]{ removeEntity(); }, QKeySequence::Delete));
    editMenu->addAction(makeAction("D&uplicate Entity", this, [this]{ duplicateEntity(); }, QKeySequence("Ctrl+D")));
    editMenu->addSeparator();
    editMenu->addAction(makeAction("Move &Up", this, [this]{ moveEntityUp(); }, QKeySequence("Ctrl+Up")));
    editMenu->addAction(makeAction("Move &Down", this, [this]{ moveEntityDown(); }, QKeySequence("Ctrl+Down")));
    editMenu->addSeparator();
    editMenu->addAction(makeAction("Sort A-Z", this, [this]{ sortEntitiesAZ(); }));

    QMenu *projectMenu = menuBar()->addMenu("&Project");
    projectMenu->addAction(makeAction("&Header Comment...", this, [this]{ editHeaderComment(); }));
    projectMenu->addAction(makeAction("&@include Files...", this, [this]{ editIncludes(); }));
    projectMenu->addAction(makeAction("&@mapsize...", this, [this]{ editMapSize(); }));
    projectMenu->addAction(makeAction("@&MaterialExclusion...", this, [this]{ editMaterialExclusions(); }));
    projectMenu->addAction(makeAction("@&AutoVisGroup...", this, [this]{ editAutoVisGroups(); }));
    projectMenu->addAction(makeAction("@&version...", this, [this]{ editFGDVersion(); }));

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(makeAction("&About FGDBuilder", this, [this]{ showAbout(); }));
}

void MainWindow::setupToolBar() {
    QToolBar *tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction(QIcon(":/icons/new.png"), "New", this, &MainWindow::newProject);
    tb->addAction(QIcon(":/icons/open.png"), "Open", this, &MainWindow::openProject);
    tb->addAction(QIcon(":/icons/save.png"), "Save", this, &MainWindow::saveProject);
    tb->addAction(QIcon(":/icons/export.png"), "Export FGD", this, &MainWindow::exportFGD);
    tb->addAction(QIcon(":/icons/add.png"), "Add Entity", this, &MainWindow::addEntity);
    tb->addAction(QIcon(":/icons/delete.png"), "Remove Entity", this, &MainWindow::removeEntity);
    tb->addAction(QIcon(":/icons/edit.png"), "Edit Entity", this, &MainWindow::editEntity);
}

void MainWindow::refreshEntityList() {
    QString filter = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : QString();
    int prevRealIndex = -1;
    if (m_entityList->currentItem())
        prevRealIndex = m_entityList->currentItem()->data(Qt::UserRole).toInt();

    m_entityList->clear();
    for (int i = 0; i < m_project.entities.size(); ++i) {
        const FGDEntity &e = m_project.entities[i];
        QString label = classTypeToString(e.classType) + " " + e.className;
        if (!filter.isEmpty() && !label.toLower().contains(filter)) continue;
        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, i);
        m_entityList->addItem(item);
        if (i == prevRealIndex) m_entityList->setCurrentItem(item);
    }
    updatePreview();
}

int MainWindow::realIndexFromRow(int row) const {
    if (row < 0 || row >= m_entityList->count()) return -1;
    return m_entityList->item(row)->data(Qt::UserRole).toInt();
}

void MainWindow::onSearchTextChanged(const QString &) {
    refreshEntityList();
}

void MainWindow::setWindowModified_(bool modified) {
    QString title = "FGDBuilder";
    if (!m_filePath.isEmpty()) title += " - " + QFileInfo(m_filePath).fileName();
    if (modified) title += " *";
    setWindowTitle(title);
}

bool MainWindow::confirmDiscard() {
    if (m_undoStack->isClean()) return true;
    auto r = QMessageBox::question(this, "Unsaved Changes",
        "You have unsaved changes. Discard them?",
        QMessageBox::Yes | QMessageBox::No);
    return r == QMessageBox::Yes;
}

void MainWindow::newProject() {
    if (!confirmDiscard()) return;
    m_project = FGDProject();
    m_filePath.clear();
    m_undoStack->clear();
    refreshEntityList();
    setWindowModified_(false);
    m_statusLabel->setText("New project created.");
    DiscordRPC::instance().setActivity("New project", "FGDBuilder v1.1.0");
}

void MainWindow::loadFromFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + path);
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    QJsonObject root = doc.object();
    FGDProject proj;
    proj.headerComment = root["headerComment"].toString();
    for (auto v : root["includes"].toArray()) proj.includes << v.toString();
    proj.fgdVersion = root["fgdVersion"].toInt(0);
    proj.mapsizeMin = root["mapsizeMin"].toInt(-16384);
    proj.mapsizeMax = root["mapsizeMax"].toInt(16384);
    proj.useMapsSize = root["useMapsSize"].toBool(false);
    for (auto v : root["materialExclusions"].toArray()) proj.materialExclusions << v.toString();
    for (auto v : root["entities"].toArray()) proj.entities.append(entityFromJson(v.toObject()));
    QJsonArray avg = root["autoVisGroups"].toArray();
    for (auto gv : avg) {
        FGDAutoVisGroup g;
        QJsonObject go = gv.toObject();
        g.parent = go["parent"].toString();
        for (auto cv : go["children"].toArray())
            g.children.append(childFromJson(cv.toObject()));
        proj.autoVisGroups.append(g);
    }
    m_project = proj;
    m_filePath = path;
    m_undoStack->clear();
    m_undoStack->setClean();
    addRecentFile(path);
    refreshEntityList();
    setWindowModified_(false);
    m_statusLabel->setText("Loaded: " + path);
    DiscordRPC::instance().setActivity(QFileInfo(path).fileName(), "FGDBuilder v1.1.0");
}

void MainWindow::saveToFile(const QString &path) {
    QJsonObject root;
    root["headerComment"] = m_project.headerComment;
    QJsonArray incs;
    for (auto &s : m_project.includes) incs.append(s);
    root["includes"] = incs;
    root["fgdVersion"] = m_project.fgdVersion;
    root["mapsizeMin"] = m_project.mapsizeMin;
    root["mapsizeMax"] = m_project.mapsizeMax;
    root["useMapsSize"] = m_project.useMapsSize;
    QJsonArray matEx;
    for (auto &s : m_project.materialExclusions) matEx.append(s);
    root["materialExclusions"] = matEx;
    QJsonArray ents;
    for (auto &e : m_project.entities) ents.append(entityToJson(e));
    root["entities"] = ents;
    QJsonArray avgArr;
    for (auto &g : m_project.autoVisGroups) {
        QJsonObject go;
        go["parent"] = g.parent;
        QJsonArray childArr;
        for (auto &c : g.children) childArr.append(childToJson(c));
        go["children"] = childArr;
        avgArr.append(go);
    }
    root["autoVisGroups"] = avgArr;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Cannot write file: " + path);
        return;
    }
    f.write(QJsonDocument(root).toJson());
    f.close();
    m_filePath = path;
    m_undoStack->setClean();
    addRecentFile(path);
    setWindowModified_(false);
    m_statusLabel->setText("Saved: " + path);
    DiscordRPC::instance().setActivity(QFileInfo(path).fileName(), "FGDBuilder v1.1.0");
}

void MainWindow::openProject() {
    if (!confirmDiscard()) return;
    QString path = QFileDialog::getOpenFileName(this, "Open Project", "", "FGDBuilder Project (*.fgdbp);;All Files (*)");
    if (!path.isEmpty()) loadFromFile(path);
}

void MainWindow::importFGD() {
    QString path = QFileDialog::getOpenFileName(this, "Import FGD", "", "Forge Game Data (*.fgd);;All Files (*)");
    if (path.isEmpty()) return;
    QString err;
    FGDProject imported = FGDImporter::importFromFile(path, err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Import Error", err);
        return;
    }
    QString msg = QString("Imported %1 entities from:\n%2\n\nReplace current project or merge into it?")
        .arg(imported.entities.size()).arg(path);
    QMessageBox dlg(this);
    dlg.setWindowTitle("Import FGD");
    dlg.setText(msg);
    QPushButton *replaceBtn = dlg.addButton("Replace", QMessageBox::AcceptRole);
    QPushButton *mergeBtn   = dlg.addButton("Merge",   QMessageBox::AcceptRole);
    dlg.addButton("Cancel", QMessageBox::RejectRole);
    dlg.exec();
    if (dlg.clickedButton() == replaceBtn) {
        if (!confirmDiscard()) return;
        m_project = imported;
        m_filePath.clear();
        m_undoStack->clear();
        refreshEntityList();
        m_undoStack->resetClean();
        setWindowModified_(true);
        m_statusLabel->setText("Imported (replace): " + path);
        DiscordRPC::instance().setActivity("Imported: " + QFileInfo(path).fileName(), "FGDBuilder v1.1.0");
    } else if (dlg.clickedButton() == mergeBtn) {
        auto before = m_project.entities;
        auto after = m_project.entities;
        for (auto &e : imported.entities) after.append(e);
        m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Import FGD (merge)",
            [this]{ refreshEntityList(); }));
        m_statusLabel->setText("Imported (merge): " + path);
    }
}

void MainWindow::saveProject() {
    if (m_filePath.isEmpty()) { saveProjectAs(); return; }
    saveToFile(m_filePath);
}

void MainWindow::saveProjectAs() {
    QString path = QFileDialog::getSaveFileName(this, "Save Project As", "", "FGDBuilder Project (*.fgdbp);;All Files (*)");
    if (!path.isEmpty()) saveToFile(path);
}

QString MainWindow::defaultExportPath() const {
    if (!m_filePath.isEmpty()) {
        QFileInfo fi(m_filePath);
        return fi.absoluteDir().filePath(fi.baseName() + ".fgd");
    }
    return QString();
}

static QStringList validateProject(const FGDProject &proj) {
    QStringList errors;
    QSet<QString> names;
    QSet<QString> allNames;
    for (auto &e : proj.entities) allNames.insert(e.className);
    for (auto &e : proj.entities) {
        if (e.className.isEmpty()) { errors << "Entity with empty class name found."; continue; }
        if (names.contains(e.className))
            errors << "Duplicate entity name: " + e.className;
        names.insert(e.className);
        for (auto &base : e.bases) {
            if (!allNames.contains(base))
                errors << e.className + ": base class '" + base + "' not found in project.";
        }
        for (auto &kv : e.keyvalues) {
            if (kv.type == "flags") {
                for (auto &fi : kv.flagItems) {
                    bool ok;
                    long long val = fi.first.toLongLong(&ok);
                    if (!ok || val <= 0 || (val & (val - 1)) != 0)
                        errors << e.className + ": flag value '" + fi.first + "' is not a power of two.";
                }
            }
        }
    }
    return errors;
}

void MainWindow::exportFGD() {
    QStringList validationErrors = validateProject(m_project);
    if (!validationErrors.isEmpty()) {
        QString msg = "The following issues were found:\n\n" + validationErrors.join("\n") + "\n\nExport anyway?";
        auto r = QMessageBox::warning(this, "Validation Warnings", msg, QMessageBox::Yes | QMessageBox::No);
        if (r != QMessageBox::Yes) return;
    }
    QString defPath = defaultExportPath();
    QString path = QFileDialog::getSaveFileName(this, "Export FGD", defPath, "Forge Game Data (*.fgd);;All Files (*)");
    if (path.isEmpty()) return;
    QString content = FGDGenerator::generate(m_project);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot write file: " + path);
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    f.close();
    m_statusLabel->setText("Exported: " + path);
    QMessageBox::information(this, "Export Successful", "FGD exported to:\n" + path);
}

void MainWindow::addEntity() {
    QStringList bases;
    for (auto &e : m_project.entities)
        if (e.classType == EntityClassType::BaseClass)
            bases << e.className;
    EntityEditor dlg(this, FGDEntity(), bases);
    if (dlg.exec() == QDialog::Accepted) {
        FGDEntity e = dlg.getEntity();
        if (e.className.isEmpty()) { QMessageBox::warning(this, "Warning", "Entity class name cannot be empty."); return; }
        auto before = m_project.entities;
        auto after = m_project.entities;
        after.append(e);
        int newRow = after.size() - 1;
        m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Add Entity",
            [this, newRow]{ refreshEntityList(); for (int i = 0; i < m_entityList->count(); ++i) { if (m_entityList->item(i)->data(Qt::UserRole).toInt() == newRow) { m_entityList->setCurrentRow(i); break; } } }));
    }
}

void MainWindow::editEntity() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx < 0) return;
    QStringList bases;
    for (auto &e : m_project.entities)
        if (e.classType == EntityClassType::BaseClass)
            bases << e.className;
    EntityEditor dlg(this, m_project.entities[realIdx], bases);
    if (dlg.exec() == QDialog::Accepted) {
        FGDEntity e = dlg.getEntity();
        if (e.className.isEmpty()) { QMessageBox::warning(this, "Warning", "Entity class name cannot be empty."); return; }
        auto before = m_project.entities;
        auto after = m_project.entities;
        after[realIdx] = e;
        m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Edit Entity",
            [this, realIdx]{ refreshEntityList(); for (int i = 0; i < m_entityList->count(); ++i) { if (m_entityList->item(i)->data(Qt::UserRole).toInt() == realIdx) { m_entityList->setCurrentRow(i); break; } } }));
    }
}

void MainWindow::removeEntity() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx < 0) return;
    auto r = QMessageBox::question(this, "Remove Entity",
        "Remove entity '" + m_project.entities[realIdx].className + "'?",
        QMessageBox::Yes | QMessageBox::No);
    if (r != QMessageBox::Yes) return;
    auto before = m_project.entities;
    auto after = m_project.entities;
    after.removeAt(realIdx);
    m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Remove Entity",
        [this]{ refreshEntityList(); }));
}

void MainWindow::duplicateEntity() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx < 0) return;
    FGDEntity copy = m_project.entities[realIdx];
    copy.className += "_copy";
    auto before = m_project.entities;
    auto after = m_project.entities;
    after.insert(realIdx + 1, copy);
    int newReal = realIdx + 1;
    m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Duplicate Entity",
        [this, newReal]{ refreshEntityList(); for (int i = 0; i < m_entityList->count(); ++i) { if (m_entityList->item(i)->data(Qt::UserRole).toInt() == newReal) { m_entityList->setCurrentRow(i); break; } } }));
}

void MainWindow::moveEntityUp() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx <= 0) return;
    auto before = m_project.entities;
    auto after = m_project.entities;
    after.swapItemsAt(realIdx, realIdx - 1);
    int newReal = realIdx - 1;
    m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Move Entity Up",
        [this, newReal]{ refreshEntityList(); for (int i = 0; i < m_entityList->count(); ++i) { if (m_entityList->item(i)->data(Qt::UserRole).toInt() == newReal) { m_entityList->setCurrentRow(i); break; } } }));
}

void MainWindow::moveEntityDown() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx < 0 || realIdx >= m_project.entities.size() - 1) return;
    auto before = m_project.entities;
    auto after = m_project.entities;
    after.swapItemsAt(realIdx, realIdx + 1);
    int newReal = realIdx + 1;
    m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Move Entity Down",
        [this, newReal]{ refreshEntityList(); for (int i = 0; i < m_entityList->count(); ++i) { if (m_entityList->item(i)->data(Qt::UserRole).toInt() == newReal) { m_entityList->setCurrentRow(i); break; } } }));
}

void MainWindow::sortEntitiesAZ() {
    auto before = m_project.entities;
    auto after = m_project.entities;
    std::sort(after.begin(), after.end(), [](const FGDEntity &a, const FGDEntity &b){
        return a.className.toLower() < b.className.toLower();
    });
    m_undoStack->push(new EntityListCommand(&m_project.entities, before, after, "Sort Entities A-Z",
        [this]{ refreshEntityList(); }));
}

void MainWindow::onEntitySelectionChanged() {
    updatePreview();
    QString fileLabel = m_filePath.isEmpty() ? "Untitled" : QFileInfo(m_filePath).fileName();
    DiscordRPC::instance().setActivity(fileLabel, "FGDBuilder v1.1.0");
}

void MainWindow::updatePreview() {
    int row = m_entityList->currentRow();
    int realIdx = realIndexFromRow(row);
    if (realIdx >= 0) {
        FGDProject single;
        single.entities.append(m_project.entities[realIdx]);
        m_preview->setPlainText(FGDGenerator::generate(single).trimmed());
    } else {
        m_preview->setPlainText(FGDGenerator::generate(m_project));
    }
}

void MainWindow::editIncludes() {
    IncludeManager dlg(this, m_project.includes);
    if (dlg.exec() == QDialog::Accepted) {
        m_project.includes = dlg.getIncludes();
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::editMaterialExclusions() {
    BaseClassManager dlg(this, m_project.materialExclusions);
    if (dlg.exec() == QDialog::Accepted) {
        m_project.materialExclusions = dlg.getExclusions();
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::editAutoVisGroups() {
    AutoVisGroupEditor dlg(this, m_project.autoVisGroups);
    if (dlg.exec() == QDialog::Accepted) {
        m_project.autoVisGroups = dlg.getGroups();
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::editMapSize() {
    MapSizeDialog dlg(this, m_project.useMapsSize, m_project.mapsizeMin, m_project.mapsizeMax);
    if (dlg.exec() == QDialog::Accepted) {
        m_project.useMapsSize = dlg.isEnabled();
        m_project.mapsizeMin = dlg.getMin();
        m_project.mapsizeMax = dlg.getMax();
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::editHeaderComment() {
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, "Header Comment",
        "Comment lines at the top of the FGD file:", m_project.headerComment, &ok);
    if (ok) {
        m_project.headerComment = text;
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::editFGDVersion() {
    bool ok;
    int v = QInputDialog::getInt(this, "@version",
        "FGD version number (0 = omit @version from output):",
        m_project.fgdVersion, 0, 9999, 1, &ok);
    if (ok) {
        m_project.fgdVersion = v;
        m_undoStack->resetClean();
        updatePreview();
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "About FGDBuilder",
        "<b>FGDBuilder v1.1.0</b><br>"
        "Build date: " __DATE__ "<br><br>"
        "A tool for creating Forge Game Data (.fgd) files.<br><br>"
        "Supports @PointClass, @SolidClass, @NPCClass,<br>"
        "@BaseClass, @FilterClass, @KeyFrameClass, @MoveClass,<br>"
        "@include, @mapsize, @MaterialExclusion, @AutoVisGroup.<br><br>"
        "FGD files define entities for use in Valve's Hammer editor.<br><br>"
        "<a href='https://github.com/bswap64/fgdbuilder'>https://github.com/bswap64/fgdbuilder</a>");
}

void MainWindow::addRecentFile(const QString &path) {
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > MaxRecentFiles) m_recentFiles.removeLast();
    updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu() {
    if (!m_recentMenu) return;
    m_recentMenu->clear();
    if (m_recentFiles.isEmpty()) {
        m_recentMenu->addAction("(empty)")->setEnabled(false);
        return;
    }
    for (const QString &path : m_recentFiles) {
        QFileInfo fi(path);
        QString label = fi.fileName() + "  [" + fi.absolutePath() + "]";
        QAction *a = m_recentMenu->addAction(label);
        a->setToolTip(path);
        connect(a, &QAction::triggered, this, [this, path]{ openRecentFile(path); });
    }
}

void MainWindow::openRecentFile(const QString &path) {
    if (!confirmDiscard()) return;
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "File Not Found", "File not found:\n" + path);
        m_recentFiles.removeAll(path);
        updateRecentFilesMenu();
        return;
    }
    loadFromFile(path);
}
