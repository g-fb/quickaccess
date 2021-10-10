#ifndef FOLDERSSETTINGSPAGE_H
#define FOLDERSSETTINGSPAGE_H

#include <QWidget>

class FoldersSettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FoldersSettingsPage(QWidget *parent = nullptr);

Q_SIGNALS:
    void changed();
};

#endif // FOLDERSSETTINGSPAGE_H
