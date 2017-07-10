#include "packagekitmanager.h"
#include "distchecktransactionpkgkit.h"
#include "distupgradetransactionpkgkit.h"
#include "infotransactionrpm.h"
#include "installtransactionpkgkit.h"
#include "listtransactionpkgkit.h"
#include "osupdatesizetransactionpkgkit.h"
#include "packagenametransactionpkgkit.h"
#include "packagefilestransactionpkgkit.h"
#include "repositorytransactionpkgkit.h"
#include "uninstalltransactionpkgkit.h"

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDebug>
#include <QFileInfo>

// rpmlib
#include <rpm/rpmlib.h>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>
#include <PackageKit/packagekit-qt5/Daemon>
// for PackageKit D-Bus defines
#include <PackageKit/packagekit-qt5/common.h>

namespace
{

// the idle timeout in ms, after which PackageKit is considered idle
const int IDLE_TIMEOUT_MS = 30000;

QString packageNameFromRpm(const QString& fileName)
{
    const QString baseName = fileName.right(fileName.size()
                                            - fileName.lastIndexOf('/')
                                            - 1 /* don't include / */);
    const QRegExp re("-[0-9]+");
    const QStringList parts = baseName.split(re);
    qDebug() << fileName << baseName << parts;
    if (parts.size()) {
        return parts.at(0);
    } else {
        return baseName;
    }
}

}

