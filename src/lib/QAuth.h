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
    QAuthPrompt(QObject *parent = 0);
    enum Type {
        NONE,
        UNKNOWN,
        LOGIN_USER,
        LOGIN_PASSWORD,
        CHANGE_CURRENT,
        CHANGE_NEW,
        CHANGE_REPEAT
    };
    Type type() const;
    QString message() const;
    bool hidden() const;
    QByteArray response() const;
    void setResponse(const QByteArray &r);
Q_SIGNALS:
    void responseChanged();
protected:
    void setType(Type type);
    void setMessage(const QString &message);
    void setHidden(bool hidden);
private:
    class Private;
    Private *d { nullptr };
    friend class QAuth;
    friend class QAuthRequest;
    friend QDataStream& operator<<(QDataStream &s, const QAuthPrompt &m);
    friend QDataStream& operator>>(QDataStream &s, QAuthPrompt &m);
};

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
    QAuthRequest(QObject *parent = 0);
    QString info() const;
    QList<QAuthPrompt*> prompts() const;
public Q_SLOTS:
    void done();
protected:
    void setInfo(const QString &info);
    void setPrompts(const QList<QAuthPrompt*> prompts);
private:
    class Private;
    Private *d { nullptr };
    friend class QAuth;
    friend class QAuthPrompt;
    friend QDataStream& operator<<(QDataStream &s, const QAuthRequest &m);
    friend QDataStream& operator>>(QDataStream &s, QAuthRequest &m);
};

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
 * Just construct, connect the signals (especially \ref request)
 * and fire up \ref start
 */
class QAuth : public QObject {
    Q_OBJECT
    // not setting NOTIFY for the properties - they should be set only once before calling start
    Q_PROPERTY(bool autologin READ autologin WRITE setAutologin)
    Q_PROPERTY(bool verbose READ verbose WRITE setVerbose)
    Q_PROPERTY(QString user READ user WRITE setUser)
    Q_PROPERTY(QString session READ session WRITE setSession)
public:
    class Private;
    explicit QAuth(const QString &user = QString(), const QString &session = QString(), bool autologin = false, QObject *parent = 0, bool verbose = false);
    explicit QAuth(QObject *parent);
    virtual ~QAuth();

    bool autologin() const;
    bool verbose() const;
    QString user() const;
    QString session() const;

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
    void request(QAuthRequest *request);

    void error(QString message);
    void authentication(QString user, bool success);
    void session(bool success);
    void finished(bool success);

protected:
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

#endif // QAUTH_H
