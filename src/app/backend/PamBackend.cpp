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
    {   { QAuthPrompt::LOGIN_USER, "login:", false },
        { QAuthPrompt::LOGIN_PASSWORD, "Password: ", true }
    }
};

static Request changePassRequest {
    {   { QAuthPrompt::CHANGE_CURRENT, "(current) UNIX password: ", true },
        { QAuthPrompt::CHANGE_NEW, "New password: ", true },
        { QAuthPrompt::CHANGE_REPEAT, "Retype new password: ", true }
    }
};

static Prompt invalidPrompt {};

PamData::PamData() { }

QAuthPrompt::Type PamData::detectPrompt(const struct pam_message* msg) const {
    if (msg->msg_style == PAM_PROMPT_ECHO_OFF) {
        QString message(msg->msg);
        if (message.indexOf(QRegExp("\\bpassword\\b", Qt::CaseInsensitive)) >= 0) {
            if (message.indexOf(QRegExp("\\b(re-?(enter|type)|again|confirm|repeat)\\b", Qt::CaseInsensitive)) >= 0) {
                return QAuthPrompt::CHANGE_REPEAT;
            }
            else if (message.indexOf(QRegExp("\\bnew\\b", Qt::CaseInsensitive)) >= 0) {
                return QAuthPrompt::CHANGE_NEW;
            }
            else if (message.indexOf(QRegExp("\\b(old|current)\\b", Qt::CaseInsensitive)) >= 0) {
                return QAuthPrompt::CHANGE_CURRENT;
            }
            else {
                return QAuthPrompt::LOGIN_PASSWORD;
            }
        }
    }
    else {
        return QAuthPrompt::LOGIN_USER;
    }

    return QAuthPrompt::UNKNOWN;
}

const Prompt& PamData::findPrompt(const struct pam_message* msg) const {
    QAuthPrompt::Type type = detectPrompt(msg);

    for (const Prompt &p : m_prompts) {
        if (type == p.type && p.message == msg->msg)
            return p;
    }

    return invalidPrompt;
}

Prompt& PamData::findPrompt(const struct pam_message* msg) {
    QAuthPrompt::Type type = detectPrompt(msg);

    for (Prompt &p : m_prompts) {
        if (type == QAuthPrompt::UNKNOWN && msg->msg == p.message)
            return p;
        if (type == p.type)
            return p;
    }

    return invalidPrompt;
}

void PamData::insertPrompt(const struct pam_message* msg, bool predict) {
    Prompt &p = findPrompt(msg);
    if (p.valid()) {
        for (const Prompt &stored : m_currentRequest.prompts) {
            if (stored.type == p.type && stored.message == p.message)
                return;
        }
        p.message = msg->msg;
        if (p.response.isEmpty())
            m_currentRequest.prompts.append(p);
    }
    else {
        if (predict) {
            if (detectPrompt(msg) == QAuthPrompt::LOGIN_USER)
                m_currentRequest = Request(loginRequest);
            else if (detectPrompt(msg) == QAuthPrompt::CHANGE_CURRENT)
                m_currentRequest = Request(changePassRequest);
            m_prompts.append(m_currentRequest.prompts);
        }
        else {
            m_prompts.append(Prompt(detectPrompt(msg), msg->msg, msg->msg_style == PAM_PROMPT_ECHO_OFF));
            m_currentRequest.prompts.append(m_prompts.last());
        }
    }

}

void PamData::insertInfo(const struct pam_message* msg) {
    if (QString(msg->msg).indexOf(QRegExp("^Changing password for [^ ]+$"))) {
        m_currentRequest = Request(changePassRequest);
        m_prompts.append(m_currentRequest.prompts);
    }
}

const QByteArray& PamData::getResponse(const struct pam_message* msg) const {
    return findPrompt(msg).response;
}

const Request& PamData::getRequest() const {
    return m_currentRequest;
}

void PamData::completeRequest(const Request& request) {
    m_currentRequest.clear();
    for (const Prompt &newPrompt : request.prompts) {
        bool found = false;
        for (Prompt &oldPrompt : m_prompts) {
            // if they are the same, save the response
            if (oldPrompt.type == newPrompt.type &&
                oldPrompt.message == newPrompt.message &&
                oldPrompt.hidden == newPrompt.hidden) {
                oldPrompt.response = newPrompt.response;
                found = true;
                break;
            }
        }
        if (!found)
            m_prompts.append(newPrompt);
    }
}




PamBackend::PamBackend(QAuthApp *parent)
        : Backend(parent)
        , m_data(new PamData())
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
        m_pam->setItem(PAM_XDISPLAY, qPrintable(display));
        m_pam->setItem(PAM_TTY, qPrintable(display));
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

int PamBackend::converse(int n, const struct pam_message **msg, struct pam_response **resp) {
    qDebug() << " AUTH: PAM: Conversation with" << n << "messages";

    if (n <= 0 || n > PAM_MAX_NUM_MSG)
        return PAM_CONV_ERR;

    for (int i = 0; i < n; i++) {
        switch(msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_OFF:
            case PAM_PROMPT_ECHO_ON:
                m_data->insertPrompt(msg[i], n == 1);
                break;
            case PAM_ERROR_MSG:
                m_app->error(msg[i]->msg);
                break;
            case PAM_TEXT_INFO:
                // if there's only the info message, let's predict the prompts too
                if (n == 1)
                    m_data->insertInfo(msg[i]);
                m_app->info(msg[i]->msg);
                break;
            default:
                break;
        }
    }

    Request sent = m_data->getRequest();
    Request received;

    if (sent.valid()) {
        received = m_app->request(sent);

        if (!received.valid())
            return PAM_CONV_ERR;

        m_data->completeRequest(received);
    }

    *resp = (struct pam_response *) calloc(n, sizeof(struct pam_response));
    if (!*resp) {
        return PAM_BUF_ERR;
    }

    for (int i = 0; i < n; i++) {
        const QByteArray &response = m_data->getResponse(msg[i]);

        resp[i]->resp = (char *) malloc(response.length() + 1);
        // on error, get rid of everything
        if (!resp[i]->resp) {
            for (int j = 0; j < n; j++) {
                free(resp[i]->resp);
                resp[i]->resp = nullptr;
            }
            free(*resp);
            *resp = nullptr;
            return PAM_BUF_ERR;
        }

        memcpy(resp[i]->resp, response.constData(), response.length());
        resp[i]->resp[response.length()] = '\0';
    }

    return PAM_SUCCESS;
}
