#include "manager.h"

namespace PackageManagement
{

class ManagerPrivate
{
};

Manager::UpgradeInformation::UpgradeInformation()
    : bytesToDownload(-1)
    , bytesToInstall(-1)
    , bytesToRemove(-1)
{
}


Manager::Manager(QObject *parent)
    : QObject(parent)
    , d(new ManagerPrivate)
{
}

Manager::~Manager()
{
    delete d;
}

}
