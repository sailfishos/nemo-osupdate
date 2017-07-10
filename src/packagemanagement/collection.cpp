#include "collection.h"

namespace PackageManagement
{

class CollectionPrivate
{
public:
    bool myIsClosed;
    QList<Collection::Package> myPackages;
};

Collection::Package::Package(const QString& theName,
                             const QString& theVersion,
                             const QString& theSummary)
    : name(theName)
    , version(theVersion)
    , summary(theSummary)
{
}


Collection::Collection()
    : d(new CollectionPrivate)
{
    d->myIsClosed = false;
}

Collection::~Collection()
{
    delete d;
}

void Collection::append(const Collection::Package& package)
{
    if (!d->myIsClosed) {
        d->myPackages << package;
    }
}

int Collection::size() const
{
    return d->myPackages.size();
}

const QList<Collection::Package>& Collection::packages() const
{
    return d->myPackages;
}

bool Collection::isClosed() const
{
    return d->myIsClosed;
}

void Collection::close()
{
    d->myIsClosed = true;
}

void Collection::clear()
{
    d->myIsClosed = false;
    d->myPackages.clear();
}

}
