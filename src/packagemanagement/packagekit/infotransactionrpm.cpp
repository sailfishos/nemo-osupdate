#include "infotransactionrpm.h"

#include <QTimer>
#include <QDebug>

// rpmlib
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

namespace PackageManagement
{

InfoTransactionRpm::InfoTransactionRpm(const QString& filename,
                                       Manager::PackageInformation::Ptr info)
    : mySuccess(false)
{
    info->file = filename;
    FD_t fd = Fopen(filename.toUtf8().constData(), "r");

    if (fd != 0) {
        Header hdr;
        rpmReadConfigFiles(NULL, NULL);
        rpmts ts = rpmtsCreate();

        rpmRC status = rpmReadPackageFile(ts, fd, filename.toUtf8().constData(),
                                        &hdr);
        if (status != RPMRC_OK
                && status != RPMRC_NOTTRUSTED
                && status != RPMRC_NOKEY) {
            myErrorDetails = QString(strerror(errno));
            qDebug() << "could not read rpm:" << status << myErrorDetails;
        } else {
            HeaderIterator iter = headerInitIterator(hdr);

            rpmtd td = rpmtdNew();
            for (HeaderIterator iter = headerInitIterator(hdr);
                 headerNext(iter, td);) {

                rpmTagVal tag = rpmtdTag(td);
                QVariantList values = readTagData(td);

                if (!values.isEmpty()) {
                    switch (tag) {
                    case RPMTAG_NAME:
                        info->name = values.at(0).toString();
                        break;
                    case RPMTAG_SUMMARY:
                        info->summary = values.at(0).toString();
                        break;
                    case RPMTAG_VERSION:
                        info->version = values.at(0).toString();
                    default:
                        break;
                    }
                }

                rpmtdFreeData(td);
            }

            qDebug() << "RPM Package Info:" << info->name << info->version
                     << info->summary;

            headerFreeIterator(iter);
            headerFree(hdr);

            mySuccess = true;
        }
        rpmtsFree(ts);
        Fclose(fd);
    }
}

void InfoTransactionRpm::start()
{
    QTimer::singleShot(0, this, SLOT(slotFinished()));
}

QVariantList InfoTransactionRpm::readTagData(rpmtd_s* tagData)
{
    QVariantList values;

    rpmTagType tagType = rpmtdType(tagData);
    rpmtdInit(tagData);
    int nextStatus = rpmtdNext(tagData);
    while (nextStatus != -1) {
        switch (tagType) {
        case RPM_INT32_TYPE:
            values << QVariant(*(rpmtdGetUint32(tagData)));
            break;
        case RPM_INT64_TYPE:
            values << QVariant(qulonglong(*(rpmtdGetUint64(tagData))));
            break;
        case RPM_STRING_TYPE:
        case RPM_STRING_ARRAY_TYPE:
        case RPM_I18NSTRING_TYPE:
            values << QVariant(rpmtdGetString(tagData));
            break;
        default:
            break;
        }

        nextStatus = rpmtdNext(tagData);
    }

    return values;
}

void InfoTransactionRpm::slotFinished()
{
    deleteLater();

    if (mySuccess) {
        emit success();
    } else {
        emit failure(myErrorDetails);
    }
    emit finished();
}

}
