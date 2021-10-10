#include "generalsettingspage.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QtWidgets>
#include "settings.h"

GeneralSettingsPage::GeneralSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto formLayout = new QFormLayout(this);

    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);
    layout->setMargin(0);
    auto label = new QLabel(i18n("Use -1 to show all or 0 to show none"), this);
    auto submenuEntries = new QSpinBox(this);
    submenuEntries->setValue(QuickAccessSettings::submenuEntriesCount());
    submenuEntries->setMinimum(-1);
    submenuEntries->setMaximum(999);
    layout->addWidget(submenuEntries);
    layout->addWidget(label);
    formLayout->addRow(i18n("Extraction folder"), widget);

    auto showTrayIcon = new QCheckBox(this);
    showTrayIcon->setChecked(QuickAccessSettings::showInTray());
    showTrayIcon->setText(i18n("Show in system tray/notification area"));
    formLayout->addRow(QString(), showTrayIcon);

    auto useSections = new QCheckBox(this);
    useSections->setChecked(QuickAccessSettings::useSections());
    useSections->setText(i18n("Use sections instead of separators"));
    formLayout->addRow(QString(), useSections);

    connect(submenuEntries, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneralSettingsPage::changed);
    connect(showTrayIcon, &QCheckBox::clicked, this,
            &GeneralSettingsPage::changed);
    connect(useSections, &QCheckBox::clicked,
            this, &GeneralSettingsPage::changed);
}
