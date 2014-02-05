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

#ifndef QAUTH_H
#define QAUTH_H

#include <QtCore/QObject>
#include <QProcessEnvironment>

namespace QAuth {

    class Prompt : public QObject {
        Q_OBJECT
        Q_ENUMS(Type)
        Q_PROPERTY(Type type READ type)
        Q_PROPERTY(QString message READ message)
        Q_PROPERTY(bool hidden READ hidden)
        Q_PROPERTY(QByteArray response READ response WRITE setResponse NOTIFY responseChanged)
    public:
        Prompt(QObject *parent = 0);
        enum Type {
            User,
            Password
        };
        Type type() const;
        QString message() const;
        bool hidden() const;
        QByteArray response() const;
        void setResponse(const QByteArray &r);
    signals:
        void responseChanged();
    private:
        class Private;
        Private *d { nullptr };
        friend class QAuth;
    };

    class Request : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString info READ info)
        Q_PROPERTY(QList<Prompt> prompts READ prompts)
    public:
        Request(QObject *parent = 0);
        QString info() const;
        QList<Prompt> prompts() const;
    private:
        class Private;
        Private *d { nullptr };
        friend class QAuth;
    };

    class QAuth : public QObject
    {
        Q_OBJECT
    public:
        class Private;
        explicit QAuth(QObject *parent = 0);
        virtual ~QAuth();

        /**
         * Set the session to be started after authenticating.
         * @param path Path of the session executable to be started
         */
        void setExecutable(const QString &path);

        /**
         * Set mode to autologin.
         * Ignored if session is not started
         * @param on true if should autologin
         */
        void setAutologin(bool on = true);

        /**
         * Forwards the output of the underlying authenticator to the current process
         * @param on true if should forward the output
         */
        void setVerbosity(bool on = true);

    public slots:
        /**
         * Sets up the environment and starts the authentication
         */
        void start();

    signals:
        void internalError(QString message);
        void authenticated(QString user);
        void sessionOpened();
        void finished(int exitValue);

    protected:
        /**
         * Prompt to the user coming from the underlying authentication stack.
         * To be overriden by the user of the library
         * @param message the message prompting the user, to be presented to him
         * @param echo true if the input data should be also shown to the user,
         *             typically for usernames; false if it should be hidden - for
         *             passphrases, etc.
         * @return data retrieved from the user
         */
        virtual QByteArray prompt(const QString &message, bool echo = false);
        /**
         * Information message coming from the underlying authentication stack.
         * To be overriden by the user of the library
         * @param message the message to be presented to the user
         */
        virtual void info(const QString &message);
        /**
         * Error message coming from the underlying authentication stack.
         * To be overriden by the user of the library
         * @param message the message to presented to the user
         */
        virtual void error(const QString &message);

        /**
         * If starting a session, you should override this to provide the basic
         * process environment.
         * User-specific data such as HOME is generated automatically.
         * @return initial environment values for the session
         */
        virtual QProcessEnvironment provideEnvironment();

    private:
        friend Private;
        Private *d { nullptr };
    };
}

#endif // QAUTH_H
