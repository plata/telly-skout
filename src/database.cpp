/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <KLocalizedString>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "TellySkoutSettings.h"
#include "database.h"
#include "fetcher.h"

#define TRUE_OR_RETURN(x)                                                                                                                                      \
    if (!x)                                                                                                                                                    \
        return false;

Database::Database()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    QString databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir(databasePath).mkpath(databasePath);
    db.setDatabaseName(databasePath + QStringLiteral("/database.db3"));
    db.open();

    if (!createTables()) {
        qCritical() << "Failed to create database";
    }

    cleanup();
}

bool Database::createTables()
{
    qDebug() << "Create DB tables";
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Countries (id TEXT UNIQUE, name TEXT, url TEXT);")));
    TRUE_OR_RETURN(
        execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Channels (id TEXT UNIQUE, name TEXT, url TEXT, image TEXT, notify BOOL, favorite BOOL);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS CountryChannels (id TEXT UNIQUE, country TEXT, channel TEXT);")));
    TRUE_OR_RETURN(
        execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Programs (id TEXT UNIQUE, channel TEXT, start INTEGER, stop INTEGER, title TEXT, subtitle TEXT, "
                               "description TEXT, category TEXT);")));

    TRUE_OR_RETURN(execute(QStringLiteral("PRAGMA user_version = 1;")));
    return true;
}

bool Database::execute(const QString &query)
{
    QSqlQuery q;
    q.prepare(query);
    return execute(q);
}

bool Database::execute(QSqlQuery &query)
{
    if (!query.exec()) {
        qWarning() << "Failed to execute SQL Query";
        qWarning() << query.lastQuery();
        qWarning() << query.lastError();
        return false;
    }
    return true;
}

int Database::version()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("PRAGMA user_version;"));
    execute(query);
    if (query.next()) {
        bool ok;
        int value = query.value(0).toInt(&ok);
        qDebug() << "Database version " << value;
        if (ok) {
            return value;
        }
    } else {
        qCritical() << "Failed to check database version";
    }
    return -1;
}

void Database::cleanup()
{
    const TellySkoutSettings settings;
    const unsigned int count = settings.deleteAfterCount();

    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-static_cast<qint64>(count));
    const qint64 sinceEpoch = dateTime.toSecsSinceEpoch();

    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM Programs WHERE stop < :sinceEpoch;"));
    query.bindValue(QStringLiteral(":sinceEpoch"), sinceEpoch);
    execute(query);
}

bool Database::countryExists(const QString &url)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (url) FROM Countries WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), url);
    Database::instance().execute(query);
    query.next();
    return query.value(0).toInt() != 0;
}

bool Database::channelExists(const QString &url)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (url) FROM Channels WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), url);
    Database::instance().execute(query);
    query.next();
    return query.value(0).toInt() != 0;
}

void Database::addCountry(const QString &id, const QString &name, const QString &url)
{
    if (countryExists(url)) {
        return;
    }
    qDebug() << "Add country" << name;

    QUrl urlFromInput = QUrl::fromUserInput(url);
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Countries VALUES (:id, :name, :url);"));
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":name"), name);
    query.bindValue(QStringLiteral(":url"), urlFromInput.toString());
    execute(query);

    Q_EMIT countryAdded(urlFromInput.toString());

    // Fetcher::instance().fetchCountry(urlFromInput.toString()); // TODO: url -> ID
}

void Database::addChannel(const QString &id, const QString &name, const QString &url, const QString &country, const QString &image, bool favorite)
{
    if (channelExists(url)) {
        return;
    }
    qDebug() << "Add channel" << name;

    QUrl urlFromInput = QUrl::fromUserInput(url);
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Channels VALUES (:id, :name, :url, :image, :notify, :favorite);"));
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":name"), name);
    query.bindValue(QStringLiteral(":url"), urlFromInput.toString());
    query.bindValue(QStringLiteral(":country"), country);
    query.bindValue(QStringLiteral(":image"), image);
    query.bindValue(QStringLiteral(":notify"), false);
    query.bindValue(QStringLiteral(":favorite"), favorite);
    execute(query);

    Q_EMIT channelAdded(urlFromInput.toString());

    // Fetcher::instance().fetchChannel(urlFromInput.toString(), urlFromInput.toString()); // TODO: url -> ID
}
