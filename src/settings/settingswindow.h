#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <KConfigDialog>
#include <KConfigSkeleton>

class SettingsWindow : public KConfigDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent, KConfigSkeleton *skeleton);

Q_SIGNALS:
    void changed();
    void openStartUpDialog();

private:
    KSharedConfig::Ptr m_config {nullptr};
    bool m_changed {false};
    bool hasChanged();
};

#endif // SETTINGSWINDOW_H
