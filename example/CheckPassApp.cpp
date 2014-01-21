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

#include "CheckPassApp.h"
#include <QAuth>
#include <QDebug>
#include <iostream>
#include <string>

class CheckPass : public QAuth {
    Q_OBJECT
public:
    CheckPass(QObject *parent);

protected:
    virtual QByteArray prompt(const QString &message, bool echo = false);
};

CheckPass::CheckPass(QObject *parent)
        : QAuth(parent) {

}

QByteArray CheckPass::prompt(const QString& message, bool echo)
{
    std::string password;
    std::cout << message.toStdString();
    std::cin >> password;
    return password.c_str();
}


CheckPassApp::CheckPassApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_auth(new CheckPass(this)) {
    //m_auth->setVerbosity(true);
    connect(m_auth, SIGNAL(finished(int)), this, SLOT(quit()));
    m_auth->start();
}

CheckPassApp::~CheckPassApp() {

}

int main(int argc, char** argv) {
    CheckPassApp app(argc, argv);
    return app.exec();
}

#include "CheckPassApp.moc"
