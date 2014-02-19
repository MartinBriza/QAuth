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

#include <QTimer>
#include <QFile>
#include <QDataStream>
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

    if ((pos = args.indexOf("--autologin")) >= 0) {
        m_backend->setAutologin(true);
    }

    if (server.isEmpty() || m_id <= 0) {
        qCritical() << "This application is not supposed to be executed manually";
        exit(OTHER_ERROR);
        return;
    }

    connect(m_socket, SIGNAL(connected()), this, SLOT(doAuth()));
    m_socket->connectToServer(server, QIODevice::ReadWrite | QIODevice::Unbuffered);
}

void QAuthApp::doAuth() {
    QDataStream str(m_socket);
    str << Msg::HELLO << m_id;
    if (str.status() != QDataStream::Ok)
        qCritical() << "Couldn't write initial message:" << str.status();

    if (!m_backend->start()) {
        exit(AUTH_ERROR);
        return;
    }

    if (!m_backend->authenticate()) {
        exit(AUTH_ERROR);
        return;
    }

    QString user = m_backend->userName();
    QProcessEnvironment env = authenticated(user);
    m_session->setUser(user);

    if (!m_session->path().isEmpty()) {
        m_session->setProcessEnvironment(env);
    }

    if (!m_backend->openSession()) {
        exit(SESSION_ERROR);
        return;
    }

    sessionOpened();
}

void QAuthApp::error(const QString& message) {
    QDataStream str(m_socket);
    str << Msg::ERROR << message;
    m_socket->waitForBytesWritten();
}

Request QAuthApp::request(const Request& request) {
    Msg m;
    Request response;
    QDataStream str(m_socket);
    str << Msg::REQUEST << request;
    m_socket->waitForBytesWritten();
    m_socket->waitForReadyRead(-1);
    str >> m >> response;
    qDebug() << "Received a response for a request";
    if (m != REQUEST) {
        response = Request();
        qCritical() << "Received a wrong opcode instead of REQUEST:" << m;
    }
    return response;
}

QProcessEnvironment QAuthApp::authenticated(const QString &user) {
    Msg m;
    QProcessEnvironment response;
    QDataStream str(m_socket);
    str << Msg::AUTHENTICATED << user;
    m_socket->waitForBytesWritten();
    m_socket->waitForReadyRead(-1);
    str >> m >> response;
    qDebug() << "Received a response" << response.toStringList();
    if (m != AUTHENTICATED) {
        response = QProcessEnvironment();
        qCritical() << "Received a wrong opcode instead of ENVIRONMENT:" << m;
    }
    return response;
}

void QAuthApp::sessionOpened() {
    QDataStream str(m_socket);
    str << Msg::AUTHENTICATED;
    m_socket->waitForBytesWritten();
}

Session *QAuthApp::session() {
    return m_session;
}

QAuthApp::~QAuthApp() {

}

int main(int argc, char** argv) {
    QAuthApp app(argc, argv);
    return app.exec();
}

