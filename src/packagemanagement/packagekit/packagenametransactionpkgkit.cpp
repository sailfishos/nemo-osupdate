#include "packagenametransactionpkgkit.h"

#include <QDebug>

namespace PackageManagement
{

PackageNameTransactionPkgKit::PackageNameTransactionPkgKit(const QString& filePath,
                                                           Manager::PackageInformation::Ptr info)
    : myFilePath(filePath)
    , myInfo(info)
    , myFailureHandled(false)
{
}

PackageNameTransactionPkgKit::~PackageNameTransactionPkgKit()
{
    qDebug() << "deleting PackageNameTransactionPkgKit";
}

void PackageNameTransactionPkgKit::start()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));

    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(slotPackage(PackageKit::Transaction::Info,QString,QString)));

    myInfo->name = "";
    myInfo->version = "";
    myInfo->summary = "";
    myInfo->version = "";

    tx->searchFiles(myFilePath, PackageKit::Transaction::FilterInstalled);
}

void PackageNameTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << "PackageNameTransactionPkgKit::slotTransactionFinished" << status << runtime;

    deleteLater();

    if (status == PackageKit::Transaction::ExitSuccess) {
        emit success();
        emit finished();
    } else if (!myFailureHandled) {
        emit failure(QString("unknown failure with exit status %1")
                     .arg(status));
        emit finished();
    }
}

void PackageNameTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "PackageNameTransactionPkgKit::slotTransactionError"
             << error << details;

    myFailureHandled = true;

    deleteLater();
    emit failure(details);
    emit finished();
}

void PackageNameTransactionPkgKit::slotPackage(PackageKit::Transaction::Info info,
                                               const QString& pkgId,
                                               const QString& summary)
{
    Q_UNUSED(info)
    Q_UNUSED(summary)

    const QString name = PackageKit::Transaction::packageName(pkgId);
    const QString version = PackageKit::Transaction::packageVersion(pkgId);
    myInfo->name = name;
    myInfo->version = version;
    emit progress(-1);
}

}
