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

#include <iostream>
#include <string>

#include <termio.h>

CheckPassApp::CheckPassApp(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_auth(new QAuth(this)) {
    m_auth->setVerbose(true);
    connect(m_auth, SIGNAL(finished(bool)), this, SLOT(handleResult(bool)));
    connect(m_auth, SIGNAL(requestChanged()), this, SLOT(handleRequest()));
    connect(m_auth, SIGNAL(error(QString)), this, SLOT(displayError(QString)));
    m_auth->request()->setFinishAutomatically(true);
    m_auth->start();
}

CheckPassApp::~CheckPassApp() {

}

void CheckPassApp::displayError(QString message) {
    std::cerr << "Error: " << message.toStdString() << std::endl;
}

void CheckPassApp::handleResult(bool status) {
    exit(!status);
}

void CheckPassApp::handleRequest() {
    struct termio tty;
    unsigned short flags;

    if (!m_auth->request()->info().isEmpty())
        std::cout << "Info: " << m_auth->request()->info().toStdString() << std::endl;

    Q_FOREACH (QAuthPrompt *p, m_auth->request()->prompts()) {
        std::string response;
        std::cout << "Prompt: " << p->message().toStdString();

        if (p->hidden()) {
            ioctl(fileno(stdin), TCGETA, &tty);
            flags = tty.c_lflag;
            tty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
            ioctl(fileno(stdin), TCSETAF, &tty);
        }

        std::cin >> response;
        p->setResponse(response.c_str());

        if (p->hidden()) {
            tty.c_lflag = flags;
            ioctl(fileno(stdin), TCSETAW, &tty);
            fputc('\n', stdout);
        }
    }
}

int main(int argc, char** argv) {
    CheckPassApp app(argc, argv);
    return app.exec();
}

#include "CheckPassApp.moc"
