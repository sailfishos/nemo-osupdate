#include "listtransactionpkgkit.h"

#include <QDebug>

namespace PackageManagement
{

ListTransactionPkgKit::ListTransactionPkgKit(Collection::Ptr collection)
    : myCollection(collection)
    , myFailureHandled(false)
{

}

ListTransactionPkgKit::~ListTransactionPkgKit()
{
}

void ListTransactionPkgKit::start()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));

    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotPackage(PackageKit::Transaction::Info,QString,QString)));

    tx->getPackages(PackageKit::Transaction::FilterInstalled);
}

void ListTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    Q_UNUSED(runtime)

    myCollection->close();
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

void ListTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "ListTransactionPkgKit::slotTransactionError"
             << error << details;

    myFailureHandled = true;
    myCollection->close();
    deleteLater();
    emit failure(details);
    emit finished();
}

void ListTransactionPkgKit::slotPackage(PackageKit::Transaction::Info info,
                                        const QString& pkgId,
                                        const QString& summary)
{
    Q_UNUSED(info)

    const QString name = PackageKit::Transaction::packageName(pkgId);
    const QString version = PackageKit::Transaction::packageVersion(pkgId);

    Collection::Package pkg(name, version, summary);
    myCollection->append(pkg);
    emit progress(-1);
}

}
