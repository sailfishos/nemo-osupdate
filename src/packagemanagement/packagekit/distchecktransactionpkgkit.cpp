#include "distchecktransactionpkgkit.h"

#include <QDir>
#include <QFile>
#include <QDebug>

namespace PackageManagement
{

DistCheckTransactionPkgKit::DistCheckTransactionPkgKit(Manager::UpgradeInformation::Ptr info,
        const QString &osVersionPackageRepo,
        const QString &osVersionPackageId,
        bool refreshCache)
    : myAction(refreshCache ? Refresh : CheckInstalled)
    , myUpgradeInfo(info)
    , myHaveUpgrade(false)
    , myOsVersionPackageRepo(osVersionPackageRepo)
    , myOsVersionPackageId(osVersionPackageId)
{
    /* Checking for upgrades works like this:
     *
     * 1. refresh package cache
     * 2. check for update for the version package
     * 3. if update was found, retrieve OS version and summary from it
     */
}

DistCheckTransactionPkgKit::~DistCheckTransactionPkgKit()
{
    qDebug() << Q_FUNC_INFO;
}

void DistCheckTransactionPkgKit::start()
{
    qDebug() << Q_FUNC_INFO;

    if (myOsVersionPackageRepo.isEmpty()) {
        QMetaObject::invokeMethod(this, "failure", Qt::QueuedConnection,
                                  Q_ARG(QString, "Invalid repo for OS version lookup"));
        return;
    }

    if (myOsVersionPackageId.isEmpty()) {
        QMetaObject::invokeMethod(this, "failure", Qt::QueuedConnection,
                                  Q_ARG(QString, "Invalid package for OS version lookup"));
        return;
    }

    if (myAction == Refresh) {
        refresh();
    } else {
        checkInstalledVersion();
    }
}

PackageKit::Transaction* DistCheckTransactionPkgKit::makeTransaction()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));

    return tx;
}

void DistCheckTransactionPkgKit::refresh()
{
    qDebug() << Q_FUNC_INFO;

    PackageKit::Transaction* tx = makeTransaction();
    tx->repoSetData(myOsVersionPackageRepo, "refresh-now", "true");
}

void DistCheckTransactionPkgKit::checkInstalledVersion()
{
    qDebug() << Q_FUNC_INFO;

    PackageKit::Transaction* tx = makeTransaction();

    connect(tx,
            SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotVersionPackageResolved(
                     PackageKit::Transaction::Info,QString,QString)));

    tx->resolve(myOsVersionPackageId,
                PackageKit::Transaction::FilterNotSource |
                PackageKit::Transaction::FilterInstalled);
}

void DistCheckTransactionPkgKit::checkForUpgrade()
{
    qDebug() << Q_FUNC_INFO;

    PackageKit::Transaction* tx = makeTransaction();

    connect(tx,
            SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotVersionPackageResolved(
                     PackageKit::Transaction::Info,QString,QString)));

    tx->resolve(myOsVersionPackageId,
                PackageKit::Transaction::FilterNewest |
                PackageKit::Transaction::FilterNotSource |
                PackageKit::Transaction::FilterNotInstalled);
}

void DistCheckTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << Q_FUNC_INFO << status << runtime;

    switch (myAction) {
    case Refresh:
        myAction = CheckInstalled;
        checkInstalledVersion();
        break;
    case CheckInstalled:
        myAction = CheckForUpgrade;
        checkForUpgrade();
        break;
    case CheckForUpgrade:
        if (myHaveUpgrade) {
            emit success();
        }
        emit finished();
        deleteLater();
        break;
    }
}

void DistCheckTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString &details)
{
    qDebug() << Q_FUNC_INFO << error << details;

    // TODO: details appear in the transfer UI and should therefore be processed
    // here to show some useful text to the user
    QString userDetails = details;

    switch (error) {
    case PackageKit::Transaction::ErrorPackageConflicts:
        qDebug() << "Package Conflicts" << details;
        break;
    default:
        break;
    }

    deleteLater();
    emit failure(userDetails);
    emit finished();
}

void DistCheckTransactionPkgKit::slotVersionPackageResolved(
        PackageKit::Transaction::Info info,
        const QString& packageId,
        const QString& summary)
{
    qDebug() << Q_FUNC_INFO << info << packageId << summary;

    const QString version = PackageKit::Transaction::packageVersion(packageId);

    myUpgradeInfo->version = version;
    myUpgradeInfo->summary = summary;

    switch (myAction) {
    case CheckInstalled:
        myInstalledVersion = version;
        break;
    case CheckForUpgrade:
        if (version != myInstalledVersion) {
            myHaveUpgrade = true;
        }
        break;
    default:
        break;
    }
}

}
