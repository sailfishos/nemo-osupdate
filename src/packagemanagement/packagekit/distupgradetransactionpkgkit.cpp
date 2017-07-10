#include "distupgradetransactionpkgkit.h"

#include <QDebug>

namespace
{

// max number of refresh/download cycle retries
const int MAX_RETRIES = 3;
}

namespace PackageManagement
{

DistUpgradeTransactionPkgKit::DistUpgradeTransactionPkgKit(const QString &upgradeDistroId, bool downloadOnly)
    : myAction(downloadOnly ? Download : Upgrade)
    , myProgress(-1)
    , myRetryCounter(0)
    , myUpgradeDistroId(upgradeDistroId)
    , myCurrentTransaction(0)
{

}

DistUpgradeTransactionPkgKit::~DistUpgradeTransactionPkgKit()
{
    qDebug() << Q_FUNC_INFO;
}

void DistUpgradeTransactionPkgKit::start()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Upgrade distro ID:" << myUpgradeDistroId;

    if (myUpgradeDistroId.isEmpty()) {
        QMetaObject::invokeMethod(this, "failure", Qt::QueuedConnection,
                                  Q_ARG(QString, "Invalid distro id for OS upgrade"));
        return;
    }

    if (myAction == Download) {
        download();
    } else {
        upgrade();
    }
}

void DistUpgradeTransactionPkgKit::cancel()
{
    qDebug() << Q_FUNC_INFO;
    if (myCurrentTransaction) {
        myCurrentTransaction->cancel();
    } else {
        emit failure("cancelled");
        emit finished();
        deleteLater();
    }
}

PackageKit::Transaction* DistUpgradeTransactionPkgKit::makeTransaction()
{
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));

    connect(tx, SIGNAL(changed()),
            this, SLOT(slotUpgradeProgress()));

    return tx;
}

void DistUpgradeTransactionPkgKit::download()
{
    qDebug() << Q_FUNC_INFO;

    myCurrentTransaction = makeTransaction();

    connect(myCurrentTransaction,
            SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotPackage(PackageKit::Transaction::Info,QString,QString)));

    myCurrentTransaction->upgradeSystem(myUpgradeDistroId,
                                        PackageKit::Transaction::UpgradeKindMinimal);
}

void DistUpgradeTransactionPkgKit::upgrade()
{
    qDebug() << Q_FUNC_INFO;

    myCurrentTransaction = makeTransaction();

    connect(myCurrentTransaction,
            SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotPackage(PackageKit::Transaction::Info,QString,QString)));

    myCurrentTransaction->upgradeSystem(myUpgradeDistroId,
                                        PackageKit::Transaction::UpgradeKindComplete);
    myProgress = -1;
    emit progress(-1);
}

void DistUpgradeTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << Q_FUNC_INFO << status << runtime;
    myCurrentTransaction = 0;

    if (status == PackageKit::Transaction::ExitSuccess) {
        switch (myAction) {
        case Download:
            emit success();
            emit finished();
            deleteLater();
            break;
        case Upgrade:
            emit success();
            emit finished();
            deleteLater();
            break;
        }
    } else if (status == PackageKit::Transaction::ExitCancelled) {
        emit failure("cancelled");
        emit finished();
        deleteLater();
    } else {
        if (myAction == Download && myRetryCounter < MAX_RETRIES) {
            myRetryCounter++;
            download();
        } else {
            // "myErrorDetails" might be an empty string, if no "errorCode"
            // signal was received before "finished", but that's fine
            // because it's not used for anything currently.
            emit failure(myErrorDetails);
            emit finished();
            deleteLater();
        }
    }
    myErrorDetails.clear();
}

void DistUpgradeTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString &details)
{
    qDebug() << Q_FUNC_INFO << error << details;

    switch (error) {

    case PackageKit::Transaction::ErrorNoSpaceOnDevice:
        myErrorDetails = "disk full: ";
        myErrorDetails.append(details);
        break;

    default:
        myErrorDetails = details;
        break;
    }
}
void DistUpgradeTransactionPkgKit::slotPackage(
        PackageKit::Transaction::Info info,
        const QString& packageId,
        const QString& summary)
{
    // This slot is here solely for debugging purposes. We want to show all the
    // downloaded/installed packages in the logs.
    qDebug() << Q_FUNC_INFO << info << packageId << summary;
}

void DistUpgradeTransactionPkgKit::slotUpgradeProgress()
{
    PackageKit::Transaction* tx = qobject_cast<PackageKit::Transaction*>(sender());
    if (tx) {
        int value = -1;
        // never propagate progress 100 here, as we can't be sure that
        // PackageKit doesn't lie to us
        int percentage = tx->percentage();
        if (percentage < 100) {
            switch (tx->status()) {
            case PackageKit::Transaction::StatusRefreshCache:
                value = percentage / 10;
                break;
            case PackageKit::Transaction::StatusDownload:
                value = 10 + 9 * percentage / 10;
                break;
            case PackageKit::Transaction::StatusInstall:
                value = percentage;
                break;
            default:
                break;
            }

            // Never report zero progress
            value = qMax(1, value);
        }
        if (value > myProgress) {
            myProgress = value;
            emit progress(value);
        }
    }
}

}
