#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QLineEdit>
#include <QUndoStack>
#include <QMenu>
#include "FGDData.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void newProject();
    void openProject();
    void importFGD();
    void saveProject();
    void saveProjectAs();
    void exportFGD();
    void addEntity();
    void editEntity();
    void removeEntity();
    void duplicateEntity();
    void moveEntityUp();
    void moveEntityDown();
    void sortEntitiesAZ();
    void onEntitySelectionChanged();
    void editIncludes();
    void editMaterialExclusions();
    void editAutoVisGroups();
    void editMapSize();
    void editHeaderComment();
    void editFGDVersion();
    void showAbout();
    void updatePreview();
    void onSearchTextChanged(const QString &text);
    void openRecentFile(const QString &path);
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
    void addRecentFile(const QString &path);
    void updateRecentFilesMenu();
    QString defaultExportPath() const;
    int realIndexFromRow(int row) const;

    QListWidget *m_entityList;
    QTextEdit *m_preview;
    QLabel *m_statusLabel;
    QLineEdit *m_searchEdit;
    FGDProject m_project;
    QString m_filePath;
    QUndoStack *m_undoStack;
    QMenu *m_recentMenu;
    QStringList m_recentFiles;
    static const int MaxRecentFiles = 10;
};
