#ifndef UNINSTALLTRANSACTIONPKGKIT_H
#define UNINSTALLTRANSACTIONPKGKIT_H

#include "transaction.h"

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class UninstallTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    UninstallTransactionPkgKit(const QString& pkgId, bool autoRemove);

    virtual ~UninstallTransactionPkgKit();

    virtual void start();

private:
    void resolve(const QString& pkgId);
    void uninstall(const QString& pkg);
    void giveUp(const QString& details);

private slots:
    void slotResolveFinished(PackageKit::Transaction::Exit status,
                             uint runtime);
    void slotResolveError(PackageKit::Transaction::Error error,
                          const QString& details);
    void slotResolved(PackageKit::Transaction::Info info,
                      const QString& pkgId, const QString& summary);

    void slotUninstallFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotUninstallError(PackageKit::Transaction::Error error,
                          const QString& details);
    void slotUninstallProgress();

private:
    QString myPkgId;
    bool myIsAutoRemove;
    bool myIsResolved;
    PackageKit::Transaction* myCurrentTransaction;
};

}

#endif // UNINSTALLTRANSACTIONPKGKIT_H
