#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include "FGDData.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void exportFGD();
    void addEntity();
    void editEntity();
    void removeEntity();
    void duplicateEntity();
    void moveEntityUp();
    void moveEntityDown();
    void onEntitySelectionChanged();
    void editIncludes();
    void editMaterialExclusions();
    void editAutoVisGroups();
    void editMapSize();
    void editHeaderComment();
    void showAbout();
    void updatePreview();
private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void refreshEntityList();
    void setWindowModified_(bool modified);
    bool confirmDiscard();
    void loadFromFile(const QString &path);
    void saveToFile(const QString &path);
    QString projectFilePath() const { return m_filePath; }

    QListWidget *m_entityList;
    QTextEdit *m_preview;
    QLabel *m_statusLabel;
    FGDProject m_project;
    QString m_filePath;
    bool m_modified = false;
};
