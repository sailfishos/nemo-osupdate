#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_LISTTRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_LISTTRANSACTIONPKGKIT_H

#include "transaction.h"
#include "collection.h"

#include <QString>
#include <QStringList>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class ListTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    ListTransactionPkgKit(Collection::Ptr collection);
    virtual ~ListTransactionPkgKit();

    virtual void start();

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);
    void slotPackage(PackageKit::Transaction::Info info,
                     const QString& pkgId,
                     const QString& summary);

private:
    Collection::Ptr myCollection;
    bool myFailureHandled;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_LISTTRANSACTIONPKGKIT_H
