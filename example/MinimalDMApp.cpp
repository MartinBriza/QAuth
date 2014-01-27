/*
 * Simple password checking app using QAuth
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

#include "MinimalDMApp.h"
#include <QAuth>
#include <QDebug>
#include <iostream>
#include <string>

class MinimalDM : public QAuth {
    Q_OBJECT
public:
    MinimalDM(QObject *parent) : QAuth(parent) {}
    QString display { };
protected:
    virtual QProcessEnvironment provideEnvironment() {
        QProcessEnvironment env;
        env.insert("PATH", "/bin:/usr/bin:/usr/local/bin:/usr/local/sbin:/usr/sbin");
        env.insert("DISPLAY", display);
        // if I only knew how to emulate this, let's hope it will work nevertheless
//         env.insert("XDG_SEAT", seat->name());
//         env.insert("XDG_SEAT_PATH", daemonApp->displayManager()->seatPath(seat->name()));
//         env.insert("XDG_SESSION_PATH", daemonApp->displayManager()->sessionPath(process->name()));
//         env.insert("XDG_VTNR", QString::number(display->terminalId()));
        env.insert("DESKTOP_SESSION", "testsession");
        env.insert("GDMSESSION", "testsession");
        return env;
    }
    virtual QByteArray prompt(const QString &message, bool echo = false) {
        return "root"; // very safe to autologin as root, eh?
    }
};

MinimalDMApp::MinimalDMApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_auth(new MinimalDM(this)) {
    m_auth->setVerbosity(true);
    m_auth->setAutologin(true);
    m_auth->setExecutable("/usr/bin/startkde");
    connect(m_auth, SIGNAL(finished(int)), this, SLOT(handleResult(int)));
    m_auth->start();
}

MinimalDMApp::~MinimalDMApp() {

}

void MinimalDMApp::handleResult(int code) {
    exit(code);
}

int main(int argc, char** argv) {
    MinimalDMApp app(argc, argv);
    return app.exec();
}

#include "MinimalDMApp.moc"
