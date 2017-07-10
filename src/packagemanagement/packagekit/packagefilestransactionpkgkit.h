#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEFILESTRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEFILESTRANSACTIONPKGKIT_H

#include "manager.h"
#include "transaction.h"

#include <QString>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

class PackageFilesTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    PackageFilesTransactionPkgKit(const QString& packageName, QStringList& files);
    virtual ~PackageFilesTransactionPkgKit();

    virtual void start();

private:
    void resolve();
    void getFiles();
    void prepareTransaction(PackageKit::Transaction* tx);

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);
    void slotFiles(const QString &packageID, const QStringList &filenames);

    void slotPackage(PackageKit::Transaction::Info info,
                     const QString& pkgId,
                     const QString& summary);

private:
    QString myPackageName;
    QString myPackageId;
    QStringList& myFiles;
    bool myFailureHandled;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEFILESTRANSACTIONPKGKIT_H
