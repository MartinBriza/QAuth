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
    QString info;
    QList<QAuthPrompt*> prompts;
    bool finishAutomatically { false };
};

QAuthRequest::Private::Private(QObject* parent)
        : QObject(parent) { }

void QAuthRequest::Private::responseChanged() {
    Q_FOREACH(QAuthPrompt *qap, prompts) {
        if (qap->response().isEmpty())
            return;
    }
    qobject_cast<QAuthRequest*>(parent())->done();
}

QAuthRequest::QAuthRequest(const Request *request, QAuth *parent)
        : QObject(parent)
        , d(new Private(this)) {
    d->info = request->info;
    Q_FOREACH (const Prompt& p, request->prompts) {
        d->prompts << new QAuthPrompt(&p, this);
    }
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
    emit finished();
}

bool QAuthRequest::finishAutomatically() {
    return d->finishAutomatically;
}

void QAuthRequest::setFinishAutomatically(bool value) {
    if (value != d->finishAutomatically) {
        d->finishAutomatically = value;
        if (d->finishAutomatically) {
            Q_FOREACH (QAuthPrompt *qap, d->prompts) {
                connect(qap, SIGNAL(responseChanged()), d, SLOT(responseChanged()));
            }
            d->responseChanged();
        }
        else {
            Q_FOREACH (QAuthPrompt *qap, d->prompts) {
                disconnect(qap, SIGNAL(responseChanged()), d, SLOT(responseChanged()));
            }
        }
        emit finishAutomaticallyChanged();
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
