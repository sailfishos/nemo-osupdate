#include "transaction.h"

namespace PackageManagement
{

class TransactionPrivate
{
};

Transaction::Transaction(QObject *parent)
    : QObject(parent)
    , d(new TransactionPrivate)
{
}

Transaction::~Transaction()
{
    // Delete children (PackageKit::Transactions in practice) explicitly in
    // order to avoid dead locks in PackageKit::Transaction::disconnectNotify.
    while (children().count()) {
        delete children().at(0);
    }

    delete d;
}

void Transaction::cancel()
{
    deleteLater();
    emit failure("cancelled");
    emit finished();
}

bool Transaction::isAtomic() const
{
    return false;
}

}
