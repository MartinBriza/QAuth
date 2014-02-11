/*
 * Message IDs to pass between the library and the helper
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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <QDataStream>
#include <QProcessEnvironment>

#include "lib/qauth.h"

class Prompt {
public:
    QAuthPrompt::Type type;
    QByteArray response;
    QString message;
    bool hidden;
};

class Request {
public:
    QString info;
    QList<Prompt> prompts;
};

enum Msg {
    HELLO = 1,
    ERROR,
    REQUEST,
    ENVIRONMENT,
    AUTHENTICATED,
    SESSION_OPENED,
};

inline QDataStream& operator<<(QDataStream &s, const Msg &m) {
    s << qint32(m);
    return s;
}

inline QDataStream& operator>>(QDataStream &s, Msg &m) {
    // TODO seriously?
    qint32 i;
    s >> i;
    m = Msg(i);
    return s;
}

inline QDataStream& operator<<(QDataStream &s, const QProcessEnvironment &m) {
    s << m.toStringList();
    return s;
}

inline QDataStream& operator>>(QDataStream &s, QProcessEnvironment &m) {
    QStringList l;
    s >> l;
    for (QString s : l) {
        int pos = s.indexOf('=');
        m.insert(s.left(pos), s.mid(pos + 1));
    }
    return s;
}

inline QDataStream& operator<<(QDataStream &s, const Prompt &m) {
    if (m.response.isEmpty()) {
        s << qint32(m.type) << m.message << m.hidden << m.response;
    }
    else {
        s << qint32(QAuthPrompt::NONE) << QString() << false << m.response;
    }
    return s;
}

inline QDataStream& operator>>(QDataStream &s, Prompt &m) {
    qint32 type;
    QString message;
    bool hidden;
    QByteArray response;
    s >> type >> message >> hidden >> response;
    m.type = QAuthPrompt::Type(type);
    m.message = message;
    m.hidden = hidden;
    if (!response.isEmpty())
        m.response = response;
    return s;
}

inline QDataStream& operator<<(QDataStream &s, const Request &m) {
    qint32 length = m.prompts.length();
    s << m.info << length;
    Q_FOREACH(Prompt p, m.prompts) {
        s << p;
    }
    return s;
}

inline QDataStream& operator>>(QDataStream &s, Request &m) {
    QString info;
    QList<Prompt> prompts;
    qint32 length;
    s >> info >> length;
    for (int i = 0; i < length; i++) {
        Prompt p;
        s >> p;
        prompts << p;
    }
    m.info = info;
    m.prompts = prompts;
    return s;
}

#endif // MESSAGES_H