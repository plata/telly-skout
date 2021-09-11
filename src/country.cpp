/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "country.h"

#include "channelsmodel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>
#include <QSqlQuery>

Country::Country(int index)
    : QObject(nullptr)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM Countries ORDER BY name LIMIT 1 OFFSET :index;"));
    query.bindValue(QStringLiteral(":index"), index);
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to load channel" << index;
    }

    m_id = query.value(QStringLiteral("id")).toString();
    m_url = query.value(QStringLiteral("url")).toString();
    m_name = query.value(QStringLiteral("name")).toString();

    m_errorId = 0;
    m_errorString = QLatin1String("");

    connect(&Fetcher::instance(), &Fetcher::startedFetchingCountry, this, [this](const QString &id) {
        if (id == m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::countryUpdated, this, [this](const QString &id) {
        if (id == m_id) {
            setRefreshing(false);
            Q_EMIT channelCountChanged();
            setErrorId(0);
            setErrorString(QLatin1String(""));
        }
    });
    connect(&Fetcher::instance(), &Fetcher::error, this, [this](const QString &id, int errorId, const QString &errorString) {
        if (id == m_id) {
            setErrorId(errorId);
            setErrorString(errorString);
            setRefreshing(false);
        }
    });

    m_channels = new ChannelsModel(this);
}

Country::~Country()
{
}

QString Country::id() const
{
    return m_id;
}

QString Country::url() const
{
    return m_url;
}

QString Country::name() const
{
    return m_name;
}

int Country::channelCount() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM CountryChannels where country=:country;"));
    query.bindValue(QStringLiteral(":country"), m_id);
    Database::instance().execute(query);
    if (!query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

bool Country::refreshing() const
{
    return m_refreshing;
}

int Country::errorId() const
{
    return m_errorId;
}

QString Country::errorString() const
{
    return m_errorString;
}

void Country::setName(const QString &name)
{
    m_name = name;
    Q_EMIT nameChanged(m_name);
}

void Country::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Country::setErrorId(int errorId)
{
    m_errorId = errorId;
    Q_EMIT errorIdChanged(m_errorId);
}

void Country::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
    Q_EMIT errorStringChanged(m_errorString);
}

void Country::refresh()
{
    Fetcher::instance().fetchCountry(m_url, ""); // TODO: url -> ID
}

void Country::setAsFavorite()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE Countries SET favorite=TRUE WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
}

void Country::remove()
{
    // Delete Countries
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM Countries WHERE channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_url);
    Database::instance().execute(query);

    // Delete Programs
    query.prepare(QStringLiteral("DELETE FROM Programs WHERE channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_url);
    Database::instance().execute(query);

    // Delete Country
    query.prepare(QStringLiteral("DELETE FROM Countries WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
}
