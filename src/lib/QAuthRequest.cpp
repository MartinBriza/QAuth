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

#include "request.h"
#include "lib/qauth.h"

#include "Messages.h"

class QAuthRequest::Private : public QObject {
    Q_OBJECT
public slots:
    void responseChanged();
public:
    Private(QObject *parent);
    QString info { };
    QList<QAuthPrompt*> prompts { };
    bool finishAutomatically { false };
    bool finished { false };
};

QAuthRequest::Private::Private(QObject* parent)
        : QObject(parent) { }

void QAuthRequest::Private::responseChanged() {
    Q_FOREACH(QAuthPrompt *qap, prompts) {
        if (qap->response().isEmpty())
            return;
    }
    if (finishAutomatically && prompts.length() > 0)
        qobject_cast<QAuthRequest*>(parent())->done();
}

QAuthRequest::QAuthRequest(QAuth *parent)
        : QObject(parent)
        , d(new Private(this)) { }

void QAuthRequest::setRequest(const Request *request) {
    qDeleteAll(d->prompts);
    d->prompts.clear();
    d->info.clear();
    if (request != nullptr) {
        d->info = request->info;
        Q_FOREACH (const Prompt& p, request->prompts) {
            QAuthPrompt *qap = new QAuthPrompt(&p, this);
            d->prompts << qap;
            if (finishAutomatically())
                connect(qap, SIGNAL(responseChanged()), d, SLOT(responseChanged()));
        }
    }
    d->finished = false;
    Q_EMIT promptsChanged();
}

QString QAuthRequest::info() const {
    return d->info;
}

QList<QAuthPrompt*> QAuthRequest::prompts() {
    return d->prompts;
}

QDeclarativeListProperty<QAuthPrompt> QAuthRequest::promptsDecl() {
    return QDeclarativeListProperty<QAuthPrompt>(this, d->prompts);
}

void QAuthRequest::done() {
    if (!d->finished) {
        d->finished = true;
        Q_EMIT finished();
    }
}

bool QAuthRequest::finishAutomatically() {
    return d->finishAutomatically;
}

void QAuthRequest::setFinishAutomatically(bool value) {
    if (value != d->finishAutomatically) {
        d->finishAutomatically = value;
        Q_EMIT finishAutomaticallyChanged();
    }
}

Request QAuthRequest::request() const {
    Request r;
    r.info = d->info;
    Q_FOREACH (const QAuthPrompt* qap, d->prompts) {
        Prompt p;
        p.hidden = qap->hidden();
        p.message = qap->message();
        p.response = qap->response();
        p.type = qap->type();
        r.prompts << p;
    }
    return r;
}

#include "moc_request.moc"
#include "QAuthRequest.moc"
