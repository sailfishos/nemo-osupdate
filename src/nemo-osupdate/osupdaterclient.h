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

#ifndef NEMO_OS_UPDATER_CLIENT_H
#define NEMO_OS_UPDATER_CLIENT_H

#include <osupdater.h>
#include <global.h>

#include <QDBusConnection>

namespace Nemo {

class OsUpdaterClientPrivate;

class NEMO_OSUPDATE_EXPORT OsUpdaterClient : public OsUpdater
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid CONSTANT)
public:
    explicit OsUpdaterClient(const QDBusConnection &dbusConnection = QDBusConnection::systemBus(), QObject *parent = nullptr);
    virtual ~OsUpdaterClient();

    bool isValid() const;

public slots:
    void checkForUpdate();
    void downloadUpdate();
    void installDownloadedUpdate();

    void checkForUpdate(bool refreshCache, bool forced) override;
    void downloadUpdate(const QString &version) override;
    void installDownloadedUpdate(const QString &version) override;

protected slots:
    void setInterfaceLastChecked(qlonglong lastCheckedMs);
    void setInterfaceStatus(int newStatus);

private:
    Q_DECLARE_PRIVATE(OsUpdaterClient)

};

}

#endif // SAILFISHOS_CLIENT_UPDATER_INTERFACE_HPP
