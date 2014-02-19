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

#include "qauth.h"
#include "Messages.h"
#include "config.h"

#include <QProcess>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QtDeclarative/QDeclarativeTypeInfo>

#include <unistd.h>

class QAuth::SocketServer : public QLocalServer {
    Q_OBJECT
public slots:
    void incomingConnection();
public:
    static SocketServer *instance();

    QMap<qint64, QAuth::Private*> helpers;
private:
    static QAuth::SocketServer *self;
    SocketServer();
};

QAuth::SocketServer *QAuth::SocketServer::self = nullptr;

class QAuth::Private : public QObject {
    Q_OBJECT
public:
    Private(QAuth *parent);
    void setSocket(QLocalSocket *socket);
public slots:
    void dataPending();
    void childExited(int exitCode, QProcess::ExitStatus exitStatus);
    void childError(QProcess::ProcessError error);
    void requestFinished();
public:
    QAuthRequest *request { nullptr };
    QProcess *child { nullptr };
    QLocalSocket *socket { nullptr };
    QString sessionPath { };
    QString user { };
    bool autologin { false };
    QProcessEnvironment environment { };
    qint64 id { 0 };
    static qint64 lastId;
};

qint64 QAuth::Private::lastId = 1;



QAuth::SocketServer::SocketServer()
        : QLocalServer() {
    connect(this, SIGNAL(newConnection()), this, SLOT(incomingConnection()));
}

void QAuth::SocketServer::incomingConnection()  {
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

QAuth::SocketServer* QAuth::SocketServer::instance() {
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
    connect(child, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(childExited(int,QProcess::ExitStatus)));
    connect(child, SIGNAL(error(QProcess::ProcessError)), this, SLOT(childError(QProcess::ProcessError)));
}

void QAuth::Private::setSocket(QLocalSocket *socket) {
    this->socket = socket;
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataPending()));
}

void QAuth::Private::dataPending() {
    QAuth *auth = qobject_cast<QAuth*>(parent());
    Msg m;
    QDataStream str(socket);
    while (socket->bytesAvailable()) {
        str >> m;
        switch (m) {
            case ERROR: {
                QString message;
                str >> message;
                auth->error(message);
                break;
            }
            case REQUEST: {
                Request r;
                str >> r;
                request = new QAuthRequest(&r, auth);
                connect(request, SIGNAL(finished()), this, SLOT(requestFinished()));
                emit auth->request(request);
                break;
            }
            case ENVIRONMENT: {
                emit auth->environmentRequested();
                str << Msg::ENVIRONMENT << environment;
                socket->waitForBytesWritten();
                break;
            }
            case AUTHENTICATED: {
                QString user;
                str >> user;
                emit auth->authentication(user, true);
                break;
            }
            case SESSION_OPENED: {
                emit auth->session(true);
                break;
            }
            default: {
                emit auth->error(QString("QAuth: Unexpected value received: %1").arg(m));
            }
        }
    }
}

void QAuth::Private::childExited(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit)
        emit qobject_cast<QAuth*>(parent())->finished(exitCode);
    else
        emit qobject_cast<QAuth*>(parent())->error(child->errorString());
}

void QAuth::Private::childError(QProcess::ProcessError error) {
    Q_UNUSED(error);
    emit qobject_cast<QAuth*>(parent())->error(child->errorString());
}

void QAuth::Private::requestFinished() {
    QDataStream str(socket);
    str << REQUEST << request->request();
    socket->waitForBytesWritten();
}

QAuth::QAuth(const QString &user, const QString &session, bool autologin, QObject *parent, bool verbose)
        : QObject(parent)
        , d(new Private(this)) {
    setUser(user);
    setAutologin(autologin);
    setSession(session);
    setVerbose(verbose);
}

QAuth::QAuth(QObject* parent)
        : QObject(parent)
        , d(new Private(this)) {

}

QAuth::~QAuth() {
    delete d;
}

bool QAuth::autologin() const {
    return d->autologin;
}

QString QAuth::session() const {
    return d->sessionPath;
}

QString QAuth::user() const {
    return d->user;
}

bool QAuth::verbose() const {
    return d->child->processChannelMode() == QProcess::ForwardedChannels;
}

void QAuth::insertEnvironment(const QProcessEnvironment &env) {
    d->environment.insert(env);
}

void QAuth::insertEnvironment(const QString &key, const QString &value) {
    d->environment.insert(key, value);
}

void QAuth::setUser(const QString &user) {
    d->user = user;
}

void QAuth::setAutologin(bool on) {
    d->autologin = on;
}

void QAuth::setSession(const QString& path) {
    d->sessionPath = path;
}

void QAuth::setVerbose(bool on) {
    if (on)
        d->child->setProcessChannelMode(QProcess::ForwardedChannels);
    else
        d->child->setProcessChannelMode(QProcess::SeparateChannels);
}

void QAuth::start() {
    QStringList args;
    args << "--socket" << SocketServer::instance()->fullServerName();
    args << "--id" << QString("%1").arg(d->id);
    if (!d->sessionPath.isEmpty())
        args << "--start" << d->sessionPath;
    if (!d->user.isEmpty())
        args << "--user" << d->user;
    if (d->autologin)
        args << "--autologin";
    d->child->start(QAUTH_HELPER_PATH, args);
}

#include "QAuth.moc"
#include "moc_qauth.moc"
