#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <QWidget>

class GeneralSettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralSettingsPage(QWidget *parent = nullptr);

Q_SIGNALS:
    void changed();
};

#endif // GENERALSETTINGSPAGE_H
