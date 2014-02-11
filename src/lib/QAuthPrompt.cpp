/*
 * Qt Authentication Library
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

#include "prompt.h"
#include "lib/qauth.h"

#include "Messages.h"

class QAuthPrompt::Private : public Prompt {
public:

};

QAuthPrompt::QAuthPrompt(QAuthRequest *parent)
        : QObject(parent)
        , d(new Private) {
}

QAuthPrompt::Type QAuthPrompt::type() const {
//     return d->type;
}

QString QAuthPrompt::message() const {
//     return d->message;
}

QByteArray QAuthPrompt::response() const {
//     return d->response;
}

void QAuthPrompt::setResponse(const QByteArray &r) {
//     if (r != d->response) {
//         d->response = r;
        emit responseChanged();
//     }
}

bool QAuthPrompt::hidden() const {
//     return d->hidden;
}

#include "moc_prompt.moc"
