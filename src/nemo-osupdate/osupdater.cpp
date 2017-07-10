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

#include "osupdater.h"
#include "osupdater_p.h"

#include <QLoggingCategory>
#include <QStorageInfo>

Q_LOGGING_CATEGORY(osupdater, "org.nemo.osupdater", QtDebugMsg)

namespace {
// Battery threshold percentage to allow installation of update when not plugged into charger.
const int BATTERY_THRESHOLD = 50;
}

using namespace Nemo;

const QString OsUpdater::ServiceName = QStringLiteral("org.nemo.osupdater");
const QString OsUpdater::ServiceObject = QStringLiteral("/Updater");
const QString OsUpdater::ServiceInterface = OsUpdater::ServiceName;

OsUpdater::OsUpdater(OsUpdaterPrivate *d, QObject *parent)
    : QObject(parent)
    , d_ptr(d)
{
    connect(&d_ptr->batteryStatus, &BatteryStatus::chargePercentageChanged,
            [this] {
        d_ptr->batteryLevel = d_ptr->batteryStatus.chargePercentage();
        bool sufficientBattery = d_ptr->batteryLevel > BATTERY_THRESHOLD || d_ptr->batteryChargerConnected;
        if (d_ptr->sufficientBatteryForInstall != sufficientBattery) {
            d_ptr->sufficientBatteryForInstall = sufficientBattery;
            emit this->sufficientBatteryForInstallChanged(sufficientBattery);
        }
    });
    connect(&d_ptr->batteryStatus, &BatteryStatus::chargerStatusChanged,
            [this] {
        bool chargerConnected = d_ptr->batteryStatus.chargerStatus() == BatteryStatus::Connected;
        if (d_ptr->batteryChargerConnected != chargerConnected) {
            d_ptr->batteryChargerConnected = chargerConnected;
            emit this->batteryChargerConnectedChanged(chargerConnected);
        }
        bool sufficientBattery = d_ptr->batteryLevel > BATTERY_THRESHOLD || chargerConnected;
        if (d_ptr->sufficientBatteryForInstall != sufficientBattery) {
            d_ptr->sufficientBatteryForInstall = sufficientBattery;
            emit this->sufficientBatteryForInstallChanged(sufficientBattery);
        }
    });
}

OsUpdater::~OsUpdater()
{
    delete d_ptr;
}

QString OsUpdater::backendName() const
{
    Q_D(const OsUpdater);
    return d->backendName;
}

OsUpdater::Status OsUpdater::status() const
{
    Q_D(const OsUpdater);
    return d->status;
}

int OsUpdater::progress() const
{
    Q_D(const OsUpdater);
    return d->progress;
}

QString OsUpdater::osVersion() const
{
    Q_D(const OsUpdater);
    return d->osVersion;
}

QString OsUpdater::osCodeName() const
{
    Q_D(const OsUpdater);
    return d->osCodeName;
}

QString OsUpdater::osSummary() const
{
    Q_D(const OsUpdater);
    return d->osSummary;
}

QString OsUpdater::osCover() const
{
    Q_D(const OsUpdater);
    return d->osCover;
}

QString OsUpdater::osWebsite() const
{
    Q_D(const OsUpdater);
    return d->osWebsite;
}

qlonglong OsUpdater::osDownloadSize() const
{
    Q_D(const OsUpdater);
    return d->osDownloadSize;
}

qlonglong OsUpdater::osInstallSize() const
{
    Q_D(const OsUpdater);
    return d->osInstallSize;
}

qlonglong OsUpdater::availableSpaceForDownload() const
{
    Q_D(const OsUpdater);
    return d->availableSpaceForDownload;
}

qlonglong OsUpdater::availableSpaceForInstall() const
{
    Q_D(const OsUpdater);
    return d->availableSpaceForInstall;
}

bool OsUpdater::validOsInstallSize() const
{
    Q_D(const OsUpdater);
    return d->validOsInstallSize;
}

bool OsUpdater::checked() const
{
    Q_D(const OsUpdater);
    return d->lastChecked.isValid();
}

