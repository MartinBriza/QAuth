/*
 * Qt Authentication library
 * Copyright (C) 2013 Martin Bříza <mbriza@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef QAUTH_H
#define QAUTH_H

#include "request.h"
#include "prompt.h"

#include <QtCore/QObject>
#include <QProcessEnvironment>

/**
 * \brief
 * Main class triggering the authentication and handling all communication
 *
 * \section description
 * There are three basic kinds of authentication:
 *
 *  * Checking only the validity of the user's secrets - The default values
 *
 *  * Logging the user in after authenticating him - You'll have to set the
 *      \ref session property to do that.
 *
 *  * Logging the user in without authenticating - You'll have to set the
 *      \ref session and \ref autologin properties to do that.
 *
 * Usage:
 *
 * Just construct, connect the signals (especially \ref requestChanged)
 * and fire up \ref start
 */
class QAuth : public QObject {
    Q_OBJECT
    // not setting NOTIFY for the properties - they should be set only once before calling start
    Q_PROPERTY(bool autologin READ autologin WRITE setAutologin NOTIFY autologinChanged)
    Q_PROPERTY(bool verbose READ verbose WRITE setVerbose NOTIFY verboseChanged)
    Q_PROPERTY(QString user READ user WRITE setUser NOTIFY userChanged)
    Q_PROPERTY(QString session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(QAuthRequest* request READ request NOTIFY requestChanged)
public:
    explicit QAuth(const QString &user = QString(), const QString &session = QString(), bool autologin = false, QObject *parent = 0, bool verbose = false);
    explicit QAuth(QObject *parent);
    ~QAuth();

    enum Info {
        INFO_NONE = 0,
        INFO_UNKNOWN,
        INFO_PASS_CHANGE_REQUIRED,
        _INFO_LAST
    };

    enum Error {
        ERROR_NONE = 0,
        ERROR_UNKNOWN,
        ERROR_AUTHENTICATION,
        ERROR_INTERNAL,
        _ERROR_LAST
    };

    static void registerTypes();

    bool autologin() const;
    bool verbose() const;
    const QString &user() const;
    const QString &session() const;
    QAuthRequest *request();

    /**
     * If starting a session, you will probably want to provide some basic env variables for the session.
     * This only inserts the variables - if the current key already had a value, it will be overwritten.
     * User-specific data such as $HOME is generated automatically.
     * @param env the environment
     */
    void insertEnvironment(const QProcessEnvironment &env);

    /**
     * Works the same as \ref insertEnvironment but only for one key-value pair
     * @param key key
     * @param value value
     */
    void insertEnvironment(const QString &key, const QString &value);

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
    void setVerbose(bool on = true);

    /**
     * Sets the user which will then authenticate
     * @param user username
     */
    void setUser(const QString &user);

    /**
     * Set the session to be started after authenticating.
     * @param path Path of the session executable to be started
     */
    void setSession(const QString &path);

public Q_SLOTS:
    /**
     * Sets up the environment and starts the authentication
     */
    void start();

Q_SIGNALS:
    void autologinChanged();
    void verboseChanged();
    void userChanged();
    void sessionChanged();
    void requestChanged();

    /**
     * Emitted when authentication phase finishes
     *
     * @note If you want to set some environment variables for the session right before the
     * session is started, connect to this signal using a blocking connection and insert anything
     * you need in the slot.
     * @param user username
     * @param success true if succeeded
     */
    void authentication(QString user, bool success);

    /**
     * Emitted when session starting phase finishes
     *
     * @param success true if succeeded
     */
    void session(bool success);

    /**
     * Emitted when the helper quits, either after authentication or when the session ends.
     * Or, when something goes wrong
     *
     * @param success true if every underlying task went fine
     */
    void finished(bool success);

    /**
     * Emitted on error
     *
     * @param message message to be displayed to the user
     */
    void error(QString message, QAuth::Error type);

    /**
     * Information from the underlying stack is to be presented to the user
     *
     * @param message message to be displayed to the user
     */
    void info(QString message, QAuth::Info type);

private:
    class Private;
    class SocketServer;
    friend Private;
    friend SocketServer;
    Private *d { nullptr };
};

#endif // QAUTH_H
