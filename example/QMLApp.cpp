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

#include "QMLApp.h"

#include <QFile>

#if QT_VERSION >= 0x050000
# include <QtQuick/QQuickView>
#else
# include <QtDeclarative/QDeclarativeView>
# define QQuickView QDeclarativeView
#endif

QMLApp::QMLApp(int& argc, char** argv)
        : QGuiApplication(argc, argv) {
    QAuth::registerTypes();
    QQuickView *view = new QQuickView();
    // make my life easier for testing
    if (QFile::exists("qmlapp.qml"))
        view->setSource(QUrl::fromLocalFile("qmlapp.qml"));
    else
        view->setSource(QUrl::fromLocalFile("example/qmlapp.qml"));
    view->show();
}

QMLApp::~QMLApp() {

}

int main(int argc, char** argv) {
    QMLApp app(argc, argv);
    return app.exec();
}

#include "QMLApp.moc"
