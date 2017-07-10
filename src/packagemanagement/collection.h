#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_COLLECTION_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_COLLECTION_H

#include "nemo-osupdate/global.h"

#include <QSharedPointer>
#include <QStringList>

namespace PackageManagement
{

class CollectionPrivate;

/* A collection of packages.
 *
 * This class is for asynchronous listing of data, but it is not thread-safe.
 * If a transaction implementation requires thread-safety, it has to handle it
 * on its own.
 */
class NEMO_OSUPDATE_EXPORT Collection
{
public:
    typedef QSharedPointer<Collection> Ptr;

    struct Package
    {
        Package(const QString& theName,
                const QString& theVersion,
                const QString& theSummary);

        QString name;
        QString version;
        QString summary;
    };

    Collection();
    ~Collection();

    void append(const Package& package);
    int size() const;

    const QList<Package>& packages() const;

    bool isClosed() const;
    void close();

    void clear();

private:
    CollectionPrivate *d;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_COLLECTION_H