QDateTime OsUpdater::lastChecked() const
{
    Q_D(const OsUpdater);
    return d->lastChecked;
}

OsUpdater::ErrorState OsUpdater::errorState() const
{
    Q_D(const OsUpdater);
    return d->errorState;
}

QString OsUpdater::errorString() const
{
    Q_D(const OsUpdater);
    return d->errorString;
}

bool OsUpdater::sufficientSpaceForDownload() const
{
    Q_D(const OsUpdater);
    return d->sufficientSpaceForDownload;
}

bool OsUpdater::sufficientSpaceForInstall() const
{
    Q_D(const OsUpdater);
    return d->sufficientSpaceForInstall;
}

bool OsUpdater::sufficientBatteryForInstall() const
{
    Q_D(const OsUpdater);
    return d->sufficientBatteryForInstall;
}

bool OsUpdater::batteryChargerConnected() const
{
    Q_D(const OsUpdater);
    return d->batteryChargerConnected;
}

void OsUpdater::calculateDiskSpaceRequirements()
{
    Q_D(OsUpdater);
    // Recalculate whether we have enough space and battery to perform download/installation.
    bool downloadComplete = d->progress == 100;

    QStorageInfo rootStorage = QStorageInfo::root();
    rootStorage.refresh();

    qint64 rootBytesAvailable = rootStorage.bytesAvailable();
    qint64 homeBytesAvailable = -1;
    for (QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady() && !storage.isReadOnly()) {
            if (storage.rootPath() == QStringLiteral("/home")) {
                storage.refresh();
                homeBytesAvailable = storage.bytesAvailable();
                break;
            }
        }
    }

    bool sufficientSpaceForDownload = false;
    bool sufficientSpaceForInstall = false;

    qint64 availableSpaceForDownload = homeBytesAvailable >= 0 ? homeBytesAvailable : rootBytesAvailable;
    qint64 availableSpaceForInstall = homeBytesAvailable >= 0 || downloadComplete ? rootBytesAvailable : rootBytesAvailable - d->osDownloadSize;

    if (downloadComplete) {
        sufficientSpaceForDownload = true; // already downloaded.
        sufficientSpaceForInstall = availableSpaceForInstall > d->osInstallSize;
    } else {
        sufficientSpaceForDownload = availableSpaceForDownload > d->osDownloadSize;
        sufficientSpaceForInstall = availableSpaceForInstall > d->osInstallSize;
    }

    if (sufficientSpaceForDownload != d->sufficientSpaceForDownload) {
        d->sufficientSpaceForDownload = sufficientSpaceForDownload;
        emit sufficientSpaceForDownloadChanged(sufficientSpaceForDownload);
    }
    if (sufficientSpaceForInstall != d->sufficientSpaceForInstall) {
        d->sufficientSpaceForInstall = sufficientSpaceForInstall;
        emit sufficientSpaceForInstallChanged(sufficientSpaceForInstall);
    }
    if (availableSpaceForDownload != d->availableSpaceForDownload) {
        d->availableSpaceForDownload = availableSpaceForDownload;
        emit availableSpaceForDownloadChanged(availableSpaceForDownload);
    }
    if (availableSpaceForInstall != d->availableSpaceForInstall) {
        d->availableSpaceForInstall = availableSpaceForInstall;
        emit availableSpaceForInstallChanged(availableSpaceForInstall);
    }
}

void OsUpdater::setStatus(OsUpdater::Status status)
{
    Q_D(OsUpdater);
    if (status != d->status) {
        d->status = status;
        qCDebug(osupdater) << Q_FUNC_INFO << status;
        emit statusChanged(status);
    }
}

void OsUpdater::setOsVersion(const QString &osVersion)
{
    Q_D(OsUpdater);
    if (osVersion != d->osVersion) {
        d->osVersion = osVersion;
        qCDebug(osupdater) << Q_FUNC_INFO << osVersion;
        emit osVersionChanged(osVersion);
    }
}

