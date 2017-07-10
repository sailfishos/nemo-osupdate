#include "packagefilestransactionpkgkit.h"

#include <QDebug>

namespace PackageManagement
{

PackageFilesTransactionPkgKit::PackageFilesTransactionPkgKit(const QString& packageName,
                                                             QStringList& files)
    : myPackageName(packageName)
    , myFiles(files)
    , myFailureHandled(false)
{
}

PackageFilesTransactionPkgKit::~PackageFilesTransactionPkgKit()
{
    qDebug() << "deleting PackageFilesTransactionPkgKit";
}

void PackageFilesTransactionPkgKit::start()
{
    resolve();
}

void PackageFilesTransactionPkgKit::resolve()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(slotPackage(PackageKit::Transaction::Info,QString,QString)));
    tx->resolve(myPackageName);
}

void PackageFilesTransactionPkgKit::getFiles()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(files(QString,QStringList)),
            this, SLOT(slotFiles(QString,QStringList)));

    tx->getFiles(myPackageId);
    myPackageId = QString();
}

void PackageFilesTransactionPkgKit::prepareTransaction(PackageKit::Transaction* tx)
{
    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));
    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));
}

void PackageFilesTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << "PackageFilesTransactionPkgKit::slotTransactionFinished" << status << runtime;

    if (status == PackageKit::Transaction::ExitSuccess) {
        if (myPackageId.length()) {
            getFiles();
        } else {
            deleteLater();
            emit success();
            emit finished();
        }
    } else if (!myFailureHandled) {
        deleteLater();
        emit failure(QString("unknown failure with exit status %1")
                     .arg(status));
        emit finished();
    }
}

void PackageFilesTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "PackageFilesTransactionPkgKit::slotTransactionError"
             << error << details;

    myFailureHandled = true;

    deleteLater();
    emit failure(details);
    emit finished();
}

void PackageFilesTransactionPkgKit::slotFiles(const QString &packageID,
                                              const QStringList &filenames)
{
    Q_UNUSED(packageID)
    myFiles = filenames;
}

void PackageFilesTransactionPkgKit::slotPackage(PackageKit::Transaction::Info info,
                                               const QString& pkgId,
                                               const QString& summary)
{
    Q_UNUSED(info)
    Q_UNUSED(summary)
    myPackageId = pkgId;
}

}
