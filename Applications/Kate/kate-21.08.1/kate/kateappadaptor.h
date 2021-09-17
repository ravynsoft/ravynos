/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _kateapp_adaptor_h_
#define _kateapp_adaptor_h_

#include <QDBusAbstractAdaptor>

class KateApp;

class KateAppAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Kate.Application")
    Q_PROPERTY(QString activeSession READ activeSession)
public:
    KateAppAdaptor(KateApp *app);

    /**
     * emit the exiting signal
     */
    void emitExiting();
    void emitDocumentClosed(const QString &token);

public Q_SLOTS:
    /**
     * open a file with given url and encoding
     * will get view created
     * @param url url of the file
     * @param encoding encoding name
     * @return success
     */
    bool openUrl(const QString &url, const QString &encoding);

    /**
     * checks if the Kate instance is in the specified activity
     * @param activity activity to check
     * @return true if it is in the specified activity, false otherwise
     */
    bool isOnActivity(const QString &activity);

    /**
     * open a file with given url and encoding
     * will get view created
     * @param url url of the file
     * @param encoding encoding name
     * @return token or ERROR
     */
    QString tokenOpenUrl(const QString &url, const QString &encoding);

    /**
     * Like the above, but adds an option to let the documentManager know
     * if the file should be deleted when closed.
     * @p isTempFile should be set to true with the --tempfile option set ONLY,
     * files opened with this set to true will be deleted when closed.
     */
    bool openUrl(const QString &url, const QString &encoding, bool isTempFile);

    QString tokenOpenUrl(const QString &url, const QString &encoding, bool isTempFile);

    QString tokenOpenUrlAt(const QString &url, int line, int column, const QString &encoding, bool isTempFile);

    /**
     * set cursor of active view in active main window
     * will clear selection
     * @param line line for cursor
     * @param column column for cursor
     * @return success
     */
    bool setCursor(int line, int column);

    /**
     * helper to handle stdin input
     * open a new document/view, fill it with the text given
     * @param text text to fill in the new doc/view
     * @param encoding encoding to set for the document, if any
     * @return success
     */
    bool openInput(const QString &text, const QString &encoding);

    /**
     * activate a given session
     * @param session session name
     * @return success
     */
    bool activateSession(const QString &session);

    int desktopNumber();

    /**
     * activate this kate instance
     */
    void activate();

Q_SIGNALS:
    /**
     * Notify the world that this kate instance is exiting.
     * All apps should stop using the dbus interface of this instance after this signal got emitted.
     */
    void exiting();
    void documentClosed(const QString &token);

public:
    QString activeSession();

private:
    KateApp *m_app;
};

#endif
