/*
 *
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

#ifndef PROMPT_H
#define PROMPT_H

#include <QtCore/QObject>

class QAuth;
class QAuthRequest;
/**
 * \brief
 * One prompt input for the authentication
 *
 * \section description
 * The main, not completely obvious rationale for this class is:
 *
 * \warning Don't use the \ref message property if you have your own strings for
 *      the \ref Type -s. PAM sends horrible horrible stuff and passwd obviously
 *      doesn't tell us a thing.
 *
 * \todo maybe I should remove the protected setters and move the private classes 
 * into headers I can access from other classes
 */
class QAuthPrompt : public QObject {
    Q_OBJECT
    Q_ENUMS(Type)
    Q_PROPERTY(Type type READ type)
    Q_PROPERTY(QString message READ message)
    Q_PROPERTY(bool hidden READ hidden)
    Q_PROPERTY(QByteArray response READ response WRITE setResponse NOTIFY responseChanged)
public:
    QAuthPrompt(QAuthRequest *parent = 0);
    /**
     * \note In hex not for binary operations but to leave space for adding other codes
     */
    enum Type {
        NONE = 0x0000,
        UNKNOWN = 0x0001,
        CHANGE_CURRENT = 0x0010,
        CHANGE_NEW,
        CHANGE_REPEAT,
        LOGIN_USER = 0x0080,
        LOGIN_PASSWORD
    };
    Type type() const;
    QString message() const;
    bool hidden() const;
    QByteArray response() const;
    void setResponse(const QByteArray &r);
Q_SIGNALS:
    void responseChanged();
public:
    class Private;
    Private *d { nullptr };
};

#endif //PROMPT_H