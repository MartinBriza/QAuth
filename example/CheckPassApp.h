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

#ifndef CHECKPASSAPP_H
#define CHECKPASSAPP_H

#include <QtCore/QCoreApplication>
#include <QAuth>

class CheckPassApp : public QCoreApplication
{
    Q_OBJECT
public:
    CheckPassApp(int& argc, char** argv);
    virtual ~CheckPassApp();
private:
    void setInput(bool visible);
    QAuth *m_auth;
private slots:
    void handleResult(bool status);
    void handleRequest();
    void displayError(QString message);
};

#endif // CHECKPASSAPP_H
