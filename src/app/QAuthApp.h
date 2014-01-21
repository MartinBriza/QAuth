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

#ifndef QAuth_H
#define QAuth_H

#include <QtCore/QCoreApplication>

class Backend;
class QLocalSocket;
class QSocketNotifier;
class QAuthApp : public QCoreApplication
{
    Q_OBJECT
public:
    QAuthApp(int& argc, char** argv);
    virtual ~QAuthApp();

public slots:
    QByteArray prompt(const QString &message, bool echo);
    void info(const QString &message);
    void error(const QString &message);

private slots:
    void newData();

private:
    Backend *m_backend { nullptr };
    QLocalSocket *m_socket { nullptr };
    QString m_sessionPath { };
};

#endif // QAuth_H