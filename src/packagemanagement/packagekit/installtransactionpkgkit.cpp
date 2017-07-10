#include "installtransactionpkgkit.h"

#include <QFile>
#include <QTimer>
#include <QDebug>

namespace
{
    // maximum number of retries, retrying the same action again
    const int MAX_RETRIES = 3;

    // delay between retries in milliseconds
    const int RETRY_DELAY=10000;
}

namespace PackageManagement
{

InstallTransactionPkgKit::InstallTransactionPkgKit(const QString &repoId,
                                                   const QString& pkgId,
                                                   Manager::PackageInformation::Ptr info)
    : myState(Idle)
    , myRepoId(repoId)
    , myPkgId(pkgId)
    , myRefreshLevel(RefreshNone)
    , myIsLocalFile(false)
    , myIsUpdate(false)
    , myCurrentTransaction(0)
    , myFailureHandled(false)
    , myProgress(-1)
    , myRetryCounter(0)
    , myPackageInfo(info)
{
    myPackageInfo->name.clear();
    myPackageInfo->version.clear();
    myPackageInfo->summary.clear();
    myPackageInfo->file.clear();
}

InstallTransactionPkgKit::~InstallTransactionPkgKit()
{
    qDebug() << "deleting InstallTransactionPkgKit";
}

void InstallTransactionPkgKit::start()
{
    if (myPkgId.endsWith(".rpm") && QFile(myPkgId).exists()) {
        // we got a local package path
        myIsLocalFile = true;
        installFile(myPkgId);
    } else {
        // we probably got a package name
        resolveInstalled(myPkgId);
    }
}

void InstallTransactionPkgKit::refresh()
{
    qDebug() << Q_FUNC_INFO;

    if (myRepoId.isEmpty()) {
        QMetaObject::invokeMethod(this, "failure", Qt::QueuedConnection,
                                  Q_ARG(QString, "Invalid repo for package installation"));
        return;
    }

    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);

    myCurrentTransaction = tx;
    myState = Refreshing;

    switch (myRefreshLevel) {
    case RefreshRepo:
        qDebug() << "refreshing installation repository";
        tx->repoSetData(myRepoId, "refresh-now", "false");
        break;

    case RefreshRepoForced:
        qDebug() << "force-refreshing installation repository";
        tx->repoSetData(myRepoId, "refresh-now", "true");
        break;

    case RefreshAll:
        // this is our last resort and it may take a longer time to refresh
        qDebug() << "refreshing all repositories";
        tx->refreshCache(true);
        break;

    default:
        // should never occur to the user; if we got here, we probably forgot
        // to implement a case during development
        qWarning() << "Don't know how to refresh for refresh level"
                   << myRefreshLevel;
        break;
    }
}

void InstallTransactionPkgKit::resolveInstalled(const QString& pkgId)
{
    qDebug() << Q_FUNC_INFO << pkgId;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotResolvedInstalled(PackageKit::Transaction::Info,QString,QString)));

    myState = ResolvingInstalled;
    myInstalledPkgVersion.clear();
    myCurrentTransaction = tx;
    tx->resolve(pkgId,
                PackageKit::Transaction::FilterNotSource |
                PackageKit::Transaction::FilterNewest |
                PackageKit::Transaction::FilterNotDevel |
                PackageKit::Transaction::FilterInstalled);
}

void InstallTransactionPkgKit::resolve(const QString& pkgId)
{
    qDebug() << Q_FUNC_INFO << pkgId;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotResolved(PackageKit::Transaction::Info,QString,QString)));

    myState = Resolving;
    myResolvedPkgId.clear();
    myCurrentTransaction = tx;
    tx->resolve(pkgId,
                PackageKit::Transaction::FilterNotSource |
                PackageKit::Transaction::FilterNewest |
                PackageKit::Transaction::FilterNotDevel);
}

void InstallTransactionPkgKit::resolveDependencies(const QString& pkg)
{
    qDebug() << Q_FUNC_INFO << pkg;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this,
            SLOT(slotResolvedDependency(PackageKit::Transaction::Info,
                                        QString,QString)));

    myState = ResolvingDeps;
    myCurrentTransaction = tx;
    myDependencies.clear();
    myDependencies << pkg;
    tx->getDepends(pkg,
                   PackageKit::Transaction::FilterNotSource |
                   PackageKit::Transaction::FilterNewest |
                   PackageKit::Transaction::FilterNotDevel,
                   true /* recursive */);
}

