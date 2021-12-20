#pragma once

#include <QObject>

#include "channeldata.h"

#include <QVector>

class Channel;

class ChannelFactory : public QObject
{
    Q_OBJECT

public:
    ChannelFactory();
    ~ChannelFactory() = default;

    size_t count(bool onlyFavorites) const;
    Channel *create(bool onlyFavorites, int index) const;
    void load(bool onlyFavorites) const;

private:
    mutable QVector<ChannelData> m_channels;
    mutable QVector<ChannelData> m_favorites;
};
