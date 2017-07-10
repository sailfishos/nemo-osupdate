#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEMANAGER_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEMANAGER_H

#include "nemo-osupdate/global.h"
#include "collection.h"

#include <QObject>

namespace PackageManagement
{

class Transaction;
class ManagerPrivate;

/* Abstract base class for package manager implementations.
 */
class NEMO_OSUPDATE_EXPORT Manager : public QObject
{
    Q_OBJECT
public:
    struct PackageInformation
    {
        typedef QSharedPointer<PackageInformation> Ptr;

        QString name;
        QString version;
        QString summary;
        QString file;
    };

    struct UpgradeInformation
    {
        typedef QSharedPointer<UpgradeInformation> Ptr;

        UpgradeInformation();

        QString version;
        QString summary;
        qlonglong bytesToDownload;
        qlonglong bytesToInstall;
        qlonglong bytesToRemove;
    };

    explicit Manager(QObject *parent = nullptr);
    ~Manager();

    // Checks for a dist upgrade. Returns a transaction pointer.
    // The transaction emits success if an upgrade was found.
    virtual Transaction* checkForUpgrade(UpgradeInformation::Ptr info, bool refreshCache) = 0;

    // Checks the sizes required by the OS update and fills the corresponding
    // fields in the UpgradeInformation struct.
    virtual Transaction* getOsUpdateSize(UpgradeInformation::Ptr info) = 0;

    // Performs a dist upgrade if an upgrade is available.
    virtual Transaction* upgrade(bool downloadOnly) = 0;

    // Lists the installed packages asynchronously. Returns a transaction pointer.
    virtual Transaction* listPackages(Collection::Ptr collection) = 0;

    // Installs the package with the given unique ID.
    virtual Transaction* installPackage(const QString& pkgId,
                                        PackageInformation::Ptr info) = 0;

    // Uninstalls the package with the given unique ID.
    virtual Transaction* uninstallPackage(const QString& pkgId, bool autoRemove) = 0;

    // Retrieves information about a given package file.
    virtual Transaction* packageInformation(const QString& filename,
                                            PackageInformation::Ptr info) = 0;

    // Retrieves files contained in the given package.
    virtual Transaction* packageFiles(const QString& pkgName,
                                      QStringList& files) = 0;

    // Retrieves information about a package that includes filePath.
    virtual Transaction* packageName(const QString& filePath,
                                     PackageInformation::Ptr info) = 0;

    // Wraps the given transaction and makes sure that the given repositories
    // are the only ones enabled, and are present.
    // After the transaction finished, the defaults will be restored.
    // The version is only restored afterwards if restoreVersionAtSuccess is
    // true, or the wrapped transaction failed.
    // If not all given repositories are available, a failure will be emitted
    // and the wrapped transaction will not be started.
    virtual Transaction* requireRepositories(Transaction* tx,
                                             const QString& version,
                                             bool restoreVersionAtSuccess,
                                             const QStringList& repos) = 0;

    // Synchronous actions that return immediately.
    //

    // Compares two version strings. Returns -1 if the first is older, 0 if they
    // are equal, and 1 if the first is newer.
    virtual int compareVersions(const QString& a, const QString& b) const = 0;

    // Retrieves .desktop files contained in the given package.
    virtual QStringList desktopFiles(const QString& pkgName) const = 0;

    // Retrieves the package name containing the given .desktop file.
    virtual QString packageNameByDesktopFile(const QString& desktopFile) const = 0;

signals:
    // Gets emitted when the package manager was detected to be idle.
    void packageManagerIdle();
    // Gets emitted when the package manager was requested by a foreign application.
    void packageManagerForeignRequested();

private:
    ManagerPrivate *d;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_PACKAGEMANAGER_H
