/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef CONNECTIONWIZARD_H
#define CONNECTIONWIZARD_H

class SQLManager;
class KComboBox;
class KLineEdit;
class KPasswordLineEdit;
class QSpinBox;
class KUrlRequester;

#include "connection.h"

#include <KWallet>
#include <qwizard.h>

class ConnectionWizard : public QWizard
{
public:
    enum { Page_Driver, Page_Standard_Server, Page_SQLite_Server, Page_Save };

    ConnectionWizard(SQLManager *manager, Connection *conn, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~ConnectionWizard() override;

    SQLManager *manager()
    {
        return m_manager;
    }
    Connection *connection()
    {
        return m_connection;
    }

private:
    SQLManager *m_manager;
    Connection *m_connection;
};

class ConnectionDriverPage : public QWizardPage
{
public:
    ConnectionDriverPage(QWidget *parent = nullptr);
    void initializePage() override;
    int nextId() const override;

private:
    KComboBox *driverComboBox;
};

class ConnectionStandardServerPage : public QWizardPage
{
public:
    ConnectionStandardServerPage(QWidget *parent = nullptr);
    ~ConnectionStandardServerPage() override;
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    KLineEdit *hostnameLineEdit;
    KLineEdit *usernameLineEdit;
    KPasswordLineEdit *passwordLineEdit;
    KLineEdit *databaseLineEdit;
    KLineEdit *optionsLineEdit;
    QSpinBox *portSpinBox;
};

class ConnectionSQLiteServerPage : public QWizardPage
{
public:
    ConnectionSQLiteServerPage(QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    //     KLineEdit *pathLineEdit;
    KUrlRequester *pathUrlRequester;

    KLineEdit *optionsLineEdit;
};

class ConnectionSavePage : public QWizardPage
{
public:
    ConnectionSavePage(QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    KLineEdit *connectionNameLineEdit;
};

#endif // CONNECTIONWIZARD_H
