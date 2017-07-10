#include "repositorytransactionpkgkit.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDebug>

// libssu
#include <ssu.h>

namespace PackageManagement
{

RepositoryTransactionPkgKit::RepositoryTransactionPkgKit(Transaction* tx,
                                                         const QString& version,
                                                         bool restoreVersionAtSuccess,
                                                         const QStringList& repos)
    : myWrappedTransaction(tx)
    , myWrappedTransactionFailed(false)
    , myVersion(version)
    , myRestoreVersionAtSuccess(restoreVersionAtSuccess)
    , myRequiredRepositories(repos)
{
    if (tx) {
        connect(tx, SIGNAL(finished()),
                this, SLOT(slotWrappedTransactionFinished()));
        connect(tx, SIGNAL(success()),
                this, SIGNAL(success()));
        connect(tx, SIGNAL(failure(QString)),
                this, SLOT(slotWrappedTransactionFailure(QString)));
    }
}

void RepositoryTransactionPkgKit::start()
{
    if (!myVersion.isEmpty() && !(Ssu().deviceMode() & Ssu::RndMode)) {
        // Set new SSU version if in release mode
        myOriginalSsuVersion = Ssu().release(false);
        setSsuVersion(myVersion);
    }

    PackageKit::Transaction* tx = new PackageKit::Transaction(this);
    connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this,
            SLOT(slotListReposFinished(PackageKit::Transaction::Exit,uint)));
    connect(tx, SIGNAL(repoDetail(QString,QString,bool)),
            this, SLOT(slotGotRepository(QString,QString,bool)));
    tx->getRepoList();
}

void RepositoryTransactionPkgKit::cancel()
{
    if (myWrappedTransaction) {
        myWrappedTransaction->cancel();
    }
}

void RepositoryTransactionPkgKit::verifyRequired()
{
    QStringList missingRepos;

    foreach (const QString& repoId, myRequiredRepositories) {
        if (!myDefaults.value(repoId, false)) {
            // Required repo either missing or disabled
            missingRepos << repoId;
        }
    }

    if (missingRepos.size()) {
        // force required repos to be enabled
        qDebug() << "enabling missing repos" << missingRepos;
        QDBusInterface ssuInterface("org.nemo.ssu",
                                    "/org/nemo/ssu",
                                    "org.nemo.ssu",
                                    QDBusConnection::systemBus());
        foreach (const QString repoId, missingRepos) {
            ssuInterface.call("modifyRepo", 1, repoId); // add
            ssuInterface.call("modifyRepo", 3, repoId); // enable
        }
        ssuInterface.call("updateRepos");
    }

    myRepositoriesToProcess = extraRepos();
    isolateNext();
}

void RepositoryTransactionPkgKit::isolateNext()
{
    if (myRepositoriesToProcess.size()) {
        myCurrentlyProcessedRepository = myRepositoriesToProcess.takeFirst();

        bool isEnabled = myDefaults.value(myCurrentlyProcessedRepository, false);
        bool toEnable = myRequiredRepositories.contains(myCurrentlyProcessedRepository);

        qDebug() << (toEnable
                     ? "enabling"
                     : "disabling")
                 << myCurrentlyProcessedRepository;

        if (isEnabled != toEnable) {
            PackageKit::Transaction* tx = new PackageKit::Transaction(this);
            connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                    this,
                    SLOT(slotIsolateRepositoryFinished(PackageKit::Transaction::Exit,uint)));
            connect(tx,
                    SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                    this,
                    SLOT(slotRepositoryModificationError(PackageKit::Transaction::Error,QString)));
            tx->repoEnable(myCurrentlyProcessedRepository, toEnable);
        } else {
            isolateNext(); // just a tail recursion
        }
    } else {
        // successfully finished isolating
        runTransaction();
    }
}

void RepositoryTransactionPkgKit::restoreNext(bool atomic)
{
    if (myRepositoriesToProcess.size()) {
        myCurrentlyProcessedRepository = myRepositoriesToProcess.takeFirst();

        bool isEnabled = myRequiredRepositories.contains(myCurrentlyProcessedRepository);
        bool wasEnabled = myDefaults.value(myCurrentlyProcessedRepository, false);

        qDebug() << (wasEnabled
                     ? "enabling"
                     : "disabling")
                 << myCurrentlyProcessedRepository;

        if (wasEnabled != isEnabled) {
            PackageKit::Transaction* tx = new PackageKit::Transaction(this);
            if (!atomic) {
                connect(tx, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                        this,
                        SLOT(slotRestoreRepositoryFinished(PackageKit::Transaction::Exit,uint)));
                connect(tx,
                        SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                        this,
                        SLOT(slotRepositoryModificationError(PackageKit::Transaction::Error,QString)));
            }
            tx->repoEnable(myCurrentlyProcessedRepository, wasEnabled);

            if (atomic) {
                restoreNext(true);
            }
        } else {
            restoreNext(false); // just a tail recursion
        }
    } else {
        // finished restoring the repositories

        // Note that it is in theory quite possible that some repositories could
        // not be restored. However, there's nothing that can be done about that
        // at this point, and we will either fail during the next isolating, or
        // not, in which case all is well again.

        if (!atomic) {
            restoreSsuVersion();
            deleteLater();
            emit success();
            emit finished();
        }
    }
}

