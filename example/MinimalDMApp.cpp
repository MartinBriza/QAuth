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
#include <QFile>
#include <QTimer>
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
        , m_auth(new MinimalDM(this))
        , m_displayServer(new QProcess(this)) {
    m_auth->setVerbosity(true);
    m_auth->setAutologin(true);
    m_auth->setExecutable("/usr/bin/startkde");
    connect(m_auth, SIGNAL(finished(int)), this, SLOT(handleResult(int)));

    QTimer::singleShot(0, this, SLOT(startX()));
    QTimer::singleShot(0, m_auth, SLOT(start()));
}

MinimalDMApp::~MinimalDMApp() {

}

void MinimalDMApp::handleResult(int code) {
    exit(code);
}

void MinimalDMApp::startX() {
    for (int i = 0; ; i++) {
        if (QFile::exists(QString("/tmp/.X%1-lock").arg(i)))
            continue;
        m_displayServer->start("/usr/bin/X", {QString(":%1").arg(i)});
        if (m_displayServer->waitForStarted())
            m_auth->display = QString(":%1").arg(i);
        else
            exit(1);
        break;
    }
}

int main(int argc, char** argv) {
    MinimalDMApp app(argc, argv);
    return app.exec();
}

#include "MinimalDMApp.moc"
