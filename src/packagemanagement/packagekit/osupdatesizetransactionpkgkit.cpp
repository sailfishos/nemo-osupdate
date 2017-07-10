#include "osupdatesizetransactionpkgkit.h"

#include <QDebug>
#include <cstdio>

namespace
{

bool parseSizeInfo(const QString &sizeInfo,
                   qlonglong *downloadSize,
                   qlonglong *installSize,
                   qlonglong *removeSize)
{
    // the format is like "DOWNLOAD=1;INSTALL=8552975;REMOVE=0;CACHED=0"

    QMap<QString, qlonglong> data;
    foreach (const QString& part, sizeInfo.split(';')) {
        QStringList keyValue = part.split('=');
        if (keyValue.size() != 2) {
            return false;
        }
        bool ok = true;
        data[keyValue.first()] = keyValue.last().toLongLong(&ok);
        if (!ok) {
            return false;
        }
    }

    if (!data.contains("DOWNLOAD") ||
            !data.contains("INSTALL") ||
            !data.contains("REMOVE") ||
            !data.contains("CACHED")) {
        return false;
    }

    // cached data is already downloaded and will not be taken into account
    // anymore
    *downloadSize = data["DOWNLOAD"];
    *installSize = data["INSTALL"];
    *removeSize = data["REMOVE"];
    return true;
}

}

namespace PackageManagement
{

OsUpdateSizeTransactionPkgKit::OsUpdateSizeTransactionPkgKit(Manager::UpgradeInformation::Ptr info, const QString &upgradeDistroId)
    : myUpgradeInfo(info)
    , myState(Calculate)
    , myUpgradeDistroId(upgradeDistroId)
{
}

OsUpdateSizeTransactionPkgKit::~OsUpdateSizeTransactionPkgKit()
{
}

void OsUpdateSizeTransactionPkgKit::start()
{
    // PackageKit will do an implicit refresh into the right cache, so we
    // must not refresh explicitly, which would end up in the wrong cache
    calculate();
}

void OsUpdateSizeTransactionPkgKit::calculate()
{
    if (myUpgradeDistroId.isEmpty()) {
        QMetaObject::invokeMethod(this, "failure", Qt::QueuedConnection,
                                  Q_ARG(QString, "Invalid distro id for OS upgrade"));
        return;
    }

    PackageKit::Transaction* tx = new PackageKit::Transaction(this);

    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    // this transaction will "fail" (because the data gets passed via an
    // error code)
    connect(tx,
            SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));

    tx->upgradeSystem(QString("nemo::query-size:%1")
                      .arg(myUpgradeDistroId),
                      PackageKit::Transaction::UpgradeKindComplete);
}

qlonglong OsUpdateSizeTransactionPkgKit::parseNeededSpace(const QString &details)
{
    // "Not enough space for installation. Need %.2f MiB, have %.2f MiB.\n"
    // "Not enough space for download. Need %.2f MiB, have %.2f MiB.\n",
    // TODO : Get rid of this parsing. See also JB#36497

    QByteArray bytes = details.toLocal8Bit();
    const char *chars = bytes.constData();
    // As format gives always megabytes integers are precise enough. Just round to next MB.
    int neededSpace = 0;
    sscanf(chars,"%*s %*s %*s %*s %*s %*s %d", &neededSpace);
    return qlonglong((neededSpace + 1) * 1024 * 1024);
}

void OsUpdateSizeTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error,
        const QString& details)
{
    if (error == PackageKit::Transaction::ErrorNoDistroUpgradeData) {
        // this is not an error, but the way the OS update size information
        // is passed from the backend to the client
        if (parseSizeInfo(details,
                          &myUpgradeInfo->bytesToDownload,
                          &myUpgradeInfo->bytesToInstall,
                          &myUpgradeInfo->bytesToRemove)) {
            qDebug() << "OS update size:"
                     << (myUpgradeInfo->bytesToDownload / (1024 * 1024)) << "megabytes to download,"
                     << (myUpgradeInfo->bytesToInstall / (1024 * 1024)) << "megabytes to install,"
                     << (myUpgradeInfo->bytesToRemove / (1024 * 1024)) << "megabytes to remove";
            myState = Success;
        } else {
            qDebug() << "invalid OS update size information:" << details;
            myErrorDetails = details;
        }
    } else if (error == PackageKit::Transaction::ErrorNoSpaceOnDevice) {
        if (details.contains("download")) {
            myUpgradeInfo->bytesToDownload = parseNeededSpace(details);
        } else if (details.contains("installation")) {
            myUpgradeInfo->bytesToInstall = parseNeededSpace(details);

        }
        myErrorDetails = details;
    } else {
        myErrorDetails = details;
    }
}

void OsUpdateSizeTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit,
        uint)
{
    if (myState == Success) {
        emit success();
    } else {
        emit failure(myErrorDetails);
    }

    emit finished();
    deleteLater();
}

}
