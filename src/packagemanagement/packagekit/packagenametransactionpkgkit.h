#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGENAMETRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGENAMETRANSACTIONPKGKIT_H

#include "manager.h"
#include "transaction.h"

#include <QString>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class PackageNameTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    PackageNameTransactionPkgKit(const QString& filePath,
                                 Manager::PackageInformation::Ptr info);
    virtual ~PackageNameTransactionPkgKit();

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
    QString myFilePath;
    Manager::PackageInformation::Ptr myInfo;
    bool myFailureHandled;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGENAMETRANSACTIONPKGKIT_H
