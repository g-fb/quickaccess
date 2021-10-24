#ifndef FOLDERSSETTINGSPAGE_H
#define FOLDERSSETTINGSPAGE_H

#include <QWidget>
#include <KSharedConfig>

class QListWidget;

class FoldersSettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FoldersSettingsPage(QWidget *parent = nullptr);

    void save();

Q_SIGNALS:
    void changed();

private:
    KSharedConfig::Ptr m_config;
    QListWidget *m_foldersListView;
    void deleteFolders();
};

#endif // FOLDERSSETTINGSPAGE_H
