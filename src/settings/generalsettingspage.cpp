#include "generalsettingspage.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>

#include <KLocalizedString>

#include "settings.h"

GeneralSettingsPage::GeneralSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto formLayout = new QFormLayout(this);

    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    auto label = new QLabel(i18n("Use -1 to show all or 0 to show none"), this);
    auto submenuEntries = new QSpinBox(this);
    submenuEntries->setObjectName("kcfg_SubmenuEntriesCount");
    submenuEntries->setValue(QuickAccessSettings::submenuEntriesCount());
    submenuEntries->setMinimum(-1);
    submenuEntries->setMaximum(999);
    layout->addWidget(submenuEntries);
    layout->addWidget(label);
    formLayout->addRow(i18n("Submenu entries"), widget);

    auto showTrayIcon = new QCheckBox(this);
    showTrayIcon->setObjectName("kcfg_ShowInTray");
    showTrayIcon->setChecked(QuickAccessSettings::showInTray());
    showTrayIcon->setText(i18n("Show in system tray/notification area"));
    formLayout->addRow(QString(), showTrayIcon);

    auto useSections = new QCheckBox(this);
    useSections->setObjectName("kcfg_UseSections");
    useSections->setChecked(QuickAccessSettings::useSections());
    useSections->setText(i18n("Use sections instead of separators"));
    formLayout->addRow(QString(), useSections);

    auto *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    formLayout->addItem(spacer);
    auto *openStartUpDialogBtn = new QPushButton(this);
    openStartUpDialogBtn->setText(i18n("Open startup dialog"));
    openStartUpDialogBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    formLayout->addRow(openStartUpDialogBtn);


    connect(submenuEntries, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneralSettingsPage::changed);
    connect(showTrayIcon, &QCheckBox::clicked, this,
            &GeneralSettingsPage::changed);
    connect(useSections, &QCheckBox::clicked,
            this, &GeneralSettingsPage::changed);
    connect(openStartUpDialogBtn, &QPushButton::clicked,
            this, &GeneralSettingsPage::openStartUpDialog);
}
