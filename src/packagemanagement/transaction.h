#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_TRANSACTION_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_TRANSACTION_H

#include "nemo-osupdate/global.h"

#include <QObject>
#include <QString>

namespace PackageManagement
{

class TransactionPrivate;

/* Abstract base class for package manager transactions.
 *
 * Transactions deriving from this class are supposed to destroy themself after
 * they are finished.
 *
 * A transaction has an optional progress, and may either succeed for fail.
 */
class NEMO_OSUPDATE_EXPORT Transaction : public QObject
{
    Q_OBJECT
public:
    explicit Transaction(QObject *parent = nullptr);
    ~Transaction();

    virtual void start() = 0;

    // Override if special actions are needed when cancelling a transaction.
    virtual void cancel();

    // Override if the transaction is atomic on the backend, i.e. no other
    // processes can interfere. This may allow for optimizations at some
    // places, if this fact is known.
    virtual bool isAtomic() const;

signals:
    // Gets emitted when the transaction progress changed.
    // A progress value is -1 for undetermined, or between 0 and 100
    void progress(int value);

    // Gets emitted when the transaction failed. A failed transaction has
    // terminated.
    void failure(const QString& details);

    // Gets emitted when transaction succeeded. A succeeded transaction has
    // terminated.
    void success();

    // Gets emitted when the transaction finished, no matter if successful
    // or not.
    void finished();

private:
    TransactionPrivate *d;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_TRANSACTION_H
