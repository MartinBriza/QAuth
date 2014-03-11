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

#ifndef MINIMALDMAPP_H
#define MINIMALDMAPP_H

#include <QAuth>

#if QT_VERSION >= 0x050000
# include <QtGui/QGuiApplication>
#else
# include <QApplication>
# define QGuiApplication QApplication
#endif

class QMLApp : public QGuiApplication
{
    Q_OBJECT
public:
    QMLApp(int& argc, char** argv);
    virtual ~QMLApp();
};

#endif // MINIMALDMAPP_H
