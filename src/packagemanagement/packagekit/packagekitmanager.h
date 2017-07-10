#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEKITMANAGER_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEKITMANAGER_H

#include "nemo-osupdate/global.h"
#include "manager.h"
#include "transaction.h"

namespace PackageManagement
{

class PackageKitManagerPrivate;

/* PackageKit implementation of a package manager.
 */
class NEMO_OSUPDATE_EXPORT PackageKitManager : public Manager
{
    Q_OBJECT
public:
    explicit PackageKitManager();
    virtual ~PackageKitManager();

    // Sets the repo to use when fetching packages for installation.
    void setPackageInstallRepository(const QString &repoId);
    QString packageInstallRepository() const;

    // Sets the id of the distro to download/install for system upgrades.
    void setUpgradeDistroId(const QString &distroId);
    QString upgradeDistroId() const;

    // Sets the package that contains the current OS version info, and the repo that contains
    // this package.
    void setOsVersionPackage(const QString &packageRepoId, const QString &packageId);
    QString osVersionPackageRepository() const;
    QString osVersionPackageId() const;

    virtual Transaction* checkForUpgrade(UpgradeInformation::Ptr info, bool refreshCache);
    virtual Transaction* getOsUpdateSize(UpgradeInformation::Ptr info);
    virtual Transaction* upgrade(bool downloadOnly);
    virtual Transaction* listPackages(Collection::Ptr collection);
    virtual Transaction* installPackage(const QString& pkgId,
                                        PackageInformation::Ptr info);
    virtual Transaction* uninstallPackage(const QString& pkgId, bool autoRemove);
    virtual Transaction* packageInformation(const QString &filename,
                                            PackageInformation::Ptr info);
    virtual Transaction* packageFiles(const QString &pkgName,
                                      QStringList& files);
    virtual Transaction* packageName(const QString& filePath,
                                     PackageInformation::Ptr info);

    virtual Transaction* requireRepositories(Transaction* tx,
                                             const QString& version,
                                             bool restoreVersionAtSuccess,
                                             const QStringList& repos);

    virtual int compareVersions(const QString& a, const QString& b) const;

    virtual QStringList desktopFiles(const QString& pkgName) const;

    virtual QString packageNameByDesktopFile(const QString& desktopFile) const;

private slots:
    void slotTransactionListChanged(const QStringList& tids);

private:
    PackageKitManagerPrivate *d;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEKITMANAGER_H
