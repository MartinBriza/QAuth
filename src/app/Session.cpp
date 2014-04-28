/*
 * Session process wrapper
 * Copyright (C) 2014 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "Session.h"
#include "QAuthApp.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

Session::Session(QAuthApp *parent)
        : QProcess(parent) {
    setProcessChannelMode(QProcess::ForwardedChannels);
}

Session::~Session() {

}

bool Session::start() {
    QProcess::start("/etc/X11/xinit/Xsession", {m_path});
    return waitForStarted();
}

void Session::setPath(const QString& path) {
    m_path = path;
}

QString Session::path() const {
    return m_path;
}

void Session::bail(int status) {
    emit finished(status, QProcess::NormalExit);
    exit(status);
}

void Session::setupChildProcess() {
    struct passwd *pw = getpwnam(qobject_cast<QAuthApp*>(parent())->user().toLocal8Bit());
    if (setgid(pw->pw_gid) != 0)
        bail(2);
    initgroups(pw->pw_name, pw->pw_gid);
    if (setuid(pw->pw_uid) != 0)
        bail(2);
    chdir(pw->pw_dir);
}

#include "Session.moc"