namespace PackageManagement
{

class PackageKitManagerPrivate
{
public:
    QTimer myIdleTimer;
    QString myPackageInstallRepo;
    QString myOsVersionPackageRepo;
    QString myOsVersionPackageId;
    QString myUpgradeDistroId;
};

PackageKitManager::PackageKitManager()
    : d(new PackageKitManagerPrivate)
{
    // watch PackageKit transactions queue
    QDBusConnection::systemBus().connect(QString(),
                                         PK_PATH,
                                         PK_NAME,
                                         "TransactionListChanged",
                                         this,
                                         SLOT(slotTransactionListChanged(QStringList)));

    // setup a timer that gets triggered when the transactions queue becomes
    // empty, and gets halted when transactions get queued, in order to avoid
    // reacting on a temporarily empty queue while applications are actively
    // using it
    d->myIdleTimer.setInterval(IDLE_TIMEOUT_MS);
    d->myIdleTimer.setSingleShot(true);
    connect(&d->myIdleTimer, SIGNAL(timeout()),
            this, SIGNAL(packageManagerIdle()));
}

PackageKitManager::~PackageKitManager()
{
    qDebug() << "telling PackageKit daemon to quit";
    PackageKit::Daemon::global()->suggestDaemonQuit();

    delete d;
}

void PackageKitManager::setPackageInstallRepository(const QString &repoId)
{
    d->myPackageInstallRepo = repoId;
}

QString PackageKitManager::packageInstallRepository() const
{
    return d->myPackageInstallRepo;
}

void PackageKitManager::setUpgradeDistroId(const QString &distroId)
{
    d->myUpgradeDistroId = distroId;
}

QString PackageKitManager::upgradeDistroId() const
{
    return d->myUpgradeDistroId;
}

void PackageKitManager::setOsVersionPackage(const QString &packageRepo, const QString &packageId)
{
    d->myOsVersionPackageRepo = packageRepo;
    d->myOsVersionPackageId = packageId;
}

QString PackageKitManager::osVersionPackageRepository() const
{
    return d->myOsVersionPackageRepo;
}

QString PackageKitManager::osVersionPackageId() const
{
    return d->myOsVersionPackageId;
}

Transaction* PackageKitManager::checkForUpgrade(UpgradeInformation::Ptr info, bool refreshCache)
{
    return new DistCheckTransactionPkgKit(info, d->myOsVersionPackageRepo, d->myOsVersionPackageId, refreshCache);
}

Transaction* PackageKitManager::getOsUpdateSize(UpgradeInformation::Ptr info)
{
    return new OsUpdateSizeTransactionPkgKit(info, d->myUpgradeDistroId);
}

Transaction* PackageKitManager::upgrade(bool downloadOnly)
{
    return new DistUpgradeTransactionPkgKit(d->myUpgradeDistroId, downloadOnly);
}

Transaction* PackageKitManager::listPackages(Collection::Ptr collection)
{
    return new ListTransactionPkgKit(collection);
}

Transaction* PackageKitManager::installPackage(const QString& pkgId,
                                           PackageInformation::Ptr info)
{
    return new InstallTransactionPkgKit(d->myPackageInstallRepo, pkgId, info);
}

Transaction* PackageKitManager::uninstallPackage(const QString& pkgId,
                                             bool autoRemove)
{
    return new UninstallTransactionPkgKit(pkgId, autoRemove);
}

Transaction* PackageKitManager::packageInformation(const QString& filename,
                                               PackageInformation::Ptr info)
{
    return new InfoTransactionRpm(filename, info);
}

Transaction* PackageKitManager::packageFiles(const QString& pkgName,
                                         QStringList& files)
{
    return new PackageFilesTransactionPkgKit(pkgName, files);
}

Transaction *PackageKitManager::packageName(const QString &filePath,
                                        PackageInformation::Ptr info)
{
    return new PackageNameTransactionPkgKit(filePath, info);
}

Transaction* PackageKitManager::requireRepositories(Transaction* tx,
                                                const QString& version,
                                                bool restoreVersionAtSuccess,
                                                const QStringList& repos)
{
    return new RepositoryTransactionPkgKit(tx, version, restoreVersionAtSuccess, repos);
}

void PackageKitManager::slotTransactionListChanged(const QStringList& tids)
{
    // The interesting pieces are:
    //  - Is the transactions queue empty? Then noone is using PackageKit.
    //  - Does the queue have more than one transaction and is the last
    //    transaction not a repository enabling or disabling?
    //    Then someone (not store-client) wants to use PackageKit.
    if (tids.size()) {
        d->myIdleTimer.stop();
        PackageKit::Transaction tx(QDBusObjectPath(tids.last()));
        if (tids.size() > 1
                && tx.role() != PackageKit::Transaction::RoleRepoEnable) {
            emit packageManagerForeignRequested();
        }
    } else {
        d->myIdleTimer.start();
    }
}

int PackageKitManager::compareVersions(const QString& a, const QString& b) const
{
    // The format of version strings is [<epoch>:]<version>-<release>
    // - epoch is optional, and if not present zero ("0") is assumed
    // - first compare epoch, and if that's equal then version
    // - release is ignored by the comparison

    int aColon = a.indexOf(':');
    int bColon = b.indexOf(':');
    const QString aEpoch = aColon != -1 ? a.left(aColon) : "0";
    const QString bEpoch = bColon != -1 ? b.left(bColon) : "0";

    int result = rpmvercmp(aEpoch.toUtf8().constData(),
                           bEpoch.toUtf8().constData());

    if (!result) {
        const QString aVersion = a.mid(aColon + 1, a.lastIndexOf('-') - aColon - 1);
        const QString bVersion = b.mid(bColon + 1, b.lastIndexOf('-') - bColon - 1);

        result = rpmvercmp(aVersion.toUtf8().constData(),
                           bVersion.toUtf8().constData());
    }
    return result;
}

QStringList PackageKitManager::desktopFiles(const QString& pkgName) const
{
    // package name could point to a local package, including the full path,
    // so strip it
    if (pkgName.startsWith("/")) {
        return PackageKit::Transaction::packageDesktopFiles(
                    packageNameFromRpm(pkgName));
    } else {
        return PackageKit::Transaction::packageDesktopFiles(pkgName);
    }
}

QString PackageKitManager::packageNameByDesktopFile(const QString& desktopFile) const
{
    return PackageKit::Transaction::packageNameByDesktopFile(desktopFile);
}

}
