/*
 * PAM API Qt wrapper
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
#include "PamHandle.h"
#include "PamBackend.h"

#include <QtCore/QDebug>

bool PamHandle::putEnv(const QProcessEnvironment& env) {
    foreach (const QString& s, env.toStringList()) {
        m_result = pam_putenv(m_handle, s.toAscii());
        if (m_result != PAM_SUCCESS) {
            qWarning() << " AUTH: PAM: putEnv:" << pam_strerror(m_handle, m_result);
            return false;
        }
    }
    return true;
}

QProcessEnvironment PamHandle::getEnv() {
    QProcessEnvironment env;
    // get pam environment
    char **envlist = pam_getenvlist(m_handle);
    if (envlist == NULL) {
        qWarning() << " AUTH: PAM: getEnv: Returned NULL";
        return env;
    }

    // copy it to the env map
    for (int i = 0; envlist[i] != nullptr; ++i) {
        QString s(envlist[i]);

        // find equal sign
        int index = s.indexOf('=');

        // add to the hash
        if (index != -1)
            env.insert(s.left(index), s.mid(index + 1));

        free(envlist[i]);
    }
    free(envlist);
    return env;
}

bool PamHandle::chAuthTok(int flags) {
    m_result = pam_chauthtok(m_handle, flags | m_silent);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: chAuthTok:" << pam_strerror(m_handle, m_result);
    }
    return m_result == PAM_SUCCESS;
}

bool PamHandle::acctMgmt(int flags) {
    m_result = pam_acct_mgmt(m_handle, flags | m_silent);
    if (m_result == PAM_NEW_AUTHTOK_REQD) {
        // TODO see if this should really return the value or just true regardless of the outcome
        return chAuthTok(PAM_CHANGE_EXPIRED_AUTHTOK);
    }
    else if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: acctMgmt:" << pam_strerror(m_handle, m_result);
        return false;
    }
    return true;
}

bool PamHandle::authenticate(int flags) {
    qDebug() << " AUTH: PAM: Authenticating...";
    m_result = pam_authenticate(m_handle, flags | m_silent);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: authenticate:" << pam_strerror(m_handle, m_result);
    }
    qDebug() << " AUTH: PAM: returning.";
    return m_result == PAM_SUCCESS;
}

bool PamHandle::setCred(int flags) {
    m_result = pam_setcred(m_handle, flags | m_silent);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: setCred:" << pam_strerror(m_handle, m_result);
    }
    return m_result == PAM_SUCCESS;
}

bool PamHandle::openSession() {
    m_result = pam_open_session(m_handle, m_silent);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: openSession:" << pam_strerror(m_handle, m_result);
    }
    return m_result == PAM_SUCCESS;
}

bool PamHandle::closeSession() {
    m_result = pam_close_session(m_handle, m_silent);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: closeSession:" << pam_strerror(m_handle, m_result);
    }
    return m_result == PAM_SUCCESS;
}

bool PamHandle::setItem(int item_type, const void* item) {
    m_result = pam_set_item(m_handle, item_type, item);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: setItem:" << pam_strerror(m_handle, m_result);
    }
    return m_result == PAM_SUCCESS;
}

const void* PamHandle::getItem(int item_type) {
    const void *item;
    m_result = pam_get_item(m_handle, item_type, &item);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: getItem:" << pam_strerror(m_handle, m_result);
    }
    return item;
}

int PamHandle::converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data) {
    qDebug() << " AUTH: PAM: Preparing to converse...";
    PamBackend *c = static_cast<PamBackend *>(data);
    return c->converse(n, msg, resp);
}

bool PamHandle::start(const char *service_name) {
    m_result = pam_start(service_name, NULL, &m_conv, &m_handle);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: start" << pam_strerror(m_handle, m_result);
        return false;
    }
    else {
        qDebug() << " AUTH: PAM: Starting...";
    }
    return true;
}

bool PamHandle::end(int flags) {
    if (!m_handle)
        return false;
    m_result = pam_end(m_handle, m_result | flags);
    if (m_result != PAM_SUCCESS) {
        qWarning() << " AUTH: PAM: end:" << pam_strerror(m_handle, m_result);
        return false;
    }
    else {
        qDebug() << " AUTH: PAM: Ended.";
    }
    m_handle = NULL;
    return true;
}

QString PamHandle::errorString() {
    return pam_strerror(m_handle, m_result);
}

PamHandle::PamHandle(PamBackend *parent) {
    // create context
    m_conv = { &PamHandle::converse, parent };
}

PamHandle::~PamHandle() {
    // stop service
    end();
}
