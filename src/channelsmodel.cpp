#include "channelsmodel.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>

#include <algorithm>

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_onlyFavorites(true) // deliberately lazy to save time if only favorites required
    , m_channelFactory(m_onlyFavorites)
{
    connect(&Fetcher::instance(), &Fetcher::countryUpdated, this, [this](const CountryId &id) {
        Q_UNUSED(id)
        beginResetModel();
        qDeleteAll(m_channels);
        m_channels.clear();
        m_channelFactory.load();
        endResetModel();
    });

    connect(&Fetcher::instance(), &Fetcher::channelDetailsUpdated, this, [this](const ChannelId &id, const QString &image) {
        for (int i = 0; i < m_channels.length(); i++) {
            if (m_channels[i]->id() == id.value()) {
                m_channels[i]->setImage(image);
                m_channelFactory.update(id);
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });

    connect(&Database::instance(), &Database::channelDetailsUpdated, this, [this](const ChannelId &id, bool favorite) {
        // with "only favorites", a row must be added/removed -> not sufficient to call only dataChanged()
        if (m_onlyFavorites) {
            beginResetModel();
            qDeleteAll(m_channels);
            m_channels.clear();
            m_channelFactory.update(id);
            endResetModel();
        } else {
            for (int i = 0; i < m_channels.length(); i++) {
                if (m_channels[i]->id() == id.value()) {
                    m_channels[i]->setFavorite(favorite);
                    m_channelFactory.update(id);
                    Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                    break;
                }
            }
        }
    });

    connect(&Database::instance(), &Database::favoritesUpdated, this, [this]() {
        beginResetModel();
        qDeleteAll(m_channels);
        m_channels.clear();
        m_channelFactory.load();
        endResetModel();
    });
}

bool ChannelsModel::onlyFavorites() const
{
    return m_onlyFavorites;
}

void ChannelsModel::setOnlyFavorites(bool onlyFavorites)
{
    m_onlyFavorites = onlyFavorites;
    m_channelFactory.setOnlyFavorites(onlyFavorites);
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "channel";
    return roleNames;
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_channelFactory.count();
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
    if (role != 0) {
        return QVariant();
    }
    if (m_channels.length() <= index.row()) {
        loadChannel(index.row());
    }
    return QVariant::fromValue(m_channels[index.row()]);
}

void ChannelsModel::loadChannel(int index) const
{
    m_channels += m_channelFactory.create(index);
}

void ChannelsModel::setFavorite(const QString &channelId, bool favorite)
{
    if (favorite) {
        Database::instance().addFavorite(ChannelId(channelId));
    } else {
        Database::instance().removeFavorite(ChannelId(channelId));
    }
}

void ChannelsModel::move(int from, int to)
{
    const int destination = to > from ? to + 1 : to;

    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination);
    m_channels.move(from, to);
    endMoveRows();
}

void ChannelsModel::save()
{
    QVector<ChannelId> channelIds(m_channels.size());
    std::transform(m_channels.begin(), m_channels.end(), channelIds.begin(), [](const Channel *channel) {
        return ChannelId(channel->id());
    });
    Database::instance().sortFavorites(channelIds);
}
