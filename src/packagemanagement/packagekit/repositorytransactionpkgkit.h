#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_REPOSITORYTRANSACTIONPKGKIT_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_REPOSITORYTRANSACTIONPKGKIT_H

#include "transaction.h"

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

// PackageKit
#include <PackageKit/packagekit-qt5/Transaction>

namespace PackageManagement
{

/* Repositories enabling/disabling/verifying transaction implementation for
 * PackageKit backend.
 */
class RepositoryTransactionPkgKit : public Transaction
{
    Q_OBJECT
public:
    /* Creates a new repository transaction. repos is the list of required
     * repositories that should be enabled.
     * Pass an empty list to reset to defaults.
     * The failure details contain a space-separated list of missing repositories.
     * If restoreVersionAtSuccess is false, the SSU version will not be reverted
     * to the previous version after the wrapped transaction succeeded.
     */
    explicit RepositoryTransactionPkgKit(Transaction* tx,
                                         const QString& version,
                                         bool restoreVersionAtSuccess,
                                         const QStringList& repos);

    virtual void start();
    virtual void cancel();

private:
    /* Verifies that all required repositories are available.
     */
    void verifyRequired();

    /* Isolates the next repository that is to be processed by enabling it, if
     * required, or disabling if not required.
     */
    void isolateNext();

    /* Restores the next repository that is to be processed by restoring the
     * state it had before isolating. If atomic is true, all restore
     * operation get queued into the PackageKit transaction queue at once,
     * and slotRestoreRepositoryFinished will not be called.
     */
    void restoreNext(bool atomic);

    /* Runs the wrapped transaction.
     */
    void runTransaction();

    void giveUp(const QStringList& failedRepositories);

    QStringList extraRepos();

    void setSsuVersion(const QString& version);
    void restoreSsuVersion();

private slots:
    void slotListReposFinished(PackageKit::Transaction::Exit status,
                               uint runtime);
    void slotIsolateRepositoryFinished(PackageKit::Transaction::Exit status,
                                       uint runtime);
    void slotRestoreRepositoryFinished(PackageKit::Transaction::Exit status,
                                       uint runtime);
    void slotGotRepository(const QString& repoId,
                           const QString& description,
                           bool enabled);
    void slotWrappedTransactionFinished();
    void slotWrappedTransactionFailure(const QString& details);
    void slotRepositoryModificationError(PackageKit::Transaction::Error error,
                                         const QString& details);

private:
    // the default repository settings (these may be user-tinkered and are not
    // necessarily sane settings; that's why we don't want to work with these)
    QMap<QString, bool> myDefaults;

    Transaction* myWrappedTransaction;
    bool myWrappedTransactionFailed;

    QString myVersion;
    QString myOriginalSsuVersion;
    bool myRestoreVersionAtSuccess;
    QStringList myRequiredRepositories;
    QStringList myActualRepositories;

    // ID of the currently processed repository (to identify it in error case)
    QString myCurrentlyProcessedRepository;

    // a list of repositories to process for isolating / restoring
    QStringList myRepositoriesToProcess;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_REPOSITORYTRANSACTIONPKGKIT_H
