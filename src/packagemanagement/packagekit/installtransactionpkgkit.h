#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_INSTALLTRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_INSTALLTRANSACTIONPKGKIT_H

#include "manager.h"
#include "transaction.h"

#include <QString>
#include <QStringList>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

/* Install transaction implementation for PackageKit backend.
 */
class InstallTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    /* Creates a new install transaction. pkgId may either be a package name
     * from the repositories, or the path to a local package file.
     * The supplied package information object will be loaded with information
     * about the installed or updated package.
     */
    explicit InstallTransactionPkgKit(const QString &repoId,
                                      const QString& pkgId,
                                      Manager::PackageInformation::Ptr info);
    virtual ~InstallTransactionPkgKit();

    virtual void start();

private:
    enum TransactionState
    {
        Idle, Refreshing, Resolving, ResolvingInstalled, ResolvingDeps, Downloading, Installing
    };

    // there are so many ways to refresh the package cache, and we try them one
    // by one until it works (or fails eventually)
    enum RefreshLevel
    {
        RefreshNone,
        RefreshRepo,
        RefreshRepoForced,
        RefreshAll
    };

    void refresh();
    void resolveInstalled(const QString& pkgId);
    void resolve(const QString& pkgId);
    void resolveDependencies(const QString& pkg);
    void download(const QStringList& packages);
    void install(const QString& pkg);
    void installFile(const QString& pkgFile);
    void update(const QString& pkg);

    void prepareTransaction(PackageKit::Transaction* tx);
    void giveUpOrRetry(const QString& details);

private slots:
    void slotTransactionFinished(PackageKit::Transaction::Exit status,
                                 uint runtime);
    void slotTransactionError(PackageKit::Transaction::Error error,
                              const QString& details);

    void slotResolvedInstalled(PackageKit::Transaction::Info info,
                               const QString& pkgId,
                               const QString& summary);

    void slotResolved(PackageKit::Transaction::Info info,
                      const QString& pkgId,
                      const QString& summary);

    void slotResolvedDependency(PackageKit::Transaction::Info info,
                                const QString& pkgId,
                                const QString& summary);

    void slotInstallProgress();
    void slotRetrySameAction();

private:
    TransactionState myState;

    QString myRepoId;
    QString myPkgId;
    QString myResolvedPkgId;
    QString myInstalledPkgVersion;
    QString myLatestPkgVersion;
    QStringList myDependencies;
    int myRefreshLevel;
    bool myIsLocalFile;
    bool myIsUpdate;
    PackageKit::Transaction* myCurrentTransaction;
    bool myFailureHandled;
    int myProgress;
    int myRetryCounter;
    Manager::PackageInformation::Ptr myPackageInfo;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_INSTALLTRANSACTIONPKGKIT_H