void OsUpdater::setOsCodeName(const QString &osCodeName)
{
    Q_D(OsUpdater);
    if (osCodeName != d->osCodeName) {
        d->osCodeName = osCodeName;
        qCDebug(osupdater) << Q_FUNC_INFO << osCodeName;
        emit osCodeNameChanged(osCodeName);
    }
}

void OsUpdater::setOsSummary(const QString &osSummary)
{
    Q_D(OsUpdater);
    if (osSummary != d->osSummary) {
        d->osSummary = osSummary;
        qCDebug(osupdater) << Q_FUNC_INFO << osSummary;
        emit osSummaryChanged(osSummary);
    }
}

void OsUpdater::setOsCover(const QString &osCover)
{
    Q_D(OsUpdater);
    if (osCover != d->osCover) {
        d->osCover = osCover;
        qCDebug(osupdater) << Q_FUNC_INFO << osCover;
        emit osCoverChanged(osCover);
    }
}

void OsUpdater::setOsWebsite(const QString &osWebsite)
{
    Q_D(OsUpdater);
    if (osWebsite != d->osWebsite) {
        d->osWebsite = osWebsite;
        qCDebug(osupdater) << Q_FUNC_INFO << osWebsite;
        emit osWebsiteChanged(osWebsite);
    }
}

void OsUpdater::setProgress(int progress)
{
    Q_D(OsUpdater);
    if (progress != d->progress) {
        d->progress = progress;
        qCDebug(osupdater) << Q_FUNC_INFO << progress;
        emit progressChanged(progress);

        if (progress == 100) {
            calculateDiskSpaceRequirements();
        }
    }
}

void OsUpdater::setOsDownloadSize(qlonglong osDownloadSize)
{
    Q_D(OsUpdater);
    if (osDownloadSize != d->osDownloadSize) {
        d->osDownloadSize = osDownloadSize;
        qCDebug(osupdater) << Q_FUNC_INFO << osDownloadSize;
        emit osDownloadSizeChanged(osDownloadSize);

        calculateDiskSpaceRequirements();
    }
}

void OsUpdater::setOsInstallSize(qlonglong osInstallSize)
{
    Q_D(OsUpdater);
    if (osInstallSize != d->osInstallSize) {
        d->osInstallSize = osInstallSize;
        qCDebug(osupdater) << Q_FUNC_INFO << osInstallSize;
        emit osInstallSizeChanged(osInstallSize);

        setValidOsInstallSize(osInstallSize > 0);

        calculateDiskSpaceRequirements();
    }
}

void OsUpdater::setValidOsInstallSize(bool validOsInstallSize)
{
    Q_D(OsUpdater);
    if (validOsInstallSize != d->validOsInstallSize) {
        d->validOsInstallSize = validOsInstallSize;
        qCDebug(osupdater) << Q_FUNC_INFO << validOsInstallSize;
        emit validOsInstallSizeChanged(validOsInstallSize);
    }
}

void OsUpdater::setChecked(bool checked)
{
    Q_D(OsUpdater);
    if (checked != d->checked) {
        d->checked = checked;
        qCDebug(osupdater) << Q_FUNC_INFO << checked;
        emit checkedChanged(checked);
    }
}

void OsUpdater::setLastChecked(const QDateTime &lastChecked)
{
    Q_D(OsUpdater);
    if (lastChecked != d->lastChecked) {
        d->lastChecked = lastChecked;
        qCDebug(osupdater) << Q_FUNC_INFO << lastChecked;
        emit lastCheckedChanged(lastChecked);
    }
}

void OsUpdater::setErrorState(OsUpdater::ErrorState errorState)
{
    Q_D(OsUpdater);
    if (errorState != d->errorState) {
        d->errorState = errorState;
        qCDebug(osupdater) << Q_FUNC_INFO << errorState;
        emit errorStateChanged(errorState);
    }
}

void OsUpdater::setErrorString(const QString &errorString)
{
    Q_D(OsUpdater);
    if (errorString != d->errorString) {
        d->errorString = errorString;
        qCDebug(osupdater) << Q_FUNC_INFO << errorString;
        emit errorStringChanged(errorString);
    }
}
