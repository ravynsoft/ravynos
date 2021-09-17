/*
   SPDX-FileCopyrightText: 2010 Marco Mentasti <marcomentasti@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "connectionwizard.h"
#include "sqlmanager.h"

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordLineEdit>
#include <KUrlRequester>

#include <QFormLayout>
#include <QSpinBox>
#include <QSqlDatabase>
#include <QSqlError>

ConnectionWizard::ConnectionWizard(SQLManager *manager, Connection *conn, QWidget *parent, Qt::WindowFlags flags)
    : QWizard(parent, flags)
    , m_manager(manager)
    , m_connection(conn)
{
    setWindowTitle(i18nc("@title:window", "Connection Wizard"));

    setPage(Page_Driver, new ConnectionDriverPage);
    setPage(Page_Standard_Server, new ConnectionStandardServerPage);
    setPage(Page_SQLite_Server, new ConnectionSQLiteServerPage);
    setPage(Page_Save, new ConnectionSavePage);
}

ConnectionWizard::~ConnectionWizard()
{
}

ConnectionDriverPage::ConnectionDriverPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Database Driver"));
    setSubTitle(i18nc("@title Wizard page subtitle", "Select the database driver"));

    QFormLayout *layout = new QFormLayout();

    driverComboBox = new KComboBox();
    driverComboBox->addItems(QSqlDatabase::drivers());

    layout->addRow(i18nc("@label:listbox", "Database driver:"), driverComboBox);

    setLayout(layout);

    registerField(QStringLiteral("driver"), driverComboBox, "currentText");
}

void ConnectionDriverPage::initializePage()
{
    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());
    Connection *c = wiz->connection();

    if (!c->driver.isEmpty()) {
        driverComboBox->setCurrentItem(c->driver);
    }
}

int ConnectionDriverPage::nextId() const
{
    if (driverComboBox->currentText().contains(QLatin1String("QSQLITE"))) {
        return ConnectionWizard::Page_SQLite_Server;
    } else {
        return ConnectionWizard::Page_Standard_Server;
    }
}

ConnectionStandardServerPage::ConnectionStandardServerPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Connection Parameters"));
    setSubTitle(i18nc("@title Wizard page subtitle", "Please enter connection parameters"));

    QFormLayout *layout = new QFormLayout();

    hostnameLineEdit = new KLineEdit();
    usernameLineEdit = new KLineEdit();
    passwordLineEdit = new KPasswordLineEdit();
    databaseLineEdit = new KLineEdit();
    optionsLineEdit = new KLineEdit();
    portSpinBox = new QSpinBox();

    portSpinBox->setMaximum(65535);
    portSpinBox->setSpecialValueText(i18nc("@item Spinbox special value", "Default"));
    portSpinBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    layout->addRow(i18nc("@label:textbox", "Hostname:"), hostnameLineEdit);
    layout->addRow(i18nc("@label:textbox", "Username:"), usernameLineEdit);
    layout->addRow(i18nc("@label:textbox", "Password:"), passwordLineEdit);
    layout->addRow(i18nc("@label:spinbox", "Port:"), portSpinBox);
    layout->addRow(i18nc("@label:textbox", "Database name:"), databaseLineEdit);
    layout->addRow(i18nc("@label:textbox", "Connection options:"), optionsLineEdit);

    setLayout(layout);

    registerField(QStringLiteral("hostname*"), hostnameLineEdit);
    registerField(QStringLiteral("username"), usernameLineEdit);
    registerField(QStringLiteral("password"), passwordLineEdit, "password", "passwordChanged");
    registerField(QStringLiteral("database"), databaseLineEdit);
    registerField(QStringLiteral("stdOptions"), optionsLineEdit);
    registerField(QStringLiteral("port"), portSpinBox);
}

ConnectionStandardServerPage::~ConnectionStandardServerPage()
{
}

void ConnectionStandardServerPage::initializePage()
{
    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());
    Connection *c = wiz->connection();

    hostnameLineEdit->setText(QStringLiteral("localhost"));

    if (c->driver == field(QStringLiteral("driver")).toString()) {
        hostnameLineEdit->setText(c->hostname);
        usernameLineEdit->setText(c->username);
        passwordLineEdit->setPassword(c->password);
        databaseLineEdit->setText(c->database);
        optionsLineEdit->setText(c->options);
        portSpinBox->setValue(c->port);
    }

    hostnameLineEdit->selectAll();
}

bool ConnectionStandardServerPage::validatePage()
{
    Connection c;

    c.driver = field(QStringLiteral("driver")).toString();
    c.hostname = field(QStringLiteral("hostname")).toString();
    c.username = field(QStringLiteral("username")).toString();
    c.password = field(QStringLiteral("password")).toString();
    c.database = field(QStringLiteral("database")).toString();
    c.options = field(QStringLiteral("stdOptions")).toString();
    c.port = field(QStringLiteral("port")).toInt();

    QSqlError e;

    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());

    if (!wiz->manager()->testConnection(c, e)) {
        KMessageBox::error(this, i18n("Unable to connect to database.") + QLatin1Char('\n') + e.text());
        return false;
    }

    return true;
}

int ConnectionStandardServerPage::nextId() const
{
    return ConnectionWizard::Page_Save;
}

ConnectionSQLiteServerPage::ConnectionSQLiteServerPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Connection Parameters"));
    setSubTitle(
        i18nc("@title Wizard page subtitle", "Please enter the SQLite database file path.\nIf the file does not exist, a new database will be created."));

    QFormLayout *layout = new QFormLayout();

    pathUrlRequester = new KUrlRequester(this);
    optionsLineEdit = new KLineEdit();

    pathUrlRequester->setMode(KFile::File);
    pathUrlRequester->setFilter(QLatin1String("*.db *.sqlite|") + i18n("Database files") + QLatin1String("\n*|") + i18n("All files"));

    layout->addRow(i18nc("@label:textbox", "Path:"), pathUrlRequester);
    layout->addRow(i18nc("@label:textbox", "Connection options:"), optionsLineEdit);

    setLayout(layout);

    registerField(QStringLiteral("path*"), pathUrlRequester->lineEdit());
    registerField(QStringLiteral("sqliteOptions"), optionsLineEdit);
}

void ConnectionSQLiteServerPage::initializePage()
{
    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());
    Connection *c = wiz->connection();

    if (c->driver == field(QStringLiteral("driver")).toString()) {
        pathUrlRequester->lineEdit()->setText(c->database);
        optionsLineEdit->setText(c->options);
    }
}

bool ConnectionSQLiteServerPage::validatePage()
{
    Connection c;

    c.driver = field(QStringLiteral("driver")).toString();
    c.database = field(QStringLiteral("path")).toString();
    c.options = field(QStringLiteral("sqliteOptions")).toString();

    QSqlError e;

    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());

    if (!wiz->manager()->testConnection(c, e)) {
        KMessageBox::error(this, xi18nc("@info", "Unable to connect to database.<nl/><message>%1</message>", e.text()));
        return false;
    }

    return true;
}

int ConnectionSQLiteServerPage::nextId() const
{
    return ConnectionWizard::Page_Save;
}

ConnectionSavePage::ConnectionSavePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18nc("@title Wizard page title", "Connection Name"));
    setSubTitle(i18nc("@title Wizard page subtitle", "Enter a unique connection name"));

    QFormLayout *layout = new QFormLayout();

    connectionNameLineEdit = new KLineEdit();

    layout->addRow(i18nc("@label:textbox", "Connection name:"), connectionNameLineEdit);

    setLayout(layout);

    registerField(QStringLiteral("connectionName*"), connectionNameLineEdit);
}

void ConnectionSavePage::initializePage()
{
    QString name;

    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());
    Connection *c = wiz->connection();

    if (!c->name.isEmpty()) {
        name = c->name;
    } else if (field(QStringLiteral("driver")).toString().contains(QLatin1String("QSQLITE"))) {
        /// TODO: use db file basename
        name = QStringLiteral("SQLite");

        for (int i = 1; QSqlDatabase::contains(name); i++) {
            name = QStringLiteral("%1%2").arg(QLatin1String("SQLite")).arg(i);
        }
    } else {
        name = QStringLiteral("%1 on %2").arg(field(QStringLiteral("database")).toString()).arg(field(QStringLiteral("hostname")).toString()).simplified();

        for (int i = 1; QSqlDatabase::contains(name); i++) {
            name = QStringLiteral("%1 on %2 (%3)")
                       .arg(field(QStringLiteral("database")).toString())
                       .arg(field(QStringLiteral("hostname")).toString())
                       .arg(i)
                       .simplified();
        }
    }

    connectionNameLineEdit->setText(name);
    connectionNameLineEdit->selectAll();
}

bool ConnectionSavePage::validatePage()
{
    QString name = field(QStringLiteral("connectionName")).toString().simplified();

    ConnectionWizard *wiz = static_cast<ConnectionWizard *>(wizard());
    Connection *c = wiz->connection();

    c->name = name;
    c->driver = field(QStringLiteral("driver")).toString();

    if (field(QStringLiteral("driver")).toString().contains(QLatin1String("QSQLITE"))) {
        c->database = field(QStringLiteral("path")).toString();
        c->options = field(QStringLiteral("sqliteOptions")).toString();
    } else {
        c->hostname = field(QStringLiteral("hostname")).toString();
        c->username = field(QStringLiteral("username")).toString();
        c->password = field(QStringLiteral("password")).toString();
        c->database = field(QStringLiteral("database")).toString();
        c->options = field(QStringLiteral("stdOptions")).toString();
        c->port = field(QStringLiteral("port")).toInt();
    }

    return true;
}

int ConnectionSavePage::nextId() const
{
    return -1;
}
