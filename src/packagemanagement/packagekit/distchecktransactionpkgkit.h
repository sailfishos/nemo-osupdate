#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTCHECKTRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTCHECKTRANSACTIONPKGKIT_H

#include "transaction.h"
#include "manager.h"

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class DistCheckTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    enum Action
    {
        Refresh,
        CheckInstalled,
        CheckForUpgrade
    };

    explicit DistCheckTransactionPkgKit(Manager::UpgradeInformation::Ptr info,
                                        const QString &osVersionPackageRepo,
                                        const QString &osVersionPackageId,
                                        bool refreshCache);
    virtual ~DistCheckTransactionPkgKit();

    virtual void start();

private:
    PackageKit::Transaction* makeTransaction();

    void refresh();
    void checkInstalledVersion();
    void checkForUpgrade();

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);
    void slotVersionPackageResolved(PackageKit::Transaction::Info info,
                                    const QString& packageId,
                                    const QString& summary);

private:
    Action myAction;
    Manager::UpgradeInformation::Ptr myUpgradeInfo;
    bool myHaveUpgrade;
    QString myInstalledVersion;
    QString myOsVersionPackageRepo;
    QString myOsVersionPackageId;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTCHECKTRANSACTIONPKGKIT_H