void RepositoryTransactionPkgKit::runTransaction()
{
    if (myWrappedTransaction) {
        myWrappedTransaction->start();

        if (myWrappedTransaction->isAtomic()) {
            // we may take advantage of the atomic nature of the transaction
            // by immediately queuing the restoring of repos after the
            // transaction, so that no foreign transaction may slip inbetween
            // (and may find some repositories unexpectedly disabled)
            myRepositoriesToProcess = extraRepos();
            restoreNext(true);
        }
    } else {
        slotWrappedTransactionFinished();
    }
}

void RepositoryTransactionPkgKit::giveUp(const QStringList& failedRepositories)
{
    if (myWrappedTransaction) {
        myWrappedTransaction->cancel();
    }
    restoreSsuVersion();
    deleteLater();
    emit failure(failedRepositories.join(" "));
    emit finished();
}

QStringList RepositoryTransactionPkgKit::extraRepos()
{
    QStringList repos;
    foreach (const QString& repoId, myDefaults.keys()) {
        if (!myRequiredRepositories.contains(repoId)) {
            repos.append(repoId);
        }
    }
    return repos;
}

void RepositoryTransactionPkgKit::setSsuVersion(const QString& version)
{
    qDebug() << Q_FUNC_INFO
             << "new version" << version
             << "current version" << Ssu().release(false);

    if (!version.isEmpty() && version != Ssu().release(false)) {
        qDebug() << "changing SSU release:" << version;

        // set the SSU release twice, once locally, and once via D-Bus to make
        // sure that both, the local libssu, and the remote ssud change to the
        // same release instantly without having to rely on QSettings sync to
        // take place; otherwise we'd get a nasty race condition ending up with
        // the wrong SSU release

        Ssu().setRelease(version, false);

        QDBusMessage methodCall = QDBusMessage::createMethodCall(
                    QStringLiteral("org.nemo.ssu"),
                    QStringLiteral("/org/nemo/ssu"),
                    QStringLiteral("org.nemo.ssu"),
                    QStringLiteral("setRelease"));

        QVariantList arguments;
        arguments << version;
        arguments << false;
        methodCall.setArguments(arguments);
        QDBusConnection::systemBus().call(methodCall);
    }
}

void RepositoryTransactionPkgKit::restoreSsuVersion()
{
    if (myWrappedTransactionFailed || myRestoreVersionAtSuccess) {
        // only revert if the version was modified for this operation
        if (!myOriginalSsuVersion.isEmpty()) {
            // Reset SSU back to original if it was modified
            qDebug() << "Resetting ssu version";
            setSsuVersion(myOriginalSsuVersion);
        }
    }
}

void RepositoryTransactionPkgKit::slotListReposFinished(PackageKit::Transaction::Exit status,
                                                        uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)
    verifyRequired();
}

void RepositoryTransactionPkgKit::slotIsolateRepositoryFinished(PackageKit::Transaction::Exit status,
                                                                uint runtime)
{
    Q_UNUSED(runtime)
    if (status == PackageKit::Transaction::ExitSuccess) {
        isolateNext();
    } else {
        // couldn't isolate the repositories, so it's better to not run the
        // wrapped transaction
        giveUp(QStringList() << myCurrentlyProcessedRepository);
    }
}

void RepositoryTransactionPkgKit::slotRestoreRepositoryFinished(PackageKit::Transaction::Exit status,
                                                                uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)
    // there's no point in giving up if restoring a repository failed
    restoreNext(false);
}

void RepositoryTransactionPkgKit::slotGotRepository(const QString& repoId,
                                                    const QString& description,
                                                    bool enabled)
{
    qDebug() << "Repository:" << repoId << description << enabled;
    myActualRepositories << repoId;
    myDefaults.insert(repoId, enabled);
}

void RepositoryTransactionPkgKit::slotWrappedTransactionFinished()
{
    if (myWrappedTransaction->isAtomic()) {
        restoreSsuVersion();
        deleteLater();
        emit finished();
    } else {
        myRepositoriesToProcess = extraRepos();
        restoreNext(false);
    }
}

void RepositoryTransactionPkgKit::slotWrappedTransactionFailure(const QString& details)
{
    myWrappedTransactionFailed = true;
    qDebug() << "Wrapped transaction failed:" << details;
}

void RepositoryTransactionPkgKit::slotRepositoryModificationError(
        PackageKit::Transaction::Error error, const QString& details)
{
    qDebug() << "Failure modifying repository: " << error << details;
}

}
