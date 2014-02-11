/*
 * Qt Authentication library
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

#ifndef REQUEST_H
#define REQUEST_H

#include <QtCore/QObject>

class QAuth;
class QAuthPrompt;
class Request;
/**
 * \brief
 * QAuthRequest is the main class for tracking requests from the underlying auth stack
 *
 * \section description
 * Typically, when logging in, you'll receive a list containing one or two fields:
 *
 *  * First one for the username (if you didn't provide it before);
 *    hidden = false, type = LOGIN_USER, message = whatever the stack provides
 *
 *  * Second one for the user's password
 *    hidden = true, type = LOGIN_PASSWORD, message = whatever the stack provides
 *
 * It's up to you to fill the \ref QAuthPrompt::response property.
 * When all the fields are filled to your satisfaction, just trigger the \ref done 
 * slot and the response will go back to the authenticator.
 *
 * \todo Decide if it's sane to use the info messages from PAM or to somehow parse them
 * and make the password changing message into a Request::Type of some kind
 */
class QAuthRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString info READ info)
    Q_PROPERTY(QList<QAuthPrompt*> prompts READ prompts)
public:
    QString info() const;
    QList<QAuthPrompt*> prompts() const;
public Q_SLOTS:
    void done();
Q_SIGNALS:
    void finished();
private:
    QAuthRequest(Request *request, QAuth *parent = 0);
    friend class QAuth;
    class Private;
    Private *d { nullptr };
};

#endif //REQUEST_H