void InstallTransactionPkgKit::download(const QStringList& packages)
{
    qDebug() << Q_FUNC_INFO << packages;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(changed()),
            this, SLOT(slotInstallProgress()));

    myState = Downloading;
    myCurrentTransaction = tx;
    tx->installPackages(packages,
                        PackageKit::Transaction::TransactionFlagOnlyDownload |
                        PackageKit::Transaction::TransactionFlagOnlyTrusted);
}

void InstallTransactionPkgKit::install(const QString& pkg)
{
    qDebug() << Q_FUNC_INFO << pkg;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(changed()),
            this, SLOT(slotInstallProgress()));

    myState = Installing;
    myCurrentTransaction = tx;
    tx->installPackage(pkg);
}

void InstallTransactionPkgKit::installFile(const QString& pkgFile)
{
    qDebug() << Q_FUNC_INFO << pkgFile;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(changed()),
            this, SLOT(slotInstallProgress()));

    myState = Installing;
    myCurrentTransaction = tx;
    tx->installFile(pkgFile);
}

void InstallTransactionPkgKit::update(const QString& pkg)
{
    qDebug() << Q_FUNC_INFO << pkg;
    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    prepareTransaction(tx);
    connect(tx, SIGNAL(changed()),
            this, SLOT(slotInstallProgress()));

    myState = Installing;
    myCurrentTransaction = tx;
    tx->updatePackage(pkg);
}

void InstallTransactionPkgKit::prepareTransaction(PackageKit::Transaction* tx)
{
    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotTransactionFinished(PackageKit::Transaction::Exit,uint)));

    connect(tx, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this,
            SLOT(slotTransactionError(PackageKit::Transaction::Error,QString)));
}

void InstallTransactionPkgKit::giveUpOrRetry(const QString& details)
{
    qDebug() << Q_FUNC_INFO << details;

    delete myCurrentTransaction;
    myCurrentTransaction = 0;

    // when installing a local file, or if the package is already installed,
    // there's no point to retry actions, so just give up
    if (myIsLocalFile) {
        myFailureHandled = true;
        deleteLater();
        emit failure(details);
        emit finished();
        return;
    }

    // We don't usually refresh, because it is time-consuming. So it's quite
    // natural that we run into errors, after which we try again but with
    // refreshing.
    // If we still run into errors, we retry a number of times (but don't need
    // to refresh again, provided that refreshing was successful) to cope with
    // shaky network connections.

    if (myRefreshLevel < RefreshAll) {
        ++myRefreshLevel;
        // if we haven't refreshed the package cache yet and encounter some
        // error, we refresh and try again
        qDebug() << "refresh and retry";
        myResolvedPkgId.clear();
        myLatestPkgVersion.clear();
        myInstalledPkgVersion.clear();
        refresh();
    } else if (myRetryCounter < MAX_RETRIES) {
        ++myRetryCounter;
        qDebug() << "retry" << myRetryCounter << "/" << MAX_RETRIES
                 << "in" << RETRY_DELAY << "ms";
        QTimer::singleShot(RETRY_DELAY, this, SLOT(slotRetrySameAction()));
    } else {
        qDebug() << "giving up";
        myFailureHandled = true;
        deleteLater();
        emit failure(details);
        emit finished();
    }
}

void InstallTransactionPkgKit::slotRetrySameAction()
{
    switch (myState) {
    case Refreshing:
        refresh();
        break;
    case ResolvingInstalled:
        resolveInstalled(myPkgId);
        break;
    case Resolving:
        resolve(myPkgId);
        break;
    case ResolvingDeps:
        resolveDependencies(myResolvedPkgId);
        break;
    case Downloading:
        download(myDependencies);
        break;
    case Installing:
        if (myIsUpdate) {
            update(myResolvedPkgId);
        } else {
            install(myResolvedPkgId);
        }
        break;
    default:
        break;
   }
}

