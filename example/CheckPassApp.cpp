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

#include "CheckPassApp.h"
#include <QAuth>
#include <QDebug>
#include <iostream>
#include <string>

class CheckPass : public QAuth {
    Q_OBJECT
public:
    CheckPass(QObject *parent) : QAuth(parent) {}
protected:
    virtual QProcessEnvironment provideEnvironment() {
        QProcessEnvironment env;
        env.insert("PATH", "/bin:/usr/bin:/usr/local/bin:/usr/local/sbin:/usr/sbin");
        // not starting a display... yet
//         env.insert("DISPLAY", display->name());
//         env.insert("XAUTHORITY", QString("%1/.Xauthority").arg(pw->pw_dir));
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
        std::string input;
        std::cout << message.toStdString();
        std::cin >> input;
        return input.c_str();
    }
    virtual void info(const QString &message) {
        std::cout << message.toStdString() << std::endl;
    }
    virtual void error(const QString &message) {
        std::cerr << message.toStdString() << std::endl;
    }
};

CheckPassApp::CheckPassApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_auth(new CheckPass(this)) {
    m_auth->setVerbosity(true);
    connect(m_auth, SIGNAL(finished(int)), this, SLOT(handleResult(int)));
    m_auth->start();
}

CheckPassApp::~CheckPassApp() {

}

void CheckPassApp::handleResult(int code) {
    exit(code);
}

int main(int argc, char** argv) {
    CheckPassApp app(argc, argv);
    return app.exec();
}

#include "CheckPassApp.moc"
