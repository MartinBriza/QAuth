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

MinimalDMApp::MinimalDMApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_auth(new QAuth(this))
        , m_displayServer(new QProcess(this)) {
    m_auth->setVerbose(true);
    m_auth->setAutologin(true);
    m_auth->setSession("/usr/bin/lxsession");
    m_auth->insertEnvironment("PATH", "/bin:/usr/bin:/usr/local/bin:/usr/local/sbin:/usr/sbin");

    connect(m_displayServer, SIGNAL(started()), m_auth, SLOT(start()));
    connect(m_auth, SIGNAL(finished(int)), this, SLOT(handleResult(int)));
    connect(m_auth, SIGNAL(request(QAuthRequest*)), this, SLOT(handleRequest(QAuthRequest*)));

    QTimer::singleShot(0, this, SLOT(startX()));
}

MinimalDMApp::~MinimalDMApp() {

}

void MinimalDMApp::handleResult(int code) {
    exit(code);
}

void MinimalDMApp::handleRequest(QAuthRequest* request) {
    Q_FOREACH (QAuthPrompt *p, request->prompts()) {
        p->setResponse("root"); // very safe to autologin as root, eh?
    }
    request->done();
}

void MinimalDMApp::startX() {
    for (int i = 0; ; i++) {
        if (QFile::exists(QString("/tmp/.X%1-lock").arg(i)))
            continue;
        m_displayServer->setProcessChannelMode(QProcess::ForwardedChannels);
        m_displayServer->start("/usr/bin/X", {QString(":%1").arg(i)});
        if (m_displayServer->waitForStarted())
            m_auth->insertEnvironment("DISPLAY", QString(":%1").arg(i));
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
