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

#include <QtDeclarative/QDeclarativeView>

QMLApp::QMLApp(int& argc, char** argv)
        : QApplication(argc, argv) {
    QAuth::registerTypes();
    QDeclarativeView *view = new QDeclarativeView();
    view->setSource(QUrl::fromLocalFile("../example/qmlapp.qml"));
    view->show();
}

QMLApp::~QMLApp() {

}

int main(int argc, char** argv) {
    QMLApp app(argc, argv);
    return app.exec();
}

#include "QMLApp.moc"
