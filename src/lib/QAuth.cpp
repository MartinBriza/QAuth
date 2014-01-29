/*
 * Qt Authentication Library
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

#include "QAuth.h"
#include "Messages.h"
#include "config.h"

#include <QProcess>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>

#include <unistd.h>

class SocketServer : public QLocalServer {
    Q_OBJECT
public slots:
    void incomingConnection();
public:
    static SocketServer *instance();

    QMap<qint64, QAuth::Private*> helpers;
private:
    static SocketServer *self;
    SocketServer();
};

SocketServer *SocketServer::self = nullptr;

class QAuth::Private : public QObject {
    Q_OBJECT
public:
    Private(QAuth *parent);
    void setSocket(QLocalSocket *socket);
public slots:
    void dataPending();
public:
    QProcess *child { nullptr };
    QLocalSocket *socket { nullptr };
    QString sessionPath { };
    bool autologin { false };
    qint64 id { 0 };
    static qint64 lastId;
};

qint64 QAuth::Private::lastId = 1;

SocketServer::SocketServer()
        : QLocalServer() {
    connect(this, SIGNAL(newConnection()), this, SLOT(incomingConnection()));
}

void SocketServer::incomingConnection()  {
    while (hasPendingConnections()) {
        Msg m;
        qint64 id;
        QLocalSocket *socket = nextPendingConnection();
        socket->waitForReadyRead();
        QDataStream str(socket);
        str >> m >> id;
        if (m == Msg::HELLO && id && SocketServer::instance()->helpers.contains(id)) {
            helpers[id]->setSocket(socket);
            if (socket->bytesAvailable() > 0)
                helpers[id]->dataPending();
        }
    }
}

SocketServer* SocketServer::instance() {
    if (!self) {
        self = new SocketServer();
        // TODO until i'm not too lazy to actually hash something
        self->listen(QString("QAuth%1.%2").arg(getpid()).arg(time(NULL)));
    }
    return self;
}

QAuth::Private::Private(QAuth *parent)
        : QObject(parent)
        , child(new QProcess(this))
        , id(lastId++) {
    SocketServer::instance()->helpers[id] = this;
}

void QAuth::Private::setSocket(QLocalSocket *socket) {
    this->socket = socket;
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataPending()));
}

void QAuth::Private::dataPending() {
    QAuth *auth = qobject_cast<QAuth*>(parent());
    Msg m;
    QDataStream str(socket);
    str >> m;
    switch (m) {
        case ERROR: {
            QString message;
            str >> message;
            auth->error(message);
            str << Msg::ERROR;
            socket->flush();
            break;
        }
        case INFO: {
            QString message;
            str >> message;
            auth->info(message);
            str << Msg::INFO;
            socket->flush();
            break;
        }
        case PROMPT: {
            QString message;
            bool echo;
            str >> message >> echo;
            QByteArray response = auth->prompt(message, echo);
            str << Msg::PROMPT << response;
            socket->flush();
            break;
        }
        case ENVIRONMENT: {
            QProcessEnvironment env = auth->provideEnvironment();
            str << Msg::ENVIRONMENT << env;
            socket->flush();
            break;
        }
        default: {
            qWarning() << "QAuth: Unexpected value received:" << m;
        }
    }
}

QAuth::QAuth(QObject* parent)
        : QObject(parent)
        , d(new Private(this)) {

}

QAuth::~QAuth() {

}

void QAuth::setExecutable(const QString& path) {
    d->sessionPath = path;
}

void QAuth::setAutologin(bool on) {
    d->autologin = on;
}

void QAuth::start() {
    QStringList args;
    args << "--socket" << SocketServer::instance()->fullServerName();
    args << "--id" << QString("%1").arg(d->id);
    if (!d->sessionPath.isEmpty())
        args << "--start" << d->sessionPath;
    if (d->autologin)
        args << "--autologin";
    connect(d->child, SIGNAL(finished(int)), this, SIGNAL(finished(int)));
    d->child->start(QAUTH_HELPER_PATH, args);
}

void QAuth::setVerbosity(bool on) {
    if (on)
        d->child->setProcessChannelMode(QProcess::ForwardedChannels);
    else
        d->child->setProcessChannelMode(QProcess::SeparateChannels);
}

void QAuth::error(const QString &message) {
    qWarning() << "QAuth backend error:" << message;
}

void QAuth::info(const QString &message) {
    qDebug() << "QAuth backend info:" << message;
}

QByteArray QAuth::prompt(const QString &message, bool echo) {
    Q_UNUSED(echo);
    qDebug() << "QAuth backend request:" << message << ". Echoing";
    return message.toLatin1();
}

QProcessEnvironment QAuth::provideEnvironment() {
    return QProcessEnvironment();
}

#include "QAuth.moc"
