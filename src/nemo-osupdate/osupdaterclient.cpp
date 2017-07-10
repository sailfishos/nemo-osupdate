/* Copyright (C) 2017 Jolla Ltd.
 * Contact: Raine Makelainen <raine.makelainen@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "osupdaterclient.h"
#include "osupdater_p.h"

#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QLoggingCategory>

#include "osupdater_interface.h"

Q_LOGGING_CATEGORY(osUpdaterClient, "org.nemo.osupdater.client", QtDebugMsg)

using namespace Nemo;

namespace Nemo
{
class OsUpdaterClientPrivate : public OsUpdaterPrivate
{
public:
    explicit OsUpdaterClientPrivate(const QDBusConnection &dbusConnection, OsUpdaterClient *parent)
        : OsUpdaterPrivate()
        , dbusInterface(new OrgNemoOsupdaterInterface(OsUpdater::ServiceName, OsUpdater::ServiceObject, dbusConnection, parent))
        , dbusConnection(dbusConnection)
        , valid(dbusInterface->isValid())
    {
    }
    ~OsUpdaterClientPrivate() {
        delete dbusInterface;
    }

    OrgNemoOsupdaterInterface *dbusInterface;
    QDBusConnection dbusConnection;
    bool valid;
};
}

OsUpdaterClient::OsUpdaterClient(const QDBusConnection &dbusConnection, QObject *parent)
    : OsUpdater(new OsUpdaterClientPrivate(dbusConnection, this), parent)
{
    Q_D(OsUpdaterClient);

    if (!isValid()) {
        qCWarning(osUpdaterClient) << "Client: Updater interface is not valid!";
    }
    qCDebug(osUpdaterClient) << Q_FUNC_INFO << d->dbusInterface->osDownloadSize();
    setInterfaceStatus(d->dbusInterface->status());
    setInterfaceLastChecked(d->dbusInterface->lastChecked());
    setOsVersion(d->dbusInterface->osVersion());
    setOsCodeName(d->dbusInterface->osCodeName());
    setOsSummary(d->dbusInterface->osSummary());
    setOsCover(d->dbusInterface->osCover());
    setOsWebsite(d->dbusInterface->osWebsite());
    setProgress(d->dbusInterface->progress());
    setOsDownloadSize(d->dbusInterface->osDownloadSize());

    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osCodeNameChanged,
            this, &OsUpdaterClient::setOsCodeName);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osCoverChanged,
            this, &OsUpdaterClient::setOsCover);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osDownloadSizeChanged,
            this, &OsUpdaterClient::setOsDownloadSize);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osInstallSizeChanged,
            this, &OsUpdaterClient::setOsInstallSize);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osSummaryChanged,
            this, &OsUpdaterClient::setOsSummary);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osVersionChanged,
            this, &OsUpdaterClient::setOsVersion);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::osWebsiteChanged,
            this, &OsUpdaterClient::setOsWebsite);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::progressChanged,
            this, &OsUpdaterClient::setProgress);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::checkedChanged,
            this, &OsUpdaterClient::setChecked);
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::validOsInstallSizeChanged,
            this, &OsUpdaterClient::setValidOsInstallSize);

    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::lastCheckedChanged, this, [this](qlonglong lastChecked) {
        setLastChecked(QDateTime::fromMSecsSinceEpoch(lastChecked));
    });
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::statusChanged, this, [this](int status) {
        setStatus(static_cast<Status>(status));
    });
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::errorStateChanged, this, [this](int state) {
        setErrorState(static_cast<ErrorState>(state));
    });
    connect(d->dbusInterface, &OrgNemoOsupdaterInterface::errorStringChanged, this, &OsUpdaterClient::errorStringChanged);
}

OsUpdaterClient::~OsUpdaterClient()
{
    delete d_ptr;
}

bool OsUpdaterClient::isValid() const
{
    Q_D(const OsUpdaterClient);
    return d->valid;
}

void OsUpdaterClient::checkForUpdate(bool refreshCache, bool forced)
{
    Q_D(const OsUpdaterClient);
    d->dbusInterface->checkForUpdate(refreshCache, forced);
}

void OsUpdaterClient::downloadUpdate(const QString &version)
{
    Q_D(const OsUpdaterClient);
    d->dbusInterface->downloadUpdate(version);
}

void OsUpdaterClient::installDownloadedUpdate(const QString &version)
{
    Q_D(const OsUpdaterClient);
    d->dbusInterface->installDownloadedUpdate(version);
}

void OsUpdaterClient::setInterfaceStatus(int newStatus)
{
    qCDebug(osUpdaterClient) << Q_FUNC_INFO << newStatus;
}

void Nemo::OsUpdaterClient::checkForUpdate()
{
    checkForUpdate(true, true);
}

void Nemo::OsUpdaterClient::downloadUpdate()
{
    Q_D(OsUpdaterClient);
    if (!d->sufficientSpaceForDownload) {
        qWarning() << "Insufficient space to download update!  Aborting!";
    } else if (!d->sufficientBatteryForInstall) {
        qWarning() << "Insufficient battery to download update!  Aborting!";
    } else {
        downloadUpdate(osVersion());
    }
}

void Nemo::OsUpdaterClient::installDownloadedUpdate()
{
    installDownloadedUpdate(osVersion());
}
