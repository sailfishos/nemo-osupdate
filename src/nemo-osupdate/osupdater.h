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

#ifndef NEMO_OS_UPDATER_H
#define NEMO_OS_UPDATER_H

#include <global.h>

#include <QObject>
#include <QDateTime>

namespace Nemo {

class OsUpdaterPrivate;

class NEMO_OSUPDATE_EXPORT OsUpdater : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nemo.osupdater")

    Q_PROPERTY(QString backendName READ backendName CONSTANT)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString osVersion READ osVersion NOTIFY osVersionChanged)
    Q_PROPERTY(QString osCodeName READ osCodeName NOTIFY osCodeNameChanged)
    Q_PROPERTY(QString osSummary READ osSummary NOTIFY osSummaryChanged)
    Q_PROPERTY(QString osCover READ osCover NOTIFY osCoverChanged)
    Q_PROPERTY(QString osWebsite READ osWebsite NOTIFY osWebsiteChanged)
    Q_PROPERTY(qlonglong osDownloadSize READ osDownloadSize NOTIFY osDownloadSizeChanged)
    Q_PROPERTY(qlonglong osInstallSize READ osInstallSize NOTIFY osInstallSizeChanged)
    Q_PROPERTY(qlonglong availableSpaceForDownload READ availableSpaceForDownload NOTIFY availableSpaceForDownloadChanged)
    Q_PROPERTY(qlonglong availableSpaceForInstall READ availableSpaceForInstall NOTIFY availableSpaceForInstallChanged)
    Q_PROPERTY(bool validOsInstallSize READ validOsInstallSize NOTIFY validOsInstallSizeChanged FINAL)
    Q_PROPERTY(bool checked READ checked NOTIFY checkedChanged)
    Q_PROPERTY(QDateTime lastChecked READ lastChecked NOTIFY lastCheckedChanged)

    Q_PROPERTY(ErrorState errorState READ errorState NOTIFY errorStateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

    Q_PROPERTY(bool sufficientSpaceForDownload READ sufficientSpaceForDownload NOTIFY sufficientSpaceForDownloadChanged)
    Q_PROPERTY(bool sufficientSpaceForInstall READ sufficientSpaceForInstall NOTIFY sufficientSpaceForInstallChanged)
    Q_PROPERTY(bool sufficientBatteryForInstall READ sufficientBatteryForInstall NOTIFY sufficientBatteryForInstallChanged)
    Q_PROPERTY(bool batteryChargerConnected READ batteryChargerConnected NOTIFY batteryChargerConnectedChanged)

public:
    enum Status {
        Unknown,
        WaitingForConnection,
        Checking,
        UpToDate,
        UpdateAvailable,
        UpdateDownloaded,
        PreparingForUpdate,
        InstallingUpdate
    };
    Q_ENUM(Status)

    enum ErrorState {
        UnknownErrorState,
        NoNetworkAccess,
        ServiceIsNotAvailable,
        NoAccount,
        BatteryLevelIsTooLow,
        NoFreeSpace,
        ReadOnlyFileSystem,
        UnknownError
    };
    Q_ENUM(ErrorState)

    virtual ~OsUpdater();

    static const QString ServiceName;
    static const QString ServiceObject;
    static const QString ServiceInterface;

    QString backendName() const;
    Status status() const;
    int progress() const;
    QString osVersion() const;
    QString osCodeName() const;
    QString osSummary() const;
    QString osCover() const;
    QString osWebsite() const;
    qlonglong osDownloadSize() const;
    qlonglong osInstallSize() const;
    qlonglong availableSpaceForDownload() const;
    qlonglong availableSpaceForInstall() const;

    bool validOsInstallSize() const;
    bool checked() const;
    QDateTime lastChecked() const;
    ErrorState errorState() const;
    QString errorString() const;

    bool sufficientSpaceForDownload() const;
    bool sufficientSpaceForInstall() const;
    bool sufficientBatteryForInstall() const;
    bool batteryChargerConnected() const;

    Q_INVOKABLE void calculateDiskSpaceRequirements();

public slots:
    // Checks for OS upgrades. If forced, this request will not be ignored on mobile networks.
    virtual void checkForUpdate(bool refreshCache, bool forced) = 0;
    virtual void downloadUpdate(const QString &version) = 0;
    virtual void installDownloadedUpdate(const QString &version) = 0;

signals:
    void statusChanged(Status status);
    void progressChanged(int progress);
    void updateCheckCacheCleared();
    void updateInstallationStarted();
    void accountStatusChanged();

    void osVersionChanged(const QString &osVersion);
    void osCodeNameChanged(const QString &osCodeName);
    void osSummaryChanged(const QString &osSummary);
    void osCoverChanged(const QString &osCover);
    void osWebsiteChanged(const QString &osWebsite);
    void osDownloadSizeChanged(qlonglong osDownloadSize);
    void osInstallSizeChanged(qlonglong osInstallSize);

    void availableSpaceForDownloadChanged(qlonglong availableSpaceForDownload);
    void availableSpaceForInstallChanged(qlonglong availableSpaceForInstall);

    void validOsInstallSizeChanged(bool validOsInstallSize);
    void checkedChanged(bool checked);
    void lastCheckedChanged(const QDateTime &lastChecked);

    void errorStateChanged(ErrorState errorState);
    void errorStringChanged(const QString &errorString);

    void sufficientSpaceForDownloadChanged(bool sufficientSpaceForDownload);
    void sufficientSpaceForInstallChanged(bool sufficientSpaceForInstall);
    void sufficientBatteryForInstallChanged(bool sufficientBatteryForInstall);
    void batteryChargerConnectedChanged(bool batteryChargerConnected);

protected:
    explicit OsUpdater(OsUpdaterPrivate *d, QObject *parent = nullptr);

    void setStatus(Status status);
    void setOsVersion(const QString &osVersion);
    void setOsCodeName(const QString &osCodeName);
    void setOsSummary(const QString &osSummary);
    void setOsCover(const QString &osCover);
    void setOsWebsite(const QString &osWebsite);
    void setProgress(int progress);
    void setOsDownloadSize(qlonglong osDownloadSize);
    void setOsInstallSize(qlonglong osInstallSize);
    void setValidOsInstallSize(bool validOsInstallSize);
    void setChecked(bool checked);
    void setLastChecked(const QDateTime &lastChecked);

    void setErrorState(ErrorState errorState);
    void setErrorString(const QString &errorString);

    OsUpdaterPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OsUpdater)

};
}

#endif
