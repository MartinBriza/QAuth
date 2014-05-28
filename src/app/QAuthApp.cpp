/*
 * Main authentication application class
 * Copyright (C) 2013 Martin Bříza <mbriza@redhat.com>
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

#include "QAuthApp.h"

#include "Backend.h"
#include "Session.h"
#include "SafeDataStream.h"

#include <QTimer>
#include <QFile>
#include <QLocalSocket>
#include <QDebug>

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

QAuthApp::QAuthApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_backend(Backend::get(this))
        , m_session(new Session(this))
        , m_socket(new QLocalSocket(this)) {
    QTimer::singleShot(0, this, SLOT(setUp()));
}

void QAuthApp::setUp() {
    QStringList args = QCoreApplication::arguments();
    QString server;
    int pos;

    if ((pos = args.indexOf("--socket")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
            return;
        }
        server = args[pos + 1];
    }

    if ((pos = args.indexOf("--id")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
            return;
        }
        m_id = QString(args[pos + 1]).toLongLong();
    }

    if ((pos = args.indexOf("--start")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
            return;
        }
        m_session->setPath(args[pos + 1]);
    }

    if ((pos = args.indexOf("--user")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
            return;
        }
        m_user = args[pos + 1];
    }

    if ((pos = args.indexOf("--autologin")) >= 0) {
        m_backend->setAutologin(true);
    }

    if (server.isEmpty() || m_id <= 0) {
        qCritical() << "This application is not supposed to be executed manually";
        exit(OTHER_ERROR);
        return;
    }

    connect(m_socket, SIGNAL(connected()), this, SLOT(doAuth()));
    connect(m_session, SIGNAL(finished(int)), this, SLOT(sessionFinished(int)));
    m_socket->connectToServer(server, QIODevice::ReadWrite | QIODevice::Unbuffered);
}

void QAuthApp::doAuth() {
    SafeDataStream str(m_socket);
    str << Msg::HELLO << m_id;
    str.send();
    if (str.status() != QDataStream::Ok)
        qCritical() << "Couldn't write initial message:" << str.status();

    if (!m_backend->start(m_user)) {
        exit(AUTH_ERROR);
        return;
    }

    if (!m_backend->authenticate()) {
        authenticated(QString(""));
        exit(AUTH_ERROR);
        return;
    }

    m_user = m_backend->userName();
    QProcessEnvironment env = authenticated(m_user);

    if (!m_session->path().isEmpty()) {
        env.insert(m_session->processEnvironment());
        m_session->setProcessEnvironment(env);

        if (!m_backend->openSession()) {
            sessionOpened(false);
            exit(SESSION_ERROR);
            return;
        }

        sessionOpened(true);
    }
    else
        exit(AUTH_SUCCESS);
    return;
}

void QAuthApp::sessionFinished(int status) {
    exit(status);
}

void QAuthApp::info(const QString& message, QAuth::Info type) {
    SafeDataStream str(m_socket);
    str << Msg::INFO << message << type;
    str.send();
    m_socket->waitForBytesWritten();
}

void QAuthApp::error(const QString& message, QAuth::Error type) {
    SafeDataStream str(m_socket);
    str << Msg::ERROR << message << type;
    str.send();
    m_socket->waitForBytesWritten();
}

Request QAuthApp::request(const Request& request) {
    Msg m = Msg::MSG_UNKNOWN;
    Request response;
    SafeDataStream str(m_socket);
    str << Msg::REQUEST << request;
    str.send();
    str.receive();
    str >> m >> response;
    if (m != REQUEST) {
        response = Request();
        qCritical() << "Received a wrong opcode instead of REQUEST:" << m;
    }
    return response;
}

QProcessEnvironment QAuthApp::authenticated(const QString &user) {
    Msg m = Msg::MSG_UNKNOWN;
    QProcessEnvironment response;
    SafeDataStream str(m_socket);
    str << Msg::AUTHENTICATED << user;
    str.send();
    if (user.isEmpty())
        return response;
    str.receive();
    str >> m >> response;
    if (m != AUTHENTICATED) {
        response = QProcessEnvironment();
        qCritical() << "Received a wrong opcode instead of AUTHENTICATED:" << m;
    }
    return response;
}

void QAuthApp::sessionOpened(bool success) {
    Msg m = Msg::MSG_UNKNOWN;
    SafeDataStream str(m_socket);
    str << Msg::SESSION_STATUS << success;
    str.send();
    str.receive();
    str >> m;
    if (m != SESSION_STATUS) {
        qCritical() << "Received a wrong opcode instead of SESSION_STATUS:" << m;
    }
}

Session *QAuthApp::session() {
    return m_session;
}

const QString& QAuthApp::user() const {
    return m_user;
}

QAuthApp::~QAuthApp() {

}

int main(int argc, char** argv) {
    QAuthApp app(argc, argv);
    return app.exec();
}

