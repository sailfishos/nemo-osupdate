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

#ifndef NEMO_OS_UPDATER_P_H
#define NEMO_OS_UPDATER_P_H

#include "osupdater.h"

// nemo-qml-plugin-systemsettings
#include <batterystatus.h>

namespace Nemo {

class OsUpdaterPrivate
{
public:
    explicit OsUpdaterPrivate()
        : status(OsUpdater::Unknown)
        , progress(0)
        , osDownloadSize(0)
        , osInstallSize(0)
        , availableSpaceForDownload(0)
        , availableSpaceForInstall(0)
        , validOsInstallSize(false)
        , checked(false)
        , errorState(OsUpdater::UnknownErrorState)
        , sufficientSpaceForDownload(false)
        , sufficientSpaceForInstall(false)
        , sufficientBatteryForInstall(false)
        , batteryLevel(0)
        , batteryChargerConnected(false)
    {

    }

    QString backendName;
    OsUpdater::Status status;
    int progress;
    QString osVersion;
    QString osCodeName;
    QString osSummary;
    QString osCover;
    QString osWebsite;
    qlonglong osDownloadSize;
    qlonglong osInstallSize;
    qlonglong availableSpaceForDownload;
    qlonglong availableSpaceForInstall;
    bool validOsInstallSize;
    bool checked;
    QDateTime lastChecked;
    OsUpdater::ErrorState errorState;
    QString errorString;

    bool sufficientSpaceForDownload;
    bool sufficientSpaceForInstall;

    // Battery status
    bool sufficientBatteryForInstall;
    int batteryLevel;
    bool batteryChargerConnected;

    BatteryStatus batteryStatus;
};

}

#endif
