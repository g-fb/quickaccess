#ifndef MIGRATIONS_H
#define MIGRATIONS_H

#include <KSharedConfig>

class Migrations
{
public:
    Migrations();

    void migrate();

private:
    bool migrateV2toV3();
    KSharedConfig::Ptr m_config {nullptr};
    QString m_version;
};

#endif // MIGRATIONS_H
