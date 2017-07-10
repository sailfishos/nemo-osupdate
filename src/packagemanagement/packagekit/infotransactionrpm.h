#ifndef NEMO_OSUPDATE_PACKAGEMANAGEMENT_INFOTRANSACTIONRPM_H
#define NEMO_OSUPDATE_PACKAGEMANAGEMENT_INFOTRANSACTIONRPM_H

#include "manager.h"
#include "transaction.h"
#include <QString>
#include <QVariantList>

struct rpmtd_s;

namespace PackageManagement
{

class InfoTransactionRpm : public Transaction
{
    Q_OBJECT
public:
    InfoTransactionRpm(const QString& filename,
                       Manager::PackageInformation::Ptr info);
    virtual void start();

private:
    QVariantList readTagData(rpmtd_s* tagData);

private slots:
    void slotFinished();

private:
    bool mySuccess;
    QString myErrorDetails;
};

}

#endif // NEMO_OSUPDATE_PACKAGEMANAGEMENT_INFOTRANSACTIONRPM_H
