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

Session::Session(QAuthApp *parent)
        : QProcess(parent) {

}

Session::~Session() {

}

void Session::start() {
    QProcess::start(m_path);
}

void Session::setPath(const QString& path) {
    m_path = path;
}

void Session::setUser(const QString& user) {
    m_user = user;
}

void Session::setupChildProcess() {
    // setuid and stuff like that
}

#include "Session.moc"
