/*
 * Qt Authentication library
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

#ifndef QAUTH_H
#define QAUTH_H

#include <QtCore/QObject>

class QAuth : public QObject
{
    Q_OBJECT
public:
    class Private;
    explicit QAuth(QObject *parent = 0);
    virtual ~QAuth();

    void start();

    void setExecutable(const QString &path);
    void setVerbosity(bool on = true);

signals:
    void finished(int success);

protected:
    virtual QByteArray prompt(const QString &message, bool echo = false);
    virtual void info(const QString &message);
    virtual void error(const QString &message);

private:
    friend Private;
    Private *d { nullptr };
};

#endif // QAUTH_H
