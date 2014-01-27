/*
 * PAM authentication backend
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
#include "PamBackend.h"
#include "PamHandle.h"
#include "app/QAuthApp.h"
#include <app/Session.h>

#include <QtCore/QString>
#include <QDebug>

#include <stdlib.h>

PamBackend::PamBackend(QAuthApp *parent)
        : Backend(parent)
        , m_pam(new PamHandle(this)) {
}

bool PamBackend::start() {
    bool result;

    if (m_app->session()->path().isEmpty())
        result = m_pam->start("qauth-check");
    else if (m_autologin)
        result = m_pam->start("qauth-autologin");
    else
        result = m_pam->start("qauth-login");

    if (!result)
        m_app->error(m_pam->errorString());

    return result;
}

bool PamBackend::authenticate() {
    if (!m_pam->authenticate()) {
        m_app->error(m_pam->errorString());
        return false;
    }
    if (!m_pam->acctMgmt()) {
        m_app->error(m_pam->errorString());
        return false;
    }
    return true;
}

bool PamBackend::openSession() {
    if (!m_pam->setCred(PAM_ESTABLISH_CRED)) {
        m_app->error(m_pam->errorString());
        return false;
    }
    QString display = m_app->session()->processEnvironment().value("DISPLAY");
    if (!display.isEmpty()) {
        m_pam->setItem(PAM_XDISPLAY, display.toLatin1());
        m_pam->setItem(PAM_TTY, display.toLatin1());
    }
    if (!m_pam->openSession()) {
        m_app->error(m_pam->errorString());
        return false;
    }
    return Backend::openSession();
}

int PamBackend::converse(int n, const struct pam_message **msg, struct pam_response **resp) {
    qDebug() << " AUTH: PAM: Conversation...";
    struct pam_response *aresp;

    // check size of the message buffer
    if ((n <= 0) || (n > PAM_MAX_NUM_MSG))
        return PAM_CONV_ERR;

    // create response buffer
    if ((aresp = (struct pam_response *) calloc(n, sizeof(struct pam_response))) == nullptr)
        return PAM_BUF_ERR;

    bool failed = false;

    for (int i = 0; i < n; ++i) {
        aresp[i].resp_retcode = 0;
        aresp[i].resp = nullptr;
        switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_OFF: {
                qDebug() << " AUTH: PAM: Prompt, echo off..." << msg[i]->msg;
                QByteArray response = m_app->prompt(QString(msg[i]->msg), false);
                aresp[i].resp = (char *) malloc(response.length() + 1);
                if (aresp[i].resp == nullptr) {
                    failed = true;
                }
                else {
                    memcpy(aresp[i].resp, response.data(), response.length());
                    aresp[i].resp[response.length()] = '\0';
                }
                break;
            }
            case PAM_PROMPT_ECHO_ON: {
                qDebug() << " AUTH: PAM: Prompt, echo on..." << msg[i]->msg;
                QByteArray response = m_app->prompt(QString(msg[i]->msg), true);
                aresp[i].resp = (char *) malloc(response.length() + 1);
                if (aresp[i].resp == nullptr) {
                    failed = true;
                }
                else {
                    memcpy(aresp[i].resp, response.data(), response.length());
                    aresp[i].resp[response.length()] = '\0';
                }
                break;
            }
            case PAM_ERROR_MSG:
                qDebug() << " AUTH: PAM: Error" << msg[i]->msg;
                m_app->error(QString(msg[i]->msg));
                break;
            case PAM_TEXT_INFO:
                qDebug() << " AUTH: PAM: Info" << msg[i]->msg;
                m_app->info(QString(msg[i]->msg));
                break;
            default:
                failed = true;
        }
    }

    if (failed) {
        for (int i = 0; i < n; ++i) {
            if (aresp[i].resp != nullptr) {
                memset(aresp[i].resp, 0, strlen(aresp[i].resp));
                free(aresp[i].resp);
            }
        }
        memset(aresp, 0, n * sizeof(struct pam_response));
        free(aresp);
        *resp = nullptr;
        return PAM_CONV_ERR;
    }

    *resp = aresp;
    return PAM_SUCCESS;
}

