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
#include "config.h"
#if !defined(PAMBACKEND_H) && defined(PAM_FOUND)
#define PAMBACKEND_H

#include "Messages.h"
#include "../Backend.h"

#include <QtCore/QObject>

#include <security/pam_appl.h>

class PamHandle;
class PamBackend : public Backend
{
    Q_OBJECT
public:
    explicit PamBackend(QAuthApp *parent);
    Prompt detectMessage(const struct pam_message *msg);
    Request guessRequest(const struct pam_message *msg, bool *failed);
    Request formatRequest(int n, const struct pam_message **msg, bool *failed);
    void handleResponse(int n, const struct pam_message **msg, struct pam_response *aresp, const Request &response, bool *failed);
    int converse(int n, const struct pam_message **msg, struct pam_response **resp);

public slots:
    virtual bool start(const QString &user = QString());
    virtual bool authenticate();
    virtual bool openSession();

    virtual QString userName();

private:
    PamHandle *m_pam { nullptr };
    Request m_cachedRequest { };
};

#endif // PAMBACKEND_H