void InstallTransactionPkgKit::slotTransactionFinished(
        PackageKit::Transaction::Exit status, uint runtime)
{
    qDebug() << Q_FUNC_INFO
             << status << runtime;

    if (status == PackageKit::Transaction::ExitSuccess) {
        switch (myState) {
        case Refreshing:
            resolveInstalled(myPkgId);
            break;

        case ResolvingInstalled:
            resolve(myPkgId);
            break;

        case Resolving:
            if (!myResolvedPkgId.isEmpty()) {
                if (!myInstalledPkgVersion.isEmpty()) {
                    if (myInstalledPkgVersion == myLatestPkgVersion) {
                        giveUpOrRetry("package already installed " + myPkgId);
                        return;
                    } else {
                        myIsUpdate = true;
                    }
                }
                resolveDependencies(myResolvedPkgId);
            } else {
                giveUpOrRetry("could not find package " + myPkgId);
            }
            break;

        case ResolvingDeps:
            if (!myDependencies.isEmpty()) {
                download(myDependencies);
            } else if (myIsUpdate) {
                update(myResolvedPkgId);
            } else {
                install(myResolvedPkgId);
            }

            break;

        case Downloading:
            if (myIsUpdate) {
                update(myResolvedPkgId);
            } else {
                install(myResolvedPkgId);
            }
            break;

        case Installing:
            myCurrentTransaction = 0;
            deleteLater();

            emit success();
            emit finished();
            break;

        default:
            break;
        }
    } else if (!myFailureHandled) {
        qDebug() << "transaction failure" << myCurrentTransaction->internalErrorMessage();
        giveUpOrRetry(QString("unknown failure with exit status %1")
               .arg(status));
    }
}

void InstallTransactionPkgKit::slotTransactionError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << Q_FUNC_INFO
             << error << details;

    giveUpOrRetry(details);
}

void InstallTransactionPkgKit::slotResolvedInstalled(PackageKit::Transaction::Info info,
                                                     const QString& pkgId,
                                                     const QString& summary)
{
    Q_UNUSED(info);
    Q_UNUSED(summary);

    qDebug() << Q_FUNC_INFO << pkgId << PackageKit::Transaction::packageVersion(pkgId);

    if (myInstalledPkgVersion.isEmpty()) {
        // get the version of the installed package
        myInstalledPkgVersion = PackageKit::Transaction::packageVersion(pkgId);
    } else {
        qDebug() << "Weird I resolved several packages, even though I tried "
                    "to filter.";
    }
}

void InstallTransactionPkgKit::slotResolved(PackageKit::Transaction::Info info,
                                            const QString& pkgId,
                                            const QString& summary)
{
    qDebug() << Q_FUNC_INFO << info
             << pkgId << summary;

    if (myResolvedPkgId.isEmpty()) {
        myResolvedPkgId = pkgId;
        myLatestPkgVersion = PackageKit::Transaction::packageVersion(pkgId);

        myPackageInfo->name = PackageKit::Transaction::packageName(pkgId);
        myPackageInfo->version = myLatestPkgVersion;
        myPackageInfo->summary = summary;
    } else {
        qDebug() << "Weird, I resolved several packages, even though I tried "
                    "to filter.";
    }
}

void InstallTransactionPkgKit::slotResolvedDependency(
        PackageKit::Transaction::Info info, const QString& pkgId,
        const QString& summary)
{
    qDebug() << Q_FUNC_INFO << info
             << pkgId << summary;

    if (info != PackageKit::Transaction::InfoInstalled
            && PackageKit::Transaction::packageName(pkgId) != PackageKit::Transaction::packageName(myResolvedPkgId)) {
        myDependencies << pkgId;
    }
}

void InstallTransactionPkgKit::slotInstallProgress()
{
    if (myCurrentTransaction) {
        if (myCurrentTransaction->percentage() != 101) {
            int value = -1;

            if (myState == Downloading) {
                // range 1 ... 49 (excluding 0, waiting state)
                value = 1 + static_cast<int>(48 * myCurrentTransaction->percentage() / 100.0);
            } else if (myState == Installing) {
                // range 50 ... 100
                value = 50 + static_cast<int>(50 * myCurrentTransaction->percentage() / 100.0);
            }

            if (value > myProgress) {
                myProgress = value;
                emit progress(value);
            }
        } else {
            emit progress(myProgress);
        }
    }
}

}
