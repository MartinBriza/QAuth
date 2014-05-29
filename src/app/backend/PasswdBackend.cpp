/*
 * /etc/passwd authentication backend
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

#include "PasswdBackend.h"

#include "Messages.h"
#include "../QAuthApp.h"

#include <QtCore/QDebug>

#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <crypt.h>

PasswdBackend::PasswdBackend(QAuthApp* parent)
        : Backend(parent) { }

bool PasswdBackend::authenticate() {
    if (m_autologin)
        return true;

    Request r;
    QString password;

    if (m_user.isEmpty())
        r.prompts << Prompt(QAuthPrompt::LOGIN_USER, "Login", false);
    r.prompts << Prompt(QAuthPrompt::LOGIN_PASSWORD, "Password", true);

    Request response = m_app->request(r);
    Q_FOREACH(const Prompt &p, response.prompts) {
        switch (p.type) {
            case QAuthPrompt::LOGIN_USER:
                m_user = p.response;
                break;
            case QAuthPrompt::LOGIN_PASSWORD:
                password = p.response;
                break;
            default:
                break;
        }
    }

    struct passwd *pw = getpwnam(qPrintable(m_user));
    if (!pw) {
        m_app->error(QString("Wrong user/password combination"), QAuth::ERROR_AUTHENTICATION);
        return false;
    }

    struct spwd *spw = getspnam(pw->pw_name);
    if (!spw) {
        qWarning() << " QAuth: Shadow: Could get passwd but not shadow";
        return false;
    }

    if(!spw->sp_pwdp || !spw->sp_pwdp[0])
        return true;

    char *crypted = crypt(qPrintable(password), spw->sp_pwdp);
    if (0 == strcmp(crypted, spw->sp_pwdp)) {
        return true;
    }

    m_app->error(QString("Wrong user/password combination"), QAuth::ERROR_AUTHENTICATION);
    return false;
}

bool PasswdBackend::start(const QString& user) {
    m_user = user;
    return true;
}

QString PasswdBackend::userName() {
    return m_user;
}

#include "PasswdBackend.moc"