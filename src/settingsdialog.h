#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <KConfigDialog>
#include <KConfigSkeleton>

#include "ui_addmenu.h"
#include "ui_addcommand.h"

class Settings;
class TreeWidget;
class QTreeWidgetItem;

class AddMenu: public QDialog, public Ui::AddMenu
{
    Q_OBJECT
public:
    explicit AddMenu(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};


class AddCommand: public QDialog, public Ui::AddCommand
{
    Q_OBJECT
public:
    explicit AddCommand(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};


class SettingsDialog : public KConfigDialog
{
    Q_OBJECT

    enum QA {
        DialogNewMode = Qt::UserRole,
        DialogEditMode
    };

public:
    SettingsDialog(Settings *settings, QWidget *parent, const QString &name, KConfigSkeleton *config);

private slots:
    /**
     * Called when the user clicks Apply or OK.
     */
    void updateSettings() Q_DECL_OVERRIDE;
    /**
     * Updates dialog widgets. Here only used after loading a profile.
     * Profiles only store the settings of the last three pages in the dialog.
     */
    void updateWidgets() Q_DECL_OVERRIDE;
    /**
     * Called when the user clicks Default
     */
    void updateWidgetsDefault() Q_DECL_OVERRIDE;

    /**
     * Returns true if the current state of the dialog is different from the saved settings
     */
    bool hasChanged() Q_DECL_OVERRIDE;


private:

    /**
     * Returns true if the current state of the dialog represents the default settings.
     */
    bool isDefault() Q_DECL_OVERRIDE;

    KSharedConfig::Ptr m_config;
    Settings *m_settings;
    TreeWidget *m_commandsTree;
    AddCommand *m_addCommandDialog;
    AddMenu *m_addMenuDialog;
    bool m_changed;
    int m_dialogMode;
    QTreeWidgetItem *createItemFromConfig(KConfigGroup group);
    void contextMenu();
    void saveCommands();
    void deleteCommands();
    void editCommand();
    void cloneCommand();
    void populateTree();
    void manageCommand();
    void addMenu();
    void manageMenu();
};

#endif // SETTINGSDIALOG_H
