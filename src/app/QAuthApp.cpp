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
#include "Messages.h"
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
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(newData()));
    QStringList args = QCoreApplication::arguments();
    QString server;
    qint64 id = -1;
    int pos;

    if ((pos = args.indexOf("--socket")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
        }
        server = args[pos + 1];
        args.removeAt(pos);
        args.removeAt(pos);
    }

    if ((pos = args.indexOf("--id")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
        }
        id = QString(args[pos + 1]).toLongLong();
        args.removeAt(pos);
        args.removeAt(pos);
    }

    if ((pos = args.indexOf("--start")) >= 0) {
        if (pos >= args.length() - 1) {
            qCritical() << "This application is not supposed to be executed manually";
            exit(OTHER_ERROR);
        }
        m_sessionPath = args[pos + 1];
        args.removeAt(pos);
        args.removeAt(pos);
    }

    if (server.isEmpty() || id <= 0) {
        qCritical() << "This application is not supposed to be executed manually";
        exit(OTHER_ERROR);
    }

    m_socket->connectToServer(server, QIODevice::ReadWrite | QIODevice::Unbuffered);
    if (!m_socket->waitForConnected())
        qCritical() << "Couldn't connect!";
    QDataStream str(m_socket);
    str << Msg::HELLO << id;
    if (str.status() != QDataStream::Ok)
        qCritical() << "Couldn't write initial message:" << str.status();

    if (!m_backend->authenticate())
        exit(AUTH_ERROR);

    if (!m_sessionPath.isEmpty() && !m_backend->openSession())
        exit(SESSION_ERROR);

    exit(AUTH_SUCCESS);
}

void QAuthApp::error(const QString& message) {
    Msg m;
    QDataStream str(m_socket);
    str << Msg::ERROR << message;
    m_socket->waitForReadyRead();
    str >> m;
    if (m != ERROR)
        qCritical() << "Received a wrong opcode instead of ERROR:" << m;
}

void QAuthApp::info(const QString& message) {
    Msg m;
    QDataStream str(m_socket);
    str << Msg::INFO << message;
    m_socket->waitForReadyRead();
    str >> m;
    if (m != ERROR)
        qCritical() << "Received a wrong opcode instead of INFO:" << m;
}

QByteArray QAuthApp::prompt(const QString& message, bool echo) {
    Msg m;
    QByteArray response;
    QDataStream str(m_socket);
    str << Msg::PROMPT << message << echo;
    m_socket->waitForReadyRead();
    str >> m >> response;
    qDebug() << "Received a response" << response;
    if (m != PROMPT) {
        response = QByteArray();
        qCritical() << "Received a wrong opcode instead of PROMPT:" << m;
    }
    return response;
}

QProcessEnvironment QAuthApp::requestEnvironment() {
    Msg m;
    QProcessEnvironment response;
    QDataStream str(m_socket);
    str << Msg::ENVIRONMENT;
    m_socket->waitForReadyRead();
    str >> m >> response;
    qDebug() << "Received a response" << response.toStringList();
    if (m != ENVIRONMENT) {
        response = QProcessEnvironment();
        qCritical() << "Received a wrong opcode instead of ENVIRONMENT:" << m;
    }
    return response;
}

Session *QAuthApp::session() {
    return m_session;
}

QAuthApp::~QAuthApp() {

}

void QAuthApp::newData() {
    
}

int main(int argc, char** argv) {
    QAuthApp app(argc, argv);
    return app.exec();
}

