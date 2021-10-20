#ifndef COMMANDSSETTINGSPAGE_H
#define COMMANDSSETTINGSPAGE_H

#include <QWidget>
#include <KSharedConfig>

class TreeWidget;
class QDialogButtonBox;
class QVBoxLayout;
class QLineEdit;
class KIconButton;
class QTreeWidgetItem;

class CommandsSettingsPage : public QWidget
{
    Q_OBJECT

    enum QA {
        DialogNewMode = Qt::UserRole,
        DialogEditMode
    };

public:
    explicit CommandsSettingsPage(QWidget *parent = nullptr);

    void save();
    void createMenuDialog();
    void createCommandDialog();
    void cloneCommand();
    void editCommand();
    void contextMenu();
    QTreeWidgetItem *createItem(KConfigGroup group);

Q_SIGNALS:
    void changed();

private:
    void deleteCommands();
    KSharedConfig::Ptr m_config;
    TreeWidget *m_commandsTree;
    QDialog *m_menuDialog;
    QDialogButtonBox *m_menuDialogButtonBox;
    QVBoxLayout *m_menuDialogLayout;

    QDialog *m_commandDialog;
    QDialogButtonBox *m_commandDialogButtonBox;
    QVBoxLayout *m_commandDialogLayout;

    QLineEdit *m_commandNameInput;
    QLineEdit *m_processNameInput;
    QLineEdit *m_argumentsInput;
    QLineEdit *m_menuNameInput;
    KIconButton *m_iconSelectButton;

    int m_dialogMode {-1};
};

#endif // COMMANDSSETTINGSPAGE_H
