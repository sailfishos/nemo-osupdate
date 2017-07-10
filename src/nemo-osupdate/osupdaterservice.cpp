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

#include "osupdaterservice.h"
#include "osupdater_p.h"

// Generated
#include "osupdateradaptor.h"

#include <QDBusAbstractAdaptor>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(osUpdaterService, "org.nemo.osupdater.service", QtDebugMsg)

using namespace Nemo;

namespace Nemo
{
class OsUpdaterServicePrivate : public OsUpdaterPrivate
{
public:
    explicit OsUpdaterServicePrivate(const QDBusConnection &dbusConnection)
        : OsUpdaterPrivate()
        , adaptor(nullptr)
        , dbusConnection(dbusConnection)
    {
    }

    ~OsUpdaterServicePrivate()
    {
        delete adaptor;
    }

    OsUpdaterAdaptor *adaptor;
    QDBusConnection dbusConnection;
};
}

OsUpdaterService::OsUpdaterService(const QDBusConnection &dbusConnection, QObject *parent)
    : OsUpdater(new OsUpdaterServicePrivate(dbusConnection), parent)
{
}

OsUpdaterService::~OsUpdaterService()
{
    delete d_ptr;
}

bool OsUpdaterService::registerService()
{
    Q_D(OsUpdaterService);
    d->adaptor = new OsUpdaterAdaptor(this);
    connect(this, &OsUpdater::statusChanged, d->adaptor, &OsUpdaterAdaptor::statusChanged);
    connect(this, &OsUpdater::progressChanged, d->adaptor, &OsUpdaterAdaptor::progressChanged);

    connect(this, &OsUpdater::osVersionChanged, d->adaptor, &OsUpdaterAdaptor::osVersionChanged);
    connect(this, &OsUpdater::osCodeNameChanged, d->adaptor, &OsUpdaterAdaptor::osCodeNameChanged);
    connect(this, &OsUpdater::osSummaryChanged, d->adaptor, &OsUpdaterAdaptor::osSummaryChanged);
    connect(this, &OsUpdater::osCoverChanged, d->adaptor, &OsUpdaterAdaptor::osCoverChanged);
    connect(this, &OsUpdater::osWebsiteChanged, d->adaptor, &OsUpdaterAdaptor::osWebsiteChanged);
    connect(this, &OsUpdater::osDownloadSizeChanged, d->adaptor, &OsUpdaterAdaptor::osDownloadSizeChanged);
    connect(this, &OsUpdater::osInstallSizeChanged, d->adaptor, &OsUpdaterAdaptor::osInstallSizeChanged );
    connect(this, &OsUpdater::lastCheckedChanged, this, [this, d](QDateTime lastChecked) {
        emit d->adaptor->lastCheckedChanged(lastChecked.isValid() ? lastChecked.toMSecsSinceEpoch() : 0);
    });

    if (!d->dbusConnection.registerObject(ServiceObject, this)) {
        qCWarning(osUpdaterService) << "Unable to register updater DBus object";
        return false;
    }
    if (!d->dbusConnection.registerService(ServiceName)) {
        qCWarning(osUpdaterService) << "Unable to register updater DBus service" << ServiceName;
        return false;
    }

    return true;
}

void OsUpdaterService::setScreenShutdownModeUpgrade()
{
    qCDebug(osUpdaterService) << Q_FUNC_INFO;
    QDBusMessage clearScreenCall = QDBusMessage::createMethodCall(
                "org.nemomobile.lipstick",
                "/shutdown",
                "org.nemomobile.lipstick",
                "setShutdownMode");
    clearScreenCall.setArguments({ QStringLiteral("upgrade") });
    QDBusConnection::systemBus().call(clearScreenCall, QDBus::BlockWithGui);
}

void OsUpdaterService::setSystemUpgradeMode()
{
    qCDebug(osUpdaterService) << Q_FUNC_INFO;
    QDBusMessage setUpgradeModeCall = QDBusMessage::createMethodCall(
                "org.freedesktop.systemd1",
                "/org/freedesktop/systemd1",
                "org.freedesktop.systemd1.Manager",
                "StartUnit");
    setUpgradeModeCall.setArguments({ QStringLiteral("sw-update.target") , QStringLiteral("isolate") });
    QDBusConnection::sessionBus().call(setUpgradeModeCall, QDBus::BlockWithGui);
}

void OsUpdaterService::requestBlankingPause()
{
    qCDebug(osUpdaterService) << Q_FUNC_INFO;
    QDBusMessage blankingPauseCall = QDBusMessage::createMethodCall(
                "com.nokia.mce",
                "/com/nokia/mce/request",
                "com.nokia.mce.request",
                "req_display_blanking_pause");
    QDBusConnection::systemBus().call(blankingPauseCall, QDBus::BlockWithGui);
}

void OsUpdaterService::requestDelayedReboot()
{
    qCDebug(osUpdaterService) << Q_FUNC_INFO;
    QTimer::singleShot(1100, this, &OsUpdaterService::requestReboot);
}

void OsUpdaterService::requestReboot()
{
    qCDebug(osUpdaterService) << Q_FUNC_INFO;
    QDBusMessage reqRebootCall = QDBusMessage::createMethodCall(
                "com.nokia.dsme",
                "/com/nokia/dsme/request",
                "com.nokia.dsme.request",
                "req_reboot");
    QDBusConnection::systemBus().call(reqRebootCall, QDBus::BlockWithGui);
}
