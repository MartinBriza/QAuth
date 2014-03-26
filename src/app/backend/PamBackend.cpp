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
#include "app/Session.h"

#include "lib/qauth.h"

#include <QtCore/QString>
#include <QDebug>

#include <stdlib.h>

static Request loginRequest {
    {},
    {   { QAuthPrompt::LOGIN_USER, "login:", false },
        { QAuthPrompt::LOGIN_PASSWORD, "Password: ", true }
    }
};

static Request changePassRequest {
    { "Changing password" },
    {   { QAuthPrompt::CHANGE_CURRENT, "(current) UNIX password: ", true },
        { QAuthPrompt::CHANGE_NEW, "New password: ", true },
        { QAuthPrompt::CHANGE_REPEAT, "Retype new password: ", true }
    }
};

PamBackend::PamBackend(QAuthApp *parent)
        : Backend(parent)
        , m_pam(new PamHandle(this)) {
}

bool PamBackend::start(const QString &user) {
    bool result;

    if (m_app->session()->path().isEmpty())
        result = m_pam->start("qauth-check", user);
    else if (m_autologin)
        result = m_pam->start("qauth-autologin", user);
    else
        result = m_pam->start("qauth-login", user);

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

QString PamBackend::userName() {
    return (const char*) m_pam->getItem(PAM_USER);
}

Prompt PamBackend::detectMessage(const struct pam_message* msg) {
    Prompt p;
    p.hidden = msg->msg_style == PAM_PROMPT_ECHO_OFF;
    p.message = msg->msg;

    if (msg->msg_style == PAM_PROMPT_ECHO_OFF) {
        if (p.message.indexOf(QRegExp("\\bpassword\\b", Qt::CaseInsensitive)) >= 0) {
            if (p.message.indexOf(QRegExp("\\b(re-?(enter|type)|again|confirm|repeat)\\b", Qt::CaseInsensitive)) >= 0) {
                p.type = QAuthPrompt::CHANGE_REPEAT;
            }
            else if (p.message.indexOf(QRegExp("\\bnew\\b", Qt::CaseInsensitive)) >= 0) {
                p.type = QAuthPrompt::CHANGE_NEW;
            }
            else if (p.message.indexOf(QRegExp("\\b(old|current)\\b", Qt::CaseInsensitive)) >= 0) {
                p.type = QAuthPrompt::CHANGE_CURRENT;
            }
            else {
                p.type = QAuthPrompt::LOGIN_PASSWORD;
            }
        }
    }
    else {
        p.type = QAuthPrompt::LOGIN_USER;
    }

    return p;
}

Request PamBackend::guessRequest(const struct pam_message* msg, bool *failed) {
    Request r;
    if (msg->msg_style == PAM_TEXT_INFO) {
        if (QString(msg->msg).indexOf(QRegExp("^Changing password for [^ ]+$"))) {
            r = changePassRequest;
            r.info = QString(msg->msg);
        }
    }
    else if (msg->msg_style == PAM_PROMPT_ECHO_OFF || msg->msg_style == PAM_PROMPT_ECHO_ON) {
        Prompt base = detectMessage(msg);
        switch (base.type) {
            case QAuthPrompt::LOGIN_USER:
                r = loginRequest;
                break;
            case QAuthPrompt::LOGIN_PASSWORD:
                r = Request({}, {base});
                break;
            case QAuthPrompt::CHANGE_REPEAT:
            case QAuthPrompt::CHANGE_NEW:
            case QAuthPrompt::CHANGE_CURRENT:
                r = changePassRequest;
                break;
            default:
                *failed = true;
        }
    }
    else {
        *failed = true;
    }
    return r;
}

Request PamBackend::formatRequest(int n, const struct pam_message** msg, bool *failed) {
    Request r;
    if (n == 1 && msg[0]->msg_style != PAM_ERROR_MSG)
        guessRequest(msg[0], failed);
    else {
        for (int i = 0; i < n; ++i) {
            if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF || msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
                r.prompts << detectMessage(msg[i]);
            }
            else if (msg[i]->msg_style == PAM_TEXT_INFO) {
                qDebug() << " AUTH: PAM: Info" << msg[i]->msg;
                r.info = msg[i]->msg;
            }
            else if (msg[i]->msg_style == PAM_ERROR_MSG) {
                qDebug() << " AUTH: PAM: Error" << msg[i]->msg;
                m_app->error(QString(msg[i]->msg));
                return Request();
            }
            else {
                *failed = true;
                return Request();
            }
        }
    }
    return r;
}

void PamBackend::handleResponse(int n, const struct pam_message **msg, pam_response* aresp, const Request& response, bool *failed) {
    auto it = response.prompts.begin();
    for (int i = 0; i < n; ++i) {
        if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF || msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
            QByteArray data = (*it).response;
            size_t length = (data.length() > PAM_MAX_RESP_SIZE - 1 ? PAM_MAX_RESP_SIZE : data.length());
            aresp[i].resp = (char *) malloc(length + 1);
            if (aresp[i].resp == nullptr) {
                *failed = true;
                break;
            }
            else {
                memcpy(aresp[i].resp, data.data(), length);
                aresp[i].resp[data.length()] = '\0';
            }
            it++;
        }
    }
}

int PamBackend::converse(int n, const struct pam_message **msg, struct pam_response **resp) {
    qDebug() << " AUTH: PAM: Conversation..." << n;
    struct pam_response *aresp;

    // check size of the message buffer
    if ((n <= 0) || (n > PAM_MAX_NUM_MSG))
        return PAM_CONV_ERR;

    // create response buffer
    if ((aresp = (struct pam_response *) calloc(n, sizeof(struct pam_response))) == nullptr)
        return PAM_BUF_ERR;

    bool failed = false;

    Request request = formatRequest(n, msg, &failed);
    if (!failed) {
        Request response = m_app->request(request);
        if (response.prompts.length() == request.prompts.length())
            handleResponse(n, msg, aresp, response, &failed);
        else
            failed = true;
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

