#include "uninstalltransactionpkgkit.h"

#include <QStringList>
#include <QDebug>

namespace PackageManagement
{

UninstallTransactionPkgKit::UninstallTransactionPkgKit(const QString &pkgId,
                                                       bool autoRemove)
    : myPkgId(pkgId)
    , myIsAutoRemove(autoRemove)
    , myIsResolved(false)
{

}

UninstallTransactionPkgKit::~UninstallTransactionPkgKit()
{
    qDebug() << "deleting UninstallTransactionPkgKit";
}

void UninstallTransactionPkgKit::start()
{
    qDebug() << "UninstallTransactionPkgKit::start:" << myPkgId;
    if (myPkgId.endsWith(".rpm")) {
        // if we got a local package, extract the package name from it
        QString front = myPkgId.split("/").last().split(".").first();
        myPkgId = front.left(front.lastIndexOf("-"));
    }
    resolve(myPkgId);
}

void UninstallTransactionPkgKit::resolve(const QString& pkgId)
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotResolveFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotResolveError(PackageKit::Transaction::Error,QString)));

    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotResolved(PackageKit::Transaction::Info,QString,QString)));

    tx->resolve(pkgId,
                PackageKit::Transaction::FilterInstalled);
}

void UninstallTransactionPkgKit::uninstall(const QString& pkg)
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotUninstallFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotUninstallError(PackageKit::Transaction::Error,QString)));

    connect(tx, SIGNAL(changed()),
            this, SLOT(slotUninstallProgress()));

    myCurrentTransaction = tx;
    tx->removePackage(pkg, false, myIsAutoRemove);
}

void UninstallTransactionPkgKit::giveUp(const QString& details)
{
    qDebug() << Q_FUNC_INFO << details;

    myCurrentTransaction = 0;
    deleteLater();
    emit failure(details);
    emit finished();
}

void UninstallTransactionPkgKit::slotUninstallFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << "slotUninstallFinished:" << myPkgId << status << runtime;

    myCurrentTransaction = 0;
    deleteLater();

    if (status == PackageKit::Transaction::ExitSuccess) {
        emit success();
        emit finished();
    }
}

void UninstallTransactionPkgKit::slotUninstallError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "Uninstall Error:" << myPkgId << error << details;

    giveUp(details);
}

void UninstallTransactionPkgKit::slotResolveFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)

    if (!myIsResolved) {
        // if the package could not be resolved locally, it is not installed,
        // in which case, uninstalling should just succeed
        myCurrentTransaction = 0;
        deleteLater();
        emit success();
        emit finished();
    }
}

void UninstallTransactionPkgKit::slotResolveError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "Resolve Error:" << myPkgId << error << details;

    giveUp(details);
}

void UninstallTransactionPkgKit::slotResolved(
        PackageKit::Transaction::Info info,
        const QString& pkgId,
        const QString& summary)
{
    qDebug() << "Resolved:" << info << pkgId << summary;

    myIsResolved = true;
    uninstall(pkgId);
}

void UninstallTransactionPkgKit::slotUninstallProgress()
{
    if (myCurrentTransaction) {
        emit progress(myCurrentTransaction->percentage());
    }
}


}
