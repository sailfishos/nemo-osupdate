#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_OSUPDATESIZETRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_OSUPDATESIZETRANSACTIONPKGKIT_H

#include "transaction.h"
#include "manager.h"

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class OsUpdateSizeTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    explicit OsUpdateSizeTransactionPkgKit(Manager::UpgradeInformation::Ptr info,
                                           const QString &upgradeDistroId);
    virtual ~OsUpdateSizeTransactionPkgKit();

    virtual void start();

private:
    void calculate();
    qlonglong parseNeededSpace(const QString &details);

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);

private:
    enum State {
        Calculate,
        Success
    };

    Manager::UpgradeInformation::Ptr myUpgradeInfo;
    State myState;
    QString myErrorDetails;
    QString myUpgradeDistroId;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_OSUPDATESIZETRANSACTIONPKGKIT_H
