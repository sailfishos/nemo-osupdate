#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTUPGRADETRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTUPGRADETRANSACTIONPKGKIT_H

#include "transaction.h"
#include "manager.h"

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class DistUpgradeTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    enum Action
    {
        Download,
        Upgrade
    };

    explicit DistUpgradeTransactionPkgKit(const QString &upgradeDistroId, bool downloadOnly);
    virtual ~DistUpgradeTransactionPkgKit();

    virtual void start();
    virtual void cancel();

    virtual bool isAtomic() const { return true; }

private:
    PackageKit::Transaction* makeTransaction();

    void download();
    void upgrade();

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);
    void slotPackage(PackageKit::Transaction::Info info,
                     const QString& packageId,
                     const QString& summary);
    void slotUpgradeProgress();

private:
    Action myAction;
    int myProgress;
    int myRetryCounter;
    QString myErrorDetails;
    QString myUpgradeDistroId;
    PackageKit::Transaction* myCurrentTransaction;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_DISTUPGRADETRANSACTIONPKGKIT_H